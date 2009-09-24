/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/AsyncCompositeT.h
 * \brief ???
 */

#ifndef __ccmi_adaptor_broadcast_async_composite_t_h__
#define __ccmi_adaptor_broadcast_async_composite_t_h__

#include "algorithms/executor/Broadcast.h"
#include "algorithms/connmgr/RankBasedConnMgr.h"
#include "algorithms/protocols/broadcast/BroadcastFactory.h"
#include "algorithms/protocols/broadcast/ExecutorPool.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      ///
      /// \brief Asyc Broadcast Composite. It is single color right now
      ///
      template <class T_Schedule, class T_Sysdep, class T_Mcast, class T_ConnectionManager>
      class AsyncCompositeT : public CCMI::Executor::Composite
      {
      protected:
        CCMI::Executor::Broadcast<T_Sysdep, T_Mcast, T_ConnectionManager>    _executor __attribute__((__aligned__(16)));
        T_Schedule                            _schedule;
        BcastQueueElem               _bqelem;
        ExecutorPool               * _execpool;

      public:
        ///
        /// \brief Constructor
        ///
        AsyncCompositeT (T_Sysdep             * sd,
                         T_ConnectionManager  *cmgr,
                         XMI_Callback_t             cb_done,
                         xmi_consistency_t          consistency,
                         T_Mcast                   * mf,
                         XMI_GEOMETRY_CLASS        * geometry,
                         unsigned                    root,
                         char                      * src,
                         unsigned                    bytes,
                         ExecutorPool              * epool) :
        _executor (sd, geometry->comm(), cmgr, 0/*no color*/), _bqelem(this, root), _execpool(epool)
        {
          _executor.setMulticastInterface (mf);
          _executor.setInfo (root, bytes+1, src, bytes);
          _executor.setDoneCallback (cb_done.function, cb_done.clientdata);
          _executor.setConsistency (consistency);

          addExecutor (&_executor);

          COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
          create_schedule(&_schedule, sizeof(_schedule), root, sd, geometry);
          _executor.setSchedule (&_schedule);
        }

        ///
        /// \brief initialize the schedule based on input geometry.
        ///   Template implementation must specialize this function.
        ///
        void      create_schedule(void                      * buf,
                                  unsigned                    size,
                                  unsigned                    root,
                                  T_Sysdep                       * sd,
                                  XMI_GEOMETRY_CLASS                  * g) {CCMI_abort();};

        ExecutorPool *execpool ()
        {
          return _execpool;
        }
        BcastQueueElem *bqelem ()
        {
          return &_bqelem;
        }

        CCMI::Executor::Broadcast<T_Sysdep, T_Mcast, T_ConnectionManager> &executor()
        {
          return _executor;
        }

        ///
        /// \brief Receive the broadcast message and notify the executor
        ///
        static void staticAsyncRecvFn (void * context, void *clientdata, xmi_result_t err)
        {
          xmi_quad_t *info = NULL;
          AsyncCompositeT *composite = (AsyncCompositeT *) clientdata;

          TRACE_ADAPTOR((stderr, "In static notify recv, exe=%p\n",composite));
          composite->executor().CCMI::Executor::Broadcast<T_Sysdep, T_Mcast,T_ConnectionManager>::notifyRecv ((unsigned)-1, *info, NULL,
                                                                                          composite->executor().getPwidth());
        }

      }; //- AsyncCompositeT

      ///
      /// \brief Base factory class for broadcast factory implementations.
      ///
      template <class T_Schedule, AnalyzeFn afn, class T_Sysdep, class T_Mcast>
      class AsyncCompositeFactoryT : public BroadcastFactory<T_Sysdep, T_Mcast, CCMI::ConnectionManager::RankBasedConnMgr<T_Sysdep> >
      {
      protected:
        CCMI::ConnectionManager::RankBasedConnMgr<T_Sysdep>  _rbconnmgr; //Connection manager
        ExecutorPool                               _execpool;

      public:
        ///
        /// \brief Constructor for broadcast factory implementations.
        ///
        AsyncCompositeFactoryT
        (T_Sysdep *sd,
         T_Mcast  *mf, unsigned nconn)
          : BroadcastFactory<T_Sysdep, T_Mcast, CCMI::ConnectionManager::RankBasedConnMgr<T_Sysdep> >(mf,
                                                                                                      sd,
                                                                                                      &_rbconnmgr,
                                                                                                      nconn,
                                                                                                      (xmi_olddispatch_multicast_fn)cb_head,
                                                                                                      (xmi_olddispatch_multicast_fn)cb_head_buffered),
        _rbconnmgr(sd)
        {
          //--
          TRACE_ADAPTOR((stderr, "Binomial Broadcast Factory Constructor\n"));
        }

        ///Works on all sub-communicators
        virtual bool Analyze(XMI_GEOMETRY_CLASS *geometry)
        {
          return afn (geometry);
        }

        ///
        /// \brief Generate a non-blocking broadcast message.
        ///
        /// \param[in]  request      Opaque memory to maintain
        ///                          internal message state.
        /// \param[in]  cb_done      Callback to invoke when
        ///				 message is complete.
        /// \param[in]  consistency  Required consistency level
        /// \param[in]  geometry     Geometry to use for this
        ///				 collective operation.
        ///                          \c NULL indicates the global geometry.
        /// \param[in]  root         Rank of the node performing
        ///				 the broadcast.
        /// \param[in]  src          Source buffer to broadcast.
        /// \param[in]  bytes        Number of bytes to broadcast.
        ///
        /// \retval     0            Success
        /// \retval     1            Unavailable in geometry
        ///
        virtual CCMI::Executor::Composite * generate
        (void                      * request_buf,
         size_t                      rsize,
         XMI_Callback_t              cb_done,
         xmi_consistency_t           consistency,
         XMI_GEOMETRY_CLASS        * geometry,
         unsigned                    root,
         char                      * src,
         unsigned                    bytes)
        {
          //fprintf (stderr, "Async Broadcast Generate %d, %d\n", sizeof(T_Schedule), sizeof(CCMI_Executor_t));
          T_Schedule* a_bcast = NULL;

          XMI_assert(rsize > sizeof(T_Schedule));

          if(this->_sd->mapping.task() == root)
          {
            a_bcast = new (request_buf)
                      T_Schedule ( this->_sd, &this->_rbconnmgr,
                          cb_done, consistency, this->_minterface,
                          geometry, root, src, bytes, &this->_execpool );
            a_bcast->executor().start();
          }
          else if(this->_isBuffered)
          {
            XMI::MatchQueue  & mqueue = geometry->asyncBcastUnexpQ();
            BcastQueueElem * elem = (BcastQueueElem *) mqueue.findAndDelete(root);

            /// Try to match in unexpected queue
            if(elem)
            {
              CCMI_assert (elem->bytes() == bytes);
              elem->setPosted(bytes, src, cb_done);

              if(elem->isFinished())
              {
                //The broadcast has arleady finished
                elem->completeUnexpected();
                ((T_Schedule*)elem->composite())->execpool()->freeAsync (elem, elem->bytes());
              }
            }
            /// not found post construct bcastqueue elem and executor and then
            /// queue it in posted queue
            else
            {
              XMI_Callback_t  cb_exec_done;
              cb_exec_done.function   = posted_done;
              cb_exec_done.clientdata = request_buf; //point to the executor

              //Create a new composite and post it to posted queue
              a_bcast = new (request_buf)
                        T_Schedule (this->_sd, &this->_rbconnmgr,
                           cb_exec_done, consistency, this->_minterface,
                           geometry, root, src, bytes, &this->_execpool);

              BcastQueueElem *elem = a_bcast->bqelem();
              elem->initPostMsg (bytes, src, cb_done);
              geometry->asyncBcastPostQ().pushTail (elem);
            }
          }

          return a_bcast;
        }

        static XMI_Request_t *   cb_head_buffered
        (const xmi_quad_t* info,
         unsigned          count,
         unsigned          peer,
         unsigned          sndlen,
         unsigned          conn_id,
         void            * arg,
         unsigned        * rcvlen,
         char           ** rcvbuf,
         unsigned        * pipewidth,
         XMI_Callback_t * cb_done)
        {
          TRACE_ADAPTOR ((stderr, "Broadcast Async Handler\n"));
          fprintf(stderr, "Broadcast Async Handler!\n");
          AsyncCompositeFactoryT *factory = (AsyncCompositeFactoryT *) arg;
          CollHeaderData *cdata = (CollHeaderData *) info;

          XMI_GEOMETRY_CLASS *geometry = (XMI_GEOMETRY_CLASS *)
	    factory->_cb_geometry(cdata->_comm);

          XMI::MatchQueue  &mqueue = geometry->asyncBcastPostQ();
          BcastQueueElem *elem = (BcastQueueElem *) mqueue.findAndDelete(cdata->_root);

          T_Schedule *bcast = NULL;

          if(!elem)
          {
            CCMI_Executor_t *exec_request;
            char *unexpbuf;
            factory->_execpool.allocateAsync (&exec_request, &unexpbuf, sndlen);

            XMI_Callback_t cb_exec_done;
            ///Handler which will have to free the above allocated buffer
            cb_exec_done.function = unexpected_done;
            cb_exec_done.clientdata = exec_request;

            COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T_Schedule));
            bcast = new (exec_request)
                    T_Schedule (factory->_sd, &factory->_rbconnmgr,
                       cb_exec_done, XMI_MATCH_CONSISTENCY, factory->_minterface,
                       geometry, cdata->_root, unexpbuf, sndlen, &factory->_execpool);

            elem = bcast->bqelem();
            elem->initUnexpMsg (sndlen, unexpbuf);
            geometry->asyncBcastUnexpQ().pushTail (elem);
          }
          else
          {
            CCMI_assert (elem->bytes() == sndlen);
            bcast = (T_Schedule *) elem->composite();
          }

          * pipewidth = sndlen + 1;
          //We only support sndlen == rcvlen
          * rcvlen  = sndlen;
          * rcvbuf  = elem->rcvbuf();
          //callback to be called by multisend interface. It will notify executor
          cb_done->function   = T_Schedule::staticAsyncRecvFn;
          cb_done->clientdata = bcast;

          return bcast->executor().getRecvRequest();
        }

        static void unexpected_done (void *context, void *cd, xmi_result_t err)
        {
          TRACE_ADAPTOR ((stderr, "Unexpected bcast done\n"));

          CCMI::Adaptor::Broadcast::BcastQueueElem *bqe =
          ((T_Schedule *) cd)->bqelem();

          bqe->setFinished();
          if(bqe->isPosted())
          {
            bqe->completeUnexpected();
            ((T_Schedule*) cd)->execpool()->freeAsync (bqe, bqe->bytes());
          }
        }

        static void posted_done (void *context, void *cd, xmi_result_t err)
        {
          TRACE_ADAPTOR ((stderr, "Posted bcast done\n"));

          CCMI::Adaptor::Broadcast::BcastQueueElem *bqe =
          ((T_Schedule *) cd)->bqelem();

          bqe->completePosted();
          //nothing needs to be freed as this is an application buffer
        }

        static XMI_Request_t *   cb_head  (const xmi_quad_t    * info,
                                           unsigned          count,
                                           unsigned          peer,
                                           unsigned          sndlen,
                                           unsigned          conn_id,
                                           void            * arg,
                                           unsigned        * rcvlen,
                                           char           ** rcvbuf,
                                           unsigned        * pipewidth,
                                           XMI_Callback_t * cb_done)
          {
            TRACE_ADAPTOR ((stderr, "Broadcast Async Handler\n"));
            AsyncCompositeFactoryT *factory = (AsyncCompositeFactoryT *) arg;
          CollHeaderData *cdata = (CollHeaderData *) info;

          XMI_GEOMETRY_CLASS *geometry = (XMI_GEOMETRY_CLASS *)
	    factory->_cb_geometry(cdata->_comm);

          //Application callback
	  // todo:  WARNING:  callback is not appropriately delivered to user (header for example)
          XMI_Callback_t cb_client_done;
          xmi_type_t     user_type;
//          CCMI_Executor_t *request =  (CCMI_Executor_t *)
          factory->_cb_async (NULL,                 // Context: NULL for now, until we can get the context
                              cdata->_root,         // Root
                              (void*)cdata->_comm,  // Geometry
                              sndlen,               // sndlen
                              NULL,                 // User header, NULL for now
                              0,                    // 0 sized for now
                              (void**)rcvbuf,       // receive buffer
                              &user_type,           // type
                              (size_t*)rcvlen,      // receive length
                              &cb_client_done.function,
                              &cb_client_done.clientdata);
	  // todo:  WARNING:  we need to make sure this storage is freed!
          CCMI_Executor_t *request = (CCMI_Executor_t *)malloc(sizeof(*request));

          *pipewidth = sndlen+1;

          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T_Schedule));
          T_Schedule *bcast = new (request)
                     T_Schedule (factory->_sd, &factory->_rbconnmgr,
                        cb_client_done, XMI_MATCH_CONSISTENCY, factory->_minterface,
                        geometry, cdata->_root, *rcvbuf, *rcvlen, &factory->_execpool);

          //callback to be called by multisend interface
          cb_done->function = T_Schedule::staticAsyncRecvFn;
          cb_done->clientdata = bcast;

          ///Support single color async broadcasts
          return bcast->executor().getRecvRequest();
        }

      }; //- Async Composite Factory
    };  //- end namespace Broadcast
  };  //- end namespace Adaptor
};  //- end CCMI


#endif
