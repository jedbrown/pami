/**
 * \file adaptor/generic/multisend_impl.h
 * \brief ???
 */

#ifndef   __mpi_multisend_impl_h__
#define   __mpi_multisend_impl_h__
#include <assert.h>
#include "collectives/interface/MultiSendOld.h"
#include "collectives/interface/Adaptor.h"
#include "./regTable.h"
#include "collectives/interface/lapiunix/common/include/pgasrt.h"

extern CCMI::Adaptor::Adaptor  * _g_generic_adaptor;

#define TEST_NUM 1

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
		CCMI_Callback_t    _user_cb_done;
	    };
	    static void send_comp_handler(void* input)
		{
//		    fprintf(stderr, "Send Done Handler %p\n", input);
		    send_info *si = (send_info*) input;
		    si->_numsends++;
//		    fprintf(stderr, "Done Handler:  numsends=%d totalsends=%d\n", si->_numsends, si->_totalsends);
		    if(si->_totalsends == si->_numsends)
			{
			    if (si->_user_cb_done.function)
				{
//				    fprintf(stderr, "Delivering User Callback!!\n");
				    si->_user_cb_done.function(si->_user_cb_done.clientdata, NULL);
				}
			}
//		    fprintf(stderr, "Send Completion handler done!\n");
		}

            /**
	     * ***************************************************************************
	     * \brief reception structures and callbacks
	     * ***************************************************************************
	     */
	    static struct amheader
	    {
		__pgasrt_AMHeader_t   _hdr;
		CCMIQuad              _info;
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


	    typedef CCMI_Request_t * (*msend_recv) (const CCMIQuad  * info,
						    unsigned          count,
						    unsigned          peer,
						    unsigned          sndlen,
						    unsigned          conn_id,
						    void            * arg,
						    unsigned        * rcvlen,
						    char           ** rcvbuf,
						    unsigned        * pipewidth,
						    CCMI_Callback_t * cb_done);

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
//			    fprintf(stderr, "registering new mcast iface\n");
			    __pgasrt_tsp_amsend_reg (_regId, amsend_headerhandler);
			    _g_regtable.add(_regId,(void*)this);
			    _g_regId++;

			}
			virtual ~MulticastImpl () {}

			inline void operator delete(void * p)
			    {
				CCMI_abort();
			    }

			inline void initialize (Adaptor *adaptor)
			    {
//				fprintf(stderr, "registering message\n");
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
			unsigned  send  (CCMI_Request_t         * request,
					 const CCMI_Callback_t  * cb_done,
					 CCMI_Consistency         consistency,
					 const CCMIQuad         * info,
					 unsigned                 connection_id,
					 const char             * buf,
					 unsigned                 size,
					 unsigned               * hints,
					 unsigned               * ranks,
					 unsigned                 nranks,
					 CCMI_Op                  op    = CCMI_UNDEFINED_OP,
					 CCMI_Dt                  dtype = CCMI_UNDEFINED_DT )

			{
			    void *r           = NULL;
			    int rc            = -1;
			    _g_amheader._size = size;
			    _g_amheader._peer =  _g_generic_adaptor->mapping()->rank();
			    _g_amheader._conn = connection_id;
			    _g_amheader._regid= _regId;
			    
			    if ( info )
				memcpy (&_g_amheader._info, info, sizeof (CCMIQuad));

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
//				    fprintf(stderr, "sending to %d\n", _g_amheader._peer);
				    r = __pgasrt_tsp_amsend (ranks[count],
							     &_g_amheader._hdr,
							     (__pgasrt_local_addr_t)buf,
							     size,
							     send_comp_handler,
							     (void*)si);
//				    fprintf(stderr, "send complete to %d\n", _g_amheader._peer);
				}
//			    fprintf(stderr, "sends complete\n");
			    return rc;
			}

			virtual unsigned send (MultiSend::CCMI_Multicast_t  *mcastinfo)
			{
			    return MulticastImpl::send (mcastinfo->request,
							&mcastinfo->cb_done,
							mcastinfo->consistency,
							mcastinfo->msginfo,
							mcastinfo->connection_id,
							mcastinfo->src,
							mcastinfo->bytes,
							(unsigned *)mcastinfo->opcodes,
							mcastinfo->ranks,
							mcastinfo->nranks,
							mcastinfo->op,
							mcastinfo->dt);
			}

			virtual unsigned postRecv (CCMI_Request_t         * request,
						   const CCMI_Callback_t  * cb_done,
						   unsigned                 conn_id,
						   char                   * buf,
						   unsigned                 size,
						   unsigned                 pwidth,
						   unsigned                 hint   = CCMI_UNDEFINED_SUBTASK,
						   CCMI_Op                  op     = CCMI_UNDEFINED_OP,
						   CCMI_Dt                  dtype  = CCMI_UNDEFINED_DT )
			{
			    assert (0);
			}

			virtual unsigned postRecv (MultiSend::CCMI_MulticastRecv_t  *mrecv)
			{
			    assert (0);
			}

			virtual void advance ()
			{ 
			    __pgasrt_tsp_wait(NULL);
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
		    CCMI_Callback_t  _cb_done;
		    int              _recvlen;
		    int              _pwidth;
		};
		static inline __pgasrt_local_addr_t
		amsend_headerhandler (const __pgasrt_AMHeader_t * header,
				      void (** compHandler) (void *, void *),
				      void ** arg)
	    {
		amheader       * msg = (amheader*)header;
//		fprintf(stderr, "Received a new Message=%p!\n", msg);
		unsigned         rcvlen;
		char           * rcvbuf;
		unsigned         pwidth;
		CCMI_Callback_t  cb_done;
		MulticastImpl *  mi = (MulticastImpl*)_g_regtable.get(msg->_regid);
//		fprintf(stderr, "cb_async: mi=%p peer=%d sz=%d conn=%d AA=%p \n", 
//			mi, msg->_peer, msg->_size, msg->_conn, mi->getAsyncArg());
		CCMI_Request_t * req=
		    mi->_cb_async_head(&msg->_info,
				    1,
				    msg->_peer,
				    msg->_size,
				    msg->_conn,
				    mi->getAsyncArg(),
				    &rcvlen,
				    &rcvbuf,
				    &pwidth,
				    &cb_done);
//		fprintf(stderr, "Delevered  Callback to user rcvlen=%d buf=%p req=%p!\n", rcvlen, rcvbuf, req);


		if(rcvlen == 0)
		    {
			* compHandler  = NULL;
			* arg          = NULL;
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
//			fprintf(stderr, "New Message incoming arg=%p, cd=%p\n", arg, cd);
			return (__pgasrt_local_addr_t) rcvbuf;
		    }
	    }

	    void comphandler (void * unused, void *arg)
	    {
//		fprintf(stderr, "Completion Handler!  arg=%p\n", arg);
		comp_data *cd = (comp_data*) arg;
		for (unsigned count = 0; count < cd->_recvlen; count += cd->_pwidth)
		    {
//			fprintf(stderr, "Deliver callback to user count=%p of %p arg=%p\n", 
//				count,
//				cd->_recvlen,
//				arg);
			if (cd->_cb_done.function)
			    cd->_cb_done.function (cd->_cb_done.clientdata, NULL);
		    }
//		fprintf (stderr, "Done with comphandler\n");
	    }
	};
    };
};


#endif
