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
#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"

#include "collectives/interface/MultiSendOld.h"
#include "collectives/interface/mpi/Adaptor.h"
#include "Table.h"


//extern int HL_Rank();
//#define TRACE(x) printf x; fflush(stdout);
#define TRACE(x) 


namespace CCMI {
  namespace Adaptor {
    namespace Generic {

      protected:
//      struct MsgHeader
//      {
//        CMQuad    _info;
//        int         _size;
//        int         _conn;
//
//        inline void *buffer() { return((char *)this + sizeof (MsgHeader));}
//        inline int  totalsize () { return _size + sizeof (MsgHeader);}
//      };
//
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
          TRACE_ADAPTOR((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::send()\n", (int)this));
          MsgHeader *hdr = (MsgHeader *)
                           CCMI_Alloc (sizeof (MsgHeader) + size);

          CCMI_assert( hdr != NULL );

          hdr->_info_count = info_count;
          hdr->_size = size;
          hdr->_conn = connection_id;
          if( info )
          {
            memcpy (&hdr->_info[0], info, info_count * sizeof (CMQuad));
            if(info_count > 2)
            {
              fprintf(stderr, "FIX:  The generic adaptor only supports up to 2 quads\n");
              CCMI_abort();
            }
          }
          memcpy (hdr->buffer(), buf, size);

          int rc = -1;

          hdr->_req = (MPI_Request*)CCMI_Alloc(sizeof(MPI_Request)*nranks);
          hdr->_num = nranks;
          if(cb_done)
            hdr->_cb_done = *cb_done;
          else
            hdr->_cb_done.function = NULL;

          CCMI_assert(hdr->_req != NULL);
          for(unsigned count = 0; count < nranks; count ++)
          {
            {
              TRACE_ADVANCE((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::send() %d byte to %d\n", (int)this, hdr->totalsize(),ranks[count]));
              CCMI_assert (hints[count] == CCMI_PT_TO_PT_SUBTASK);

              rc = MPI_Isend (hdr,
                              hdr->totalsize(),
                              MPI_CHAR,
                              ranks[count],
                              (unsigned long)this,
                              MPI_COMM_WORLD,
                              &hdr->_req[count]);
              CCMI_assert (rc == MPI_SUCCESS);
            }
            _g_sendreq_list.push_front(hdr);

            //	    CCMI_Free (hdr);
            //	    if ( cb_done && cb_done->function )
            //	      (*cb_done->function)(cb_done->clientdata, NULL);
            return rc;
          }
        }
          virtual unsigned send (MultiSend::CCMI_OldMulticast_t  *mcastinfo) {    
            //          printf ("Here New\n");
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

          virtual unsigned postRecv (CCMI_Request_t         * request,
                                     const CM_Callback_t  * cb_done,
                                     unsigned                 conn_id,
                                     char                   * buf,
                                     unsigned                 size,
                                     unsigned                 pwidth,
                                     unsigned                 hint   = CCMI_UNDEFINED_SUBTASK,
                                     CM_Op                  op     = CM_UNDEFINED_OP,
                                     CM_Dt                  dtype  = CM_UNDEFINED_DT ) { CCMI_abort();}

          virtual unsigned postRecv (MultiSend::CCMI_OldMulticastRecv_t  *mrecv) { CCMI_abort();}

          virtual void advance () {

            //	  printf ("In multisend advance\n");

            TRACE_ADVANCE((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::advance()\n", (int)this));
            int flag = 0;
            MPI_Status sts;


            std::list<MsgHeader*>::iterator it;
            for(it=_g_sendreq_list.begin();it != _g_sendreq_list.end(); it++)
            {
              int numStatuses = (*it)->_num;
              flag            = 0;
              MPI_Testall(numStatuses,(*it)->_req,&flag,MPI_STATUSES_IGNORE);
              if(flag)
              {
                if((*it)->_cb_done.function )
                  (*(*it)->_cb_done.function)((*it)->_cb_done.clientdata, NULL);
                CCMI_Free ((*it)->_req);
                CCMI_Free (*it);
                _g_sendreq_list.remove((*it));
                break;
              }
            }

            flag = 0;




            int rc = MPI_Iprobe (MPI_ANY_SOURCE, (unsigned long)this, MPI_COMM_WORLD, &flag, &sts);
            CCMI_assert (rc == MPI_SUCCESS);

            if(flag)
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              MsgHeader *msg = (MsgHeader *) CCMI_Alloc (nbytes);
              int rc = MPI_Recv(msg,nbytes,MPI_BYTE,sts.MPI_SOURCE,sts.MPI_TAG, MPI_COMM_WORLD,&sts);

              CCMI_assert (rc == MPI_SUCCESS);

              TRACE_ADVANCE((stderr, "<%#.8X>CCMI::Adaptor::Generic::MulticastImpl::advance() received message\n", (int)this));

              unsigned         rcvlen;
              char           * rcvbuf;
              unsigned         pwidth;
              CM_Callback_t  cb_done;

              _cb_async_head (&msg->_info[0], msg->_info_count, sts.MPI_SOURCE, msg->_size, msg->_conn,
                              _async_arg, &rcvlen, &rcvbuf, &pwidth, &cb_done);

              if(rcvlen)
                memcpy (rcvbuf, msg->buffer(), rcvlen);

              if(pwidth == 0 && rcvlen == 0)
                if(cb_done.function)
                  cb_done.function (cb_done.clientdata, NULL);

              for(unsigned count = 0; count < rcvlen; count += pwidth)
                if(cb_done.function)
                  cb_done.function (cb_done.clientdata, NULL);

              CCMI_Free (msg);
            }

            //	  printf ("After advance\n");
          }

        }; //Generic MPI Multicast


        ///
        /// \brief Callback function for unexpected async many to many
        /// operations
        ///
        typedef CCMI_Request_t * (*manytomany_recv) (unsigned          conn_id,
                                                     void            * arg,
                                                     char           ** rcvbuf,
                                                     unsigned       ** rcvdispls,
                                                     unsigned       ** rcvlens,
                                                     unsigned       **rcvcounters,
                                                     unsigned        * nranks,
                                                     CM_Callback_t * cb_done);



        class  ManytomanyImpl : public CCMI::MultiSend::ManytomanyInterface, public CCMI::Adaptor::Message
        {

        protected:

        public:

          ManytomanyImpl () : ManytomanyInterface (), Message() {}

          virtual ~ManytomanyImpl () {}

          /// NOTE: This is required to make "C" programs link successfully with virtual destructors
          inline void operator delete(void * p)
          {
            CCMI_abort();
          }

          inline void initialize (Adaptor *adaptor) {
            TRACE_ADAPTOR((stderr, "<%#.8X>CCMI::Adaptor::Generic::ManytomanyImpl::initialize() adaptor %#.8X\n", (int)this, (int)adaptor));
            adaptor->registerMessage (static_cast<CCMI::Adaptor::Message *>(this));
          }



          ///
          /// \brief Create a many-to-many message
          /// \param request : request object
          /// \param cb_done : Callback when the many-to-many is sent
          /// \param connid  : connection id
          /// \param rcvindex : which slot of the receiver displacements to use
          /// \param buf  : start of the buffer to scatter
          /// \param sizes : size of the many-to-many buffer
          /// \param offsets : offsets from the start of the buffer
          /// \param ranks   : Ranks of the destinations
          /// \param permutation : what order to sent the messages
          /// \param nranks  : Number of destinations
          ///

          virtual void send  (CCMI_Request_t         * request,
                              const CM_Callback_t  * cb_done,
                              unsigned                 connid,
                              unsigned                 rcvindex,
                              const char             * buf,
                              unsigned               * sizes,
                              unsigned               * offsets,
                              unsigned               * counters,
                              unsigned               * ranks,
                              unsigned               * permutation,
                              unsigned                 nranks)
          {
            unsigned i;

            TRACE_ADAPTOR(("%d: ManytomanyImpl::send() (%p)\n", HL_Rank(),this));

            M2MSendReq * m2m = (M2MSendReq *)CCMI_Alloc(sizeof(M2MSendReq));
            CCMI_assert( m2m != NULL );

            m2m->_conn = connid;

            if(cb_done)
              m2m->_cb_done = *cb_done;
            else
              m2m->_cb_done.function = NULL;

            m2m->_num = 0;
            m2m->_totalsize = 0;

            for( i = 0; i < nranks; i++)
            {
              if( sizes[i] == 0 )
                continue;

              m2m->_num++;
              m2m->_totalsize += (sizes[i] + sizeof(M2MHeader));
            }

            if( m2m->_num == 0 )
            {
              if( m2m->_cb_done.function )
                (*m2m->_cb_done.function)(m2m->_cb_done.clientdata,NULL);
              CCMI_Free ( m2m );
              return ;
            }

            m2m->_reqs = (MPI_Request *)CCMI_Alloc( m2m->_num * sizeof(MPI_Request));
            CCMI_assert ( m2m->_reqs != NULL );

            m2m->_bufs = ( char *)CCMI_Alloc( m2m->_totalsize );
            CCMI_assert ( m2m->_bufs != NULL );

            M2MHeader   * hdr = (M2MHeader *) m2m->_bufs;
            MPI_Request * req = m2m->_reqs;
            for( i = 0; i < nranks; i++)
            {
              int index = permutation[i];

              CCMI_assert ( index < nranks );

              if( sizes[index] == 0 ) continue;

              hdr->_size = sizes[index];
              hdr->_conn = connid;
              memcpy (hdr->buffer(), buf+offsets[index], sizes[index]);

              int rc = -1;

              TRACE_ADAPTOR(("%d: MPI_Isend to %d tag:%p conn:%d size:%d totalsize:%d\n",
                     HL_Rank(),ranks[index],this,hdr->_conn,hdr->_size,hdr->totalsize()));
              rc = MPI_Isend (hdr,
                              hdr->totalsize(),
                              MPI_CHAR,
                              ranks[index],
                              (unsigned long)this,
                              MPI_COMM_WORLD,
                              req);
              CCMI_assert (rc == MPI_SUCCESS);

              hdr = (M2MHeader *)((char *)hdr + hdr->totalsize());
              req++;
            }

            _g_m2m_sendreq_list.push_front(m2m);

            return;
          }


          virtual void postRecv (CCMI_Request_t         * request,
                                 const CM_Callback_t  * cb_done,
                                 unsigned                 connid,
                                 char                   * buf,
                                 unsigned               * sizes,
                                 unsigned               * offsets,
                                 unsigned               * counters,
                                 unsigned                 nranks,
                                 unsigned                 myindex)
          {
            TRACE_ADAPTOR(("%d: CCMI::Adaptor::Generic::ManytomanyImpl::postRecv() (%p)\n", HL_Rank(),this));

            std::list<M2MRecvReq*>::iterator it;
            for(it=_g_m2m_recvreq_list.begin();it != _g_m2m_recvreq_list.end(); it++)
            {
              if( (*it)->_conn == connid ) break;
            }

            CCMI_assert ( it == _g_m2m_recvreq_list.end() );

            create_recvreq( connid, cb_done, buf, sizes, offsets, nranks );

            return;
          }


          virtual void advance () {
            //	  printf ("In manytomany advance\n");

            TRACE_ADAPTOR(("%d: ManytomanyImpl::advance() (tag:%p)\n", HL_Rank(),this));
            int flag = 0;
            MPI_Status sts;

            std::list<M2MSendReq*>::iterator it;
            for(it=_g_m2m_sendreq_list.begin();it != _g_m2m_sendreq_list.end(); it++)
            {
              int numStatuses = (*it)->_num;
              flag            = 0;
              MPI_Testall(numStatuses,(*it)->_reqs,&flag,MPI_STATUSES_IGNORE);
              if(flag)
              {
                if((*it)->_cb_done.function )
                  (*(*it)->_cb_done.function)((*it)->_cb_done.clientdata, NULL);

                CCMI_Free ((*it)->_reqs);
                CCMI_Free ((*it)->_bufs);

                _g_m2m_sendreq_list.remove((*it));

                CCMI_Free (*it);
                break;
              }
            }

            flag = 0;

            int rc = MPI_Iprobe (MPI_ANY_SOURCE, 
                                 (unsigned long)this,
                                 MPI_COMM_WORLD, 
                                 &flag, 
                                 &sts);

            CCMI_assert (rc == MPI_SUCCESS);

            if(flag)
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              M2MHeader *msg = (M2MHeader *) CCMI_Alloc (nbytes);
              int rc = MPI_Recv(msg,nbytes,MPI_BYTE,sts.MPI_SOURCE,sts.MPI_TAG, MPI_COMM_WORLD,&sts);

              CCMI_assert (rc == MPI_SUCCESS);

              TRACE_ADAPTOR(("%d: ManytomanyImpl::advance() received message from:%d connid:%d size:%d nbytes:%d\n", HL_Rank(),sts.MPI_SOURCE,msg->_conn,msg->_size,nbytes));


              std::list<M2MRecvReq*>::iterator it;
              for(it=_g_m2m_recvreq_list.begin();it != _g_m2m_recvreq_list.end(); it++)
              {
                if( (*it)->_conn == msg->_conn ) break;
              }

              M2MRecvReq * m2m;
              if( it == _g_m2m_recvreq_list.end() )
              {
                CM_Callback_t   cb_done;
                char            * buf;
                unsigned        * sizes;
                unsigned        * offsets;
                unsigned        * rcvcounters;
                unsigned          nranks;

                TRACE_ADAPTOR(("%d: Manytomany async callback\n",HL_Rank()));

                CCMI_assert ( _cb_m2m_head );

                _cb_m2m_head( msg->_conn, 
                                _async_arg, 
                                &buf, 
                                &offsets, 
                                &sizes, 
                                &rcvcounters, 
                                &nranks, 
                                &cb_done );

                m2m  = create_recvreq( msg->_conn, 
                                       &cb_done, 
                                       buf, 
                                       sizes, 
                                       offsets, 
                                       nranks );
              }
              else
              {
                m2m = (*it);
              }

              unsigned src = sts.MPI_SOURCE;


              if( m2m )
              {
                TRACE_ADAPTOR(("%d: Manytomany advance received from %d num:%d msgsize:%d recvsize:%d offset:%d rcvbuf:%p\n",
                       HL_Rank(),
                       src,m2m->_num,
                       msg->_size,
                       m2m->_sizes[src],
                       m2m->_offsets[src],
                       m2m->_buf));
                unsigned size = msg->_size < m2m->_sizes[src] ? msg->_size : m2m->_sizes[src]; 
                CCMI_assert( size > 0 );
                memcpy( m2m->_buf + m2m->_offsets[src], msg->buffer(), size );
                m2m->_num--;

                if( m2m->_num == 0 )
                {
                  if( m2m->_cb_done.function )
                    (*m2m->_cb_done.function)(m2m->_cb_done.clientdata,NULL);

                  _g_m2m_recvreq_list.remove(m2m);
                  CCMI_Free ( m2m );
                }
              }

              CCMI_Free ( msg );
            }
            //	  printf ("After advance\n");
          }


        private:

          M2MRecvReq * create_recvreq( unsigned                 connid,
                                       const CM_Callback_t  * cb_done,
                                       char                   * buf,
                                       unsigned               * sizes,
                                       unsigned               * offsets,
                                       unsigned                 nranks )
          {
            M2MRecvReq * m2m = (M2MRecvReq *)CCMI_Alloc( sizeof (M2MRecvReq) );
            CCMI_assert ( m2m != NULL );

            m2m->_conn = connid;

            if(cb_done)
              m2m->_cb_done = *cb_done;
            else
              m2m->_cb_done.function = NULL;

            m2m->_num = 0;

            for( unsigned i = 0; i < nranks; i++)
            {
              if( sizes[i] == 0 ) continue;

              m2m->_num++;
            }

            if( m2m->_num == 0 )
            {
              if( m2m->_cb_done.function )
                (*m2m->_cb_done.function)(m2m->_cb_done.clientdata,NULL);
              CCMI_Free ( m2m );
              return NULL;
            }

            m2m->_buf     = buf;
            m2m->_sizes   = sizes;
            m2m->_offsets = offsets;
            m2m->_nranks  = nranks;

            _g_m2m_recvreq_list.push_front(m2m);

            return m2m;
          }


        };
      };
    };
  };
};

#undef TRACE

#endif

