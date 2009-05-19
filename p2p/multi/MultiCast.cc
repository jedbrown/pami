#include "../../interface/MultiSend.h"
#include <new>

#define FCN_TABLE_SZ 32

using namespace std;
namespace LL
{
    namespace MultiSend
    {


	void * g_cast_clientdata[FCN_TABLE_SZ];
	typedef struct cast_msg_hdr_t
	{
	    int conn_id;
	    int proto_num;
	} cast_msg_hdr_t;


#if 0
	typedef struct
	{
	    LL_CollectiveProtocol_t       *registration;  /**< Pointer to registration */
	    LL_Request_t        *request;       /**< Temporary storage for the multicast message */
	    LL_Callback_t        cb_done;       /**< Completion callback */
	    unsigned             connection_id; /**< A connection is a distinct stream of data */
	    unsigned             bytes;         /**< size of the message */
	    LL_PipeWorkQueue_t  *src;           /**< source buffer */
	    LL_Topology_t       *participants;  /**< destinations to multicast to*/
	    const Quad          *msginfo;       /**< A extra info field to be send with the msg */
	    unsigned             count;         /**< info count */
	} LL_Multicast_t;
#endif


        class mCastState
        {
        public:
            inline     mCastState();
            inline int init(int             my_rank,
			    lapi_handle_t   ctxt,
			    int             hdr_hdl,
			    int             conn_id,
			    LL_Multicast_t *mcast_info);
            inline int  next();
	    inline void executeCallback();
            int                  _rank;
            lapi_handle_t        _ctxt;
            int                  _hdr_hdl;
            LL_Request_t        *_request;
            LL_Callback_t        _cb_done;
            unsigned             _connection_id;
	    LL_PipeWorkQueue_t  *_src;
	    LL_PipeWorkQueue_t  *_dst;
	    unsigned             _bytes;
            Topology            *_src_topo;
            Topology            *_dst_topo;
	    const Quad          *_msginfo;
            unsigned             _msgcount;
            int                  _size;
            int                  _mask;
	    int                  _rrank;
	    cast_msg_hdr_t       _msg_hdr;
            lapi_xfer_t          _xfer;
        };

        void cast_am_complete(lapi_handle_t * hndl,
                         void          * save)
        {
        }

        void * cast_am_hndlr(lapi_handle_t       * hndl,
                        void                * u_hdr,
                        uint                * hdr_len,
                        ulong               * ret_info,
                        compl_hndlr_t      ** chndlr,
                        void               ** saved_info)
        {
            lapi_return_info_t  * r = (lapi_return_info_t*)ret_info;
            *chndlr                 = cast_am_complete;
            *saved_info             = NULL;
            r->ret_flags            = LAPI_NORMAL;

	    cast_msg_hdr_t      *msg_hdr   = (cast_msg_hdr_t*)u_hdr;
	    int                  conn_id   = msg_hdr->conn_id;
	    int                  proto_num = msg_hdr->proto_num;

	    // Lookup the factory that generated this message
	    MulticastFactory    *f         = (MulticastFactory*)g_cast_clientdata[proto_num];
	    // Lookup the queue for this connection id, create
	    // the queue if none created yet
	    vector<void*>*       q         = f->_conn_map[conn_id];
	    mCastState   *       msg;
	    if(q == NULL)
		{
		    msg=NULL;
		    f->_conn_map[conn_id] = new vector<void*>;
//		    f->_early_conn_map[conn_id] = new vector<void*>;
		}
	    else
		{
		    msg=(mCastState*)q->front();
		}

	    if(msg == NULL)
		{
		    // No message found
		    // Create a new uninitialized message on the q, but don't call next
		    // as the user has not yet called a function to satisfy
		    // the cast operation
		    mCastState * mSS = new mCastState();
		}
	    else
		{
		    //q->erase(q->begin());
		    if(msg->next())
			{
			    // Done
			    q->erase(q->begin());
			}
		    else
			;
		}

            return NULL; // recv buff
        }

        inline mCastState::mCastState()
        {
        }
	inline int mCastState::init(int             my_rank,
				    lapi_handle_t   ctxt,
				    int             hdr_hdl,
				    int             conn_id,
				    LL_Multicast_t *mcast_info)
	{
	    size_t *ranklist = NULL;
	    if (_size == 1)
		return LL_SUCCESS;
            _rank          = my_rank;
            _ctxt          = ctxt;
            _hdr_hdl       = hdr_hdl;
            _request       = mcast_info->request;
            _cb_done       = mcast_info->cb_done;
            _connection_id = mcast_info->connection_id;
	    _src           = mcast_info->src;
	    _dst           = mcast_info->dst;
	    _bytes         = mcast_info->bytes;
            _src_topo      = (Topology*)mcast_info->src_participants;
            _dst_topo      = (Topology*)mcast_info->dst_participants;
	    _msginfo       = mcast_info->msginfo;
	    _msgcount      = mcast_info->msgcount;
//            _size          = _dst_topo->size();
            _mask          = 0x1;
//	    _src_topo->rankList(&ranklist);
	    ranklist[0]    =0;
	    size_t _root   = ranklist[0];
	    _rrank         = (_rank >=_root)?_rank-_root:_rank-_root+_size;
	    _msg_hdr       = (cast_msg_hdr_t){ hdr_hdl, conn_id };
	    return 0;
	}



        inline int mCastState::next()
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

        inline void mCastState::executeCallback()
        {
	    _cb_done.function(_cb_done.clientdata, NULL);
        }


        MulticastFactory::MulticastFactory(lapi_handle_t   ctxt,
                                           LL_Result     & status) :
            MultisendProtocolFactory(ctxt,status)
        {
            _currHndlr           = incrGetHndlr();
            LAPI_Addr_set(_ctxt,
                          (void*)&cast_am_hndlr,
                          _currHndlr);
	    g_cast_clientdata[_currHndlr] = this;
        }

        LL_Result MulticastFactory::generate(LL_Multicast_t *mcast_info)
        {
            int rank;
            LAPI_Qenv(_ctxt, TASK_ID, &rank);
            MulticastFactory    *f         = (MulticastFactory*)g_cast_clientdata[_currHndlr];
            vector<void*>*       q         = f->_conn_map[mcast_info->connection_id];
            mCastState   *       msg;
            if(q == NULL)
                {
                    msg=NULL;
                    f->_conn_map[mcast_info->connection_id]       = new vector<void*>;
                }








	    mCastState * mSS;
	    mSS = new(mcast_info->request)mCastState();
	    mSS->init(rank,
		      _ctxt,
		      _currHndlr,
		      mcast_info->connection_id,
		      mcast_info);
            mSS->next();
            return LL_SUCCESS;
        }


    };
};
