/**
 * \file adaptor/generic/multisend_impl.h
 * \brief ???
 */

#ifndef   __mpi_multisend_impl_h__
#define   __mpi_multisend_impl_h__

#include <mpi.h>
#include <assert.h>

#include <interfaces/MultiSend.h>
#include <Adaptor.h>
//#warning generic multisend implementation
namespace CCMI
{
  namespace Adaptor
  {
    namespace Generic
    {

      typedef CCMI_Request_t * (*msend_recv) (const CMQuad  * info,
                                              unsigned          count,
                                              unsigned          peer,
                                              unsigned          sndlen,
                                              unsigned          conn_id,
                                              void            * arg,
                                              unsigned        * rcvlen,
                                              char           ** rcvbuf,
                                              unsigned        * pipewidth,
                                              CM_Callback_t * cb_done);


      class  MulticastImpl : public CCMI::MultiSend::OldMulticastInterface, public CCMI::Adaptor::Message
      {

      protected:
        struct MsgHeader
        {
          CMQuad    _info;
          int         _size;
          int         _conn;

          inline void *buffer() { return((char *)this + sizeof (MsgHeader));} 
          inline int  totalsize () { return _size + sizeof (MsgHeader);} 
        };

      public:

        MulticastImpl () : OldMulticastInterface (), Message() {}

        virtual ~MulticastImpl () {}

        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        inline void operator delete(void * p)
        {
          CCMI_abort();
        }

        inline void initialize (Adaptor *adaptor) {
          TRACE_ADVANCE((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::initialize() adaptor %#.8X\n", (int)this, (int)adaptor));
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
                         const CM_Callback_t  * cb_done,
                         CCMI_Consistency         consistency,
                         const CMQuad         * info, 
                         unsigned                 connection_id,
                         const char             * buf, 
                         unsigned                 size,
                         unsigned               * hints,
                         unsigned               * ranks, 
                         unsigned                 nranks,
                         CM_Op                  op    = CM_UNDEFINED_OP,
                         CM_Dt                  dtype = CM_UNDEFINED_DT )

        {     
          MsgHeader *hdr = (MsgHeader *) 
                           CCMI_Alloc (sizeof (MsgHeader) + size);

          assert( hdr != NULL );

          hdr->_size = size;
          hdr->_conn = connection_id;
          if( info )
            memcpy (&hdr->_info, info, sizeof (CMQuad));
          memcpy (hdr->buffer(), buf, size);

          int rc = -1;

          for(unsigned count = 0; count < nranks; count ++)
          {
            TRACE_ADVANCE((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::send() %d byte to %d\n", (int)this, hdr->totalsize(),ranks[count]));
            assert (hints[count] == CCMI_PT_TO_PT_SUBTASK); 
            rc = MPI_Send (hdr, hdr->totalsize(), MPI_CHAR, ranks[count], (unsigned)this, MPI_COMM_WORLD);
            assert (rc == MPI_SUCCESS);
          }

          CCMI_Free (hdr);      

          if( cb_done && cb_done->function )
            (*cb_done->function)(cb_done->clientdata, NULL);
          return rc;
        }

        virtual unsigned send (MultiSend::CCMI_OldMulticast_t  *mcastinfo) {    
          //          printf ("Here New\n"); 
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
                                   const CM_Callback_t  * cb_done,
                                   unsigned                 conn_id,
                                   char                   * buf,
                                   unsigned                 size,
                                   unsigned                 pwidth,
                                   unsigned                 hint   = CCMI_UNDEFINED_SUBTASK,
                                   CM_Op                  op     = CM_UNDEFINED_OP,
                                   CM_Dt                  dtype  = CM_UNDEFINED_DT ) { assert (0);}

        virtual unsigned postRecv (MultiSend::CCMI_OldMulticastRecv_t  *mrecv) { assert (0);}

        virtual void advance () {

          //	  printf ("In multisend advance\n");

          TRACE_FLOW((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::advance()\n", (int)this));
          int flag = 0;
          MPI_Status sts;
          int rc = MPI_Iprobe (MPI_ANY_SOURCE, (unsigned)this, MPI_COMM_WORLD, &flag, &sts);
          assert (rc == MPI_SUCCESS);

          if(flag)
          {
            int nbytes = 0;
            MPI_Get_count(&sts, MPI_BYTE, &nbytes);
            MsgHeader *msg = (MsgHeader *) CCMI_Alloc (nbytes);
            int rc = MPI_Recv(msg,nbytes,MPI_BYTE,sts.MPI_SOURCE,sts.MPI_TAG, MPI_COMM_WORLD,&sts);

            assert (rc == MPI_SUCCESS);

            TRACE_ADVANCE((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::advance() received message (%d)\n", (int)this, nbytes));

            unsigned         rcvlen;
            char           * rcvbuf;
            unsigned         pwidth;
            CM_Callback_t  cb_done;

            _cb_async_head (&msg->_info, 1, sts.MPI_SOURCE, msg->_size, msg->_conn,
                            _async_arg, &rcvlen, &rcvbuf, &pwidth, &cb_done);

            if(rcvlen)
              memcpy (rcvbuf, msg->buffer(), rcvlen);

            for(unsigned count = 0; count < rcvlen; count += pwidth)
              if(cb_done.function)
                cb_done.function (cb_done.clientdata, NULL);

            CCMI_Free (msg);
          }

          //	  printf ("After advance\n");
        }

      }; //Generic MPI Multicast
    };
  };
};


#endif
