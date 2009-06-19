/**
 * \file adaptor/generic/multisend_impl.h
 * \brief ???
 */

#ifndef   __mpi_multisend_impl_h__
#define   __mpi_multisend_impl_h__

#ifndef MPICH_IGNORE_CXX_SEEK
#define MPICH_IGNORE_CXX_SEEK
#endif

#ifndef OMPI_SKIP_MPICXX
#define OMPI_SKIP_MPICXX
#endif

#include <mpi.h>
#include <assert.h>

#include "collectives/interface/MultiSendOld.h"
#include "collectives/interface/genericmpi/Adaptor.h"
#include "Table.h"

namespace CCMI {
  namespace Adaptor {
    namespace Generic {



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


      class  MulticastImpl : public CCMI::MultiSend::MulticastInterface, public CCMI::Adaptor::Message {

      protected:

      public:

	MulticastImpl () : MulticastInterface (), Message() {}

	virtual ~MulticastImpl () {}

	/// NOTE: This is required to make "C" programs link successfully with virtual destructors
	inline void operator delete(void * p)
	{
	  CCMI_abort();
	}

	inline void initialize (Adaptor *adaptor) {
		TRACE_ADAPTOR((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::initialize() adaptor %#.8X\n", (int)this, (int)adaptor));
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
			 unsigned                 info_count,
			 unsigned                 connection_id,
			 const char             * buf,
			 unsigned                 size,
			 unsigned               * hints,
			 unsigned               * ranks,
			 unsigned                 nranks,
			 CCMI_Op                  op    = CCMI_UNDEFINED_OP,
			 CCMI_Dt                  dtype = CCMI_UNDEFINED_DT )

	  {
	    TRACE_ADAPTOR((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::send()\n", (int)this));
	    MsgHeader *hdr = (MsgHeader *)
	      malloc (sizeof (MsgHeader) + size);

	    assert( hdr != NULL );

	    hdr->_info_count = info_count;
	    hdr->_size = size;
	    hdr->_conn = connection_id;
	    if ( info )
	      {
		memcpy (&hdr->_info[0], info, info_count * sizeof (CCMIQuad));
		if(info_count > 2)
		  {
		    fprintf(stderr, "FIX:  The generic adaptor only supports up to 2 quads\n");
		    assert(0);
		  }
	      }
	    memcpy (hdr->buffer(), buf, size);

	    int rc = -1;

	    hdr->_req = (MPI_Request*)malloc(sizeof(MPI_Request)*nranks);
	    hdr->_num = nranks;
	    if(cb_done)
	      hdr->_cb_done = *cb_done;
	    else
	      hdr->_cb_done.function = NULL;

	    assert(hdr->_req != NULL);
	    for (unsigned count = 0; count < nranks; count ++) {
	      assert (hints[count] == CCMI_PT_TO_PT_SUBTASK);

	      rc = MPI_Isend (hdr,
			      hdr->totalsize(),
			      MPI_CHAR,
			      ranks[count],
			      (unsigned)this,
			      MPI_COMM_WORLD,
			      &hdr->_req[count]);
	      assert (rc == MPI_SUCCESS);
	    }
	    _g_sendreq_list.push_front(hdr);

	    //	    free (hdr);
	    //	    if ( cb_done && cb_done->function )
	    //	      (*cb_done->function)(cb_done->clientdata, NULL);
	    return rc;
	  }

	virtual unsigned send (MultiSend::CCMI_Multicast_t  *mcastinfo) {
	  //          printf ("Here New\n");
	  return MulticastImpl::send (mcastinfo->request,
				      &mcastinfo->cb_done,
				      mcastinfo->consistency,
				      mcastinfo->msginfo,
				      mcastinfo->connection_id,
				      mcastinfo->count,
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
				   CCMI_Dt                  dtype  = CCMI_UNDEFINED_DT ) { assert (0); }

	virtual unsigned postRecv (MultiSend::CCMI_MulticastRecv_t  *mrecv) { assert (0); }

	virtual void advance () {
	  //	  printf ("In multisend advance\n");

	  TRACE_ADVANCE((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::advance()\n", (int)this));
	  int flag = 0;
	  MPI_Status sts;


	  std::list<MsgHeader*>::iterator it;
	  for (it=_g_sendreq_list.begin();it != _g_sendreq_list.end(); it++)
	    {
	      int numStatuses = (*it)->_num;
	      flag            = 0;
	      MPI_Testall(numStatuses,(*it)->_req,&flag,MPI_STATUSES_IGNORE);
	      if(flag)
		{
		  if ((*it)->_cb_done.function )
		    (*(*it)->_cb_done.function)((*it)->_cb_done.clientdata, NULL);
		  free ((*it)->_req);
		  free (*it);
		  _g_sendreq_list.remove((*it));
		  break;
		}
	    }

	  flag = 0;




	  int rc = MPI_Iprobe (MPI_ANY_SOURCE, (unsigned)this, MPI_COMM_WORLD, &flag, &sts);
	  assert (rc == MPI_SUCCESS);

	  if (flag) 
	    {
	      int nbytes = 0;
	      MPI_Get_count(&sts, MPI_BYTE, &nbytes);
	      MsgHeader *msg = (MsgHeader *) malloc (nbytes);
	      int rc = MPI_Recv(msg,nbytes,MPI_BYTE,sts.MPI_SOURCE,sts.MPI_TAG, MPI_COMM_WORLD,&sts);

	      assert (rc == MPI_SUCCESS);

	      TRACE_ADVANCE((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::advance() received message\n", (int)this));

	      unsigned         rcvlen;
	      char           * rcvbuf;
	      unsigned         pwidth;
	      CCMI_Callback_t  cb_done;

	      _cb_async_head (&msg->_info[0], msg->_info_count, sts.MPI_SOURCE, msg->_size, msg->_conn,
			      _async_arg, &rcvlen, &rcvbuf, &pwidth, &cb_done);

	      if (rcvlen)
		memcpy (rcvbuf, msg->buffer(), rcvlen);

	      if(pwidth == 0 && rcvlen == 0)
		if (cb_done.function)
		  cb_done.function (cb_done.clientdata, NULL);

	      for (unsigned count = 0; count < rcvlen; count += pwidth)
		if (cb_done.function)
		  cb_done.function (cb_done.clientdata, NULL);

	      free (msg);
	    }

	  //	  printf ("After advance\n");
	}

      }; //Generic MPI Multicast
    };
  };
};


#endif
