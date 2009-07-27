/**
 * \file adaptor/generic/multisend_impl.h
 * \brief ???
 */

#ifndef   __mpi_multisend_impl_h__
#define   __mpi_multisend_impl_h__
#include <assert.h>
#include "interface/lapiunix/Adaptor.h"
#include "./regTable.h"
#include "interface/lapiunix/common/include/pgasrt.h"

extern CCMI::Adaptor::Adaptor  * _g_generic_adaptor;

#define TEST_NUM 1
//#define DEBUG_MSEND 1

#undef TRACE

#ifdef DEBUG_MSEND
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

namespace CCMI
{
    namespace Adaptor
    {
	namespace Generic
	{
	    extern regTable _g_regtable;
            /**
	     * ***************************************************************************
	     * \brief Send info and callbacks
	     * ***************************************************************************
	     */
	    class send_info
	    {
	    public:
		int                _totalsends;
		int                _numsends;
		CM_Callback_t    _user_cb_done;
	    };
	    static void send_comp_handler(void* input)
		{
		    _g_generic_adaptor->lock();
		    TRACE((stderr, "Send Done Handler %p\n", input));
		    send_info *si = (send_info*) input;
		    si->_numsends++;
		    TRACE((stderr, "Done Handler:  numsends=%d totalsends=%d\n", si->_numsends, si->_totalsends));
		    assert(si->_numsends <= si->_totalsends);
		    if(si->_totalsends == si->_numsends)
			{
			    if (si->_user_cb_done.function)
				{
				    TRACE((stderr, "Delivering User Callback!!\n"));
				    si->_user_cb_done.function(si->_user_cb_done.clientdata, NULL);
				}
			}
		    TRACE((stderr, "Send Completion handler done!\n"));
		    _g_generic_adaptor->unlock();
		}

            /**
	     * ***************************************************************************
	     * \brief reception structures and callbacks
	     * ***************************************************************************
	     */
	    static struct amheader
	    {
		__pgasrt_AMHeader_t   _hdr;
		CMQuad              _info[2];
		int                   _info_count;
		int                   _size;
		int                   _peer;
		int                   _conn;
		int                   _regid;
	    }
	    _g_amheader __attribute__((__aligned__(16)));
	    static void comphandler (void * unused, void *);
	    static inline __pgasrt_local_addr_t
		amsend_headerhandler (const __pgasrt_AMHeader_t * header,
				      void (** compHandler) (void *, void *),
				      void ** arg);


	    typedef CM_Request_t * (*msend_recv) (const CMQuad  * info,
						    unsigned          count,
						    unsigned          peer,
						    unsigned          sndlen,
						    unsigned          conn_id,
						    void            * arg,
						    unsigned        * rcvlen,
						    char           ** rcvbuf,
						    unsigned        * pipewidth,
						    CM_Callback_t * cb_done);

	    static int _g_regId = TEST_NUM;

