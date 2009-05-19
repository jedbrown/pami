#include "../../interface/MultiSend.h"
#include <new>

#define FCN_TABLE_SZ 32

using namespace std;
namespace LL
{
    namespace MultiSend
    {
	void * g_sync_clientdata[FCN_TABLE_SZ];
	typedef struct sync_msg_hdr_t
	{
	    int conn_id;
	    int proto_num;
	} sync_msg_hdr_t;

        class mSyncState
        {
        public:
            inline mSyncState();
            inline int init(int             my_rank,
			    lapi_handle_t   ctxt,
			    int             hdr_hdl,
			    int             conn_id,
			    LL_Multisync_t *msync_info);
            inline int  next();
	    inline void executeCallback();

            int             _rank;
            lapi_handle_t   _ctxt;
            int             _hdr_hdl;
            LL_Request_t  * _request;
            LL_Callback_t   _cb_done;
            unsigned        _connection_id;
            Topology      * _topo;
            int             _size;
            int             _mask;
	    sync_msg_hdr_t  _msg_hdr;
            lapi_xfer_t     _xfer;
        };

        void sync_am_complete(lapi_handle_t * hndl,
			      void          * save)
        {
        }

        void * sync_am_hndlr(lapi_handle_t       * hndl,
                        void                * u_hdr,
                        uint                * hdr_len,
                        ulong               * ret_info,
                        compl_hndlr_t      ** chndlr,
                        void               ** saved_info)
        {
            lapi_return_info_t  * r = (lapi_return_info_t*)ret_info;
            *chndlr                 = sync_am_complete;
            *saved_info             = NULL;
            r->ret_flags            = LAPI_NORMAL;

	    sync_msg_hdr_t      *msg_hdr   = (sync_msg_hdr_t*)u_hdr;
	    int                  conn_id   = msg_hdr->conn_id;
	    int                  proto_num = msg_hdr->proto_num;

	    // Lookup the factory that generated this message
	    MultisyncFactory    *f         = (MultisyncFactory*)g_sync_clientdata[proto_num];
	    // Lookup the queue for this connection id, create
	    // the queue if none created yet
	    vector<void*>*       q         = f->_conn_map[conn_id];
	    vector<void*>*       eq        = f->_early_conn_map[conn_id];
	    mSyncState   *       msg;
	    if(q == NULL)
		{
		    msg=NULL;
		    f->_conn_map[conn_id] = new vector<void*>;
		    f->_early_conn_map[conn_id] = new vector<void*>;
		}
	    else
		{
		    msg=(mSyncState*)q->front();
		}

	    if(msg == NULL)
		{
		    // No message found
		    // Create a new uninitialized message on the q, but don't call next
		    // as the user has not yet called a function to satisfy
		    // the sync operation
		    mSyncState * mSS = new mSyncState();
		    eq->push_back(mSS);
		}
	    else
		{
		    //q->erase(q->begin());
		    if(msg->next())
			{
			    // Done
			    q->erase(q->begin());
			    msg->executeCallback();
			}
		    else
			;
		}

            return NULL; // recv buff
        }

        inline void mSyncState::executeCallback()
        {
	    _cb_done.function(_cb_done.clientdata, NULL);
        }

        inline mSyncState::mSyncState()
        {
//	    init(my_rank,ctxt,hdr_hdl,conn_id,msync_info);
        }
	inline int mSyncState::init(int             my_rank,
				    lapi_handle_t   ctxt,
				    int             hdr_hdl,
				    int             conn_id,
				    LL_Multisync_t *msync_info)
	{
	    if (_size == 1)
		return LL_SUCCESS;

            _rank          = my_rank;
            _ctxt          = ctxt;
            _hdr_hdl       = hdr_hdl;
            _request       = msync_info->request;
            _cb_done       = msync_info->cb_done;
            _connection_id = msync_info->connection_id;
            _topo          = (Topology*)msync_info->participants;
//            _size          = _topo->size();
            _size          = 0;
            _mask          = 0x1;
	    _msg_hdr       = (sync_msg_hdr_t){ hdr_hdl, conn_id };
	    return 0;
	}



        inline int mSyncState::next()
        {
            if (_mask<_size)
                {
                    // Not done;
                    ;
                }
            else
                {
		    return 1;
                    // Done
                }
            int            dst = (_rank+_mask)%_size;
            int            src = (_rank-_mask+_size)%_size;
            _xfer.Xfer_type    = LAPI_AM_LW_XFER;
            _xfer.Am.hdr_hdl   = _hdr_hdl;
            _xfer.Am.tgt       = dst;
            _xfer.Am.uhdr      = (char*)&_msg_hdr;
            _xfer.Am.uhdr_len  = sizeof(_msg_hdr);
            _xfer.Am.udata     = NULL;
            _xfer.Am.udata_len = 0;
            _xfer.Am.shdlr     = NULL;
            _xfer.Am.sinfo     = NULL;
            _xfer.Am.tgt_cntr  = (lapi_long_t)NULL;
            _xfer.Am.org_cntr  = NULL;
            _xfer.Am.cmpl_cntr = NULL;
            LAPI_Xfer(_ctxt,&_xfer);
            _mask <<= 1;
	    return 0;
        }

        MultisyncFactory::MultisyncFactory(lapi_handle_t   ctxt,
                                           LL_Result     & status) :
            MultisendProtocolFactory(ctxt,status)
        {
            _currHndlr           = incrGetHndlr();
            LAPI_Addr_set(_ctxt,
                          (void*)&sync_am_hndlr,
                          _currHndlr);
	    g_sync_clientdata[_currHndlr] = this;
        }

        LL_Result MultisyncFactory::generate(LL_Multisync_t *msync_info)
        {
            int rank;
            LAPI_Qenv(_ctxt, TASK_ID, &rank);
            // Lookup the factory and allocate one if necessary
	    // Todo....lock to protect from callbacks!
            MultisyncFactory    *f         = (MultisyncFactory*)g_sync_clientdata[_currHndlr];
            vector<void*>*       q         = f->_conn_map[msync_info->connection_id];
            vector<void*>*       eq        = f->_early_conn_map[msync_info->connection_id];
            mSyncState   *       msg;
            if(q == NULL)
                {
                    msg=NULL;
                    f->_conn_map[msync_info->connection_id]       = new vector<void*>;
                    f->_early_conn_map[msync_info->connection_id] = new vector<void*>;
                }

	    mSyncState * mSS;
	    int sz = 0;
//	    int sz = eq->size();
	    if(sz)
		{
		    mSS=(mSyncState*)eq->front();
		    eq->erase(eq->begin());
		}
	    else
		{
		}

	    mSS = new(msync_info->request)mSyncState();
	    mSS->init(rank,
		      _ctxt,
		      _currHndlr,
		      msync_info->connection_id,
		      msync_info);
	    eq->push_back(mSS);
            mSS->next();
            return LL_SUCCESS;
        }
    };
};