	    class  MulticastImpl :
	        public CCMI::MultiSend::MulticastInterface,
		public CCMI::Adaptor::Message
		{
		public:
		    int _regId;
		    MulticastImpl () : MulticastInterface (), Message()
			{
			    _regId = _g_regId;
			    TRACE((stderr, "registering new mcast iface, id=%d cd=%p\n", _regId, this));
			    __pgasrt_tsp_amsend_reg (_regId, amsend_headerhandler);
			    _g_regtable.add(_regId,(void*)this);
			    void *ok = _g_regtable.get(_regId);
			    assert(ok == this);
			    TRACE((stderr, "registration OK\n"));
			    _g_regId++;

			}
			virtual ~MulticastImpl () {}

			inline void operator delete(void * p)
			    {
				CCMI_abort();
			    }

			inline void initialize (Adaptor *adaptor)
			    {
				TRACE((stderr, "registering message\n"));
				adaptor->registerMessage (static_cast<CCMI::Adaptor::Message *>(this));
			    }

			///
			/// \brief Create a multicast message
			/// \param buf  : start of the buffer to scatter
			/// \param cb_done : Callback when the multicast is sent
			/// \param consisteny : ordering model
			/// \param info : A 16 byte field that will
			///               with the first packet. Info can be NULL if
			///               a Recv has guaranteed to be posted.
			/// \param connection_id : The connection id for this message
			/// \param size : size of the multicast buffer
			/// \param ranks   : Ranks of the destinations
			/// \param nranks  : Number of destinations
			/// \param hints   : deposit bit bcast vs pt-to-pt
			///
			unsigned  send  (CM_Request_t         * request,
					 const CM_Callback_t  * cb_done,
					 CCMI_Consistency         consistency,
					 const CMQuad         * info,
					 unsigned                 info_count,
					 unsigned                 connection_id,
					 const char             * buf,
					 unsigned                 size,
					 unsigned               * hints,
					 unsigned               * ranks,
					 unsigned                 nranks,
					 CM_Op                  op    = CM_UNDEFINED_OP,
					 CM_Dt                  dtype = CM_UNDEFINED_DT )

			{
			    _g_generic_adaptor->lock();
			    void *r           = NULL;
			    int rc            = -1;
			    _g_amheader._info_count = info_count;
			    _g_amheader._size = size;
			    _g_amheader._peer =  _g_generic_adaptor->mapping()->rank();
			    _g_amheader._conn = connection_id;
			    _g_amheader._regid= _regId;

			    if ( info )
				{
				    if(info_count > 2)
					{
					    fprintf(stderr, "FIX:  The lapiunix adaptor only supports up to 2 quads\n");
					    assert(0);
					}
				    memcpy (&_g_amheader._info[0],& info[0], info_count *sizeof (CMQuad));
				}
			    send_info *si                 = (send_info*)request;
			    si->_totalsends               = nranks;
			    si->_numsends                 = 0;
			    si->_user_cb_done.function   = cb_done->function;
			    si->_user_cb_done.clientdata = cb_done->clientdata;
			    for (unsigned count = 0; count < nranks; count ++)
				{
				    assert (hints[count] == CCMI_PT_TO_PT_SUBTASK);
//				    _g_amheader._hdr.handler   = (__pgasrt_AMHeaderHandler_t)TEST_NUM;
				    _g_amheader._hdr.handler   = amsend_headerhandler;
				    _g_amheader._hdr.headerlen = sizeof (_g_amheader);
				    TRACE((stderr, "sending to %d\n", _g_amheader._peer));
				    r = __pgasrt_tsp_amsend (ranks[count],
							     &_g_amheader._hdr,
							     (__pgasrt_local_addr_t)buf,
							     size,
							     send_comp_handler,
							     (void*)si);
				    TRACE((stderr, "%d send complete to %d\n", _g_amheader._peer,
					   ranks[count]));
				}
			    TRACE((stderr, "sends complete\n"));
			    _g_generic_adaptor->unlock();
			    return rc;
			}

			virtual unsigned send (MultiSend::CCMI_Multicast_t  *mcastinfo)
			{
			    return MulticastImpl::send (mcastinfo->request,
							&mcastinfo->cb_done,
							mcastinfo->consistency,
							mcastinfo->msginfo,
							mcastinfo->count,
							mcastinfo->connection_id,
							mcastinfo->src,
							mcastinfo->bytes,
							(unsigned *)mcastinfo->opcodes,
							mcastinfo->ranks,
							mcastinfo->nranks,
							mcastinfo->op,
							mcastinfo->dt);
			}

			virtual unsigned postRecv (CM_Request_t         * request,
						   const CM_Callback_t  * cb_done,
						   unsigned                 conn_id,
						   char                   * buf,
						   unsigned                 size,
						   unsigned                 pwidth,
						   unsigned                 hint   = CCMI_UNDEFINED_SUBTASK,
						   CM_Op                  op     = CM_UNDEFINED_OP,
						   CM_Dt                  dtype  = CM_UNDEFINED_DT )
			{
			    assert (0);
			}

			virtual unsigned postRecv (MultiSend::CCMI_MulticastRecv_t  *mrecv)
			{
			    assert (0);
			}

			virtual void advance ()
			{

			}
		public:
			void * getAsyncArg() {return this->MulticastInterface::_async_arg;}
			CCMI::MultiSend::CCMI_RecvMulticast_t getCbAsyncHead()
			    {
				return this->MulticastInterface::_cb_async_head;
			    }
		};

		class comp_data
		{
		public:
		    CM_Callback_t  _cb_done;
		    int              _recvlen;
		    int              _pwidth;
		};
		static inline __pgasrt_local_addr_t
		amsend_headerhandler (const __pgasrt_AMHeader_t * header,
				      void (** compHandler) (void *, void *),
				      void ** arg)
	    {
		_g_generic_adaptor->lock();
		amheader       * msg = (amheader*)header;
		TRACE((stderr, "Received a new Message=%p!\n", msg));
		unsigned         rcvlen;
		char           * rcvbuf;
		unsigned         pwidth;
		CM_Callback_t  cb_done;
		MulticastImpl *  mi = (MulticastImpl*)_g_regtable.get(msg->_regid);
		TRACE((stderr, "cb_async: regid=%d mi=%p micount=%d peer=%d sz=%d conn=%d AA=%p\n",
		       msg->_regid,mi, msg->_peer, msg->_info_count,msg->_size, msg->_conn, mi->getAsyncArg()));
		CM_Request_t * req=
		    mi->_cb_async_head(&msg->_info[0],
				       msg->_info_count,
				       msg->_peer,
				       msg->_size,
				       msg->_conn,
				       mi->getAsyncArg(),
				       &rcvlen,
				       &rcvbuf,
				       &pwidth,
				       &cb_done);
		      TRACE((stderr, "Delivered  Callback to user rcvlen=%d buf=%p req=%p!\n", rcvlen, rcvbuf, req));
		

		if(rcvlen == 0)
		    {
			CM_Request_t tmpreq;
			comp_data * cd = (comp_data*)&tmpreq;
			cd->_cb_done   = cb_done;
			cd->_recvlen   = rcvlen;
			cd->_pwidth    = pwidth;
			* compHandler  = NULL;
			* arg          = NULL;
			TRACE((stderr, "0 byte message!\n"));
			comphandler(NULL, (void*)&tmpreq);
			_g_generic_adaptor->unlock();
			return (__pgasrt_local_addr_t) NULL;
		    }
		else
		    {
			comp_data * cd = (comp_data*)req;
			cd->_cb_done   = cb_done;
			cd->_recvlen   = rcvlen;
			cd->_pwidth    = pwidth;
			* compHandler  = comphandler;
			* arg          = (void*)cd;
			TRACE((stderr, "New Message incoming arg=%p, cd=%p\n", arg, cd));
			_g_generic_adaptor->unlock();
			return (__pgasrt_local_addr_t) rcvbuf;
		    }
	    }

	    void comphandler (void * unused, void *arg)
	    {
		_g_generic_adaptor->lock();
		comp_data *cd = (comp_data*) arg;
		TRACE((stderr, "Completion Handler:  arg=%p, pwidth=%d rlen=%d\n", arg,cd->_pwidth, cd->_recvlen));
		if(cd->_pwidth == 0 && cd->_recvlen ==0)
		    {
			if (cd->_cb_done.function)
			    cd->_cb_done.function (cd->_cb_done.clientdata, NULL);
		    }
		for (unsigned count = 0; count < cd->_recvlen; count += cd->_pwidth)
		    {
			TRACE((stderr, "Deliver recv done callback to user count=%p of %p arg=%p\n",
				count,
				cd->_recvlen,
			       arg))
			if (cd->_cb_done.function)
			    cd->_cb_done.function (cd->_cb_done.clientdata, NULL);
		    }
		_g_generic_adaptor->unlock();
		TRACE((stderr, "Done with send comphandler\n"));
	    }
	};
    };
};

#undef TRACE
#endif
