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
#include "interface/Geometry.h"
#include "algorithms/connmgr/RankBasedConnMgr.h"
#include "./BroadcastFactory.h"
#include "./ExecutorPool.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      ///
      /// \brief Asyc Broadcast Composite. It is single color right now
      ///
      template <class T, class MAP> 
      class AsyncCompositeT : public CCMI::Executor::Composite
      {
      protected:
        CCMI::Executor::Broadcast    _executor __attribute__((__aligned__(16)));
        T                            _schedule;
        BcastQueueElem               _bqelem;
        ExecutorPool               * _execpool;

      public: 
        ///
        /// \brief Constructor
        ///
        AsyncCompositeT (MAP             * map,
                         CCMI::ConnectionManager::ConnectionManager *cmgr,
                         XMI_Callback_t             cb_done,
                         CCMI_Consistency            consistency,
                         CCMI::MultiSend::OldMulticastInterface *mf,
                         Geometry                  * geometry,
                         unsigned                    root,
                         char                      * src,
                         unsigned                    bytes,
                         ExecutorPool              * epool) :
        _executor (map, geometry->comm(), cmgr, 0/*no color*/), _bqelem(this, root), _execpool(epool)
        {
          _executor.setMulticastInterface (mf);
          _executor.setInfo (root, bytes+1, src, bytes);
          _executor.setDoneCallback (cb_done.function, cb_done.clientdata);
          _executor.setConsistency (consistency);

          addExecutor (&_executor);

          COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T));
          create_schedule(&_schedule, sizeof(_schedule), root, map, geometry);
          _executor.setSchedule (&_schedule);
        }

        ///
        /// \brief initialize the schedule based on input geometry. 
        ///   Template implementation must specialize this function.
        ///
        void      create_schedule(void                      * buf,
                                  unsigned                    size,
                                  unsigned                    root,
                                  MAP                       * map,
                                  Geometry                  * g) {CCMI_abort();};

        ExecutorPool *execpool ()
        {
          return _execpool;
        }
        BcastQueueElem *bqelem ()
        {
          return &_bqelem;
        }
        CCMI::Executor::Broadcast &executor()
        {
          return _executor;
        }

        ///
        /// \brief Receive the broadcast message and notify the executor
        ///
        static void staticAsyncRecvFn (void *clientdata, XMI_Error_t *err)
        {
          XMIQuad *info = NULL;  
          AsyncCompositeT *composite = (AsyncCompositeT *) clientdata;

          TRACE_ADAPTOR((stderr, "In static notify recv, exe=%p\n",exe)); 
          composite->executor().CCMI::Executor::Broadcast::notifyRecv ((unsigned)-1, *info, NULL, 
                                                                       composite->executor().getPwidth());
        }

      }; //- AsyncCompositeT

      ///
      /// \brief Base factory class for broadcast factory implementations.
      ///
      template <class T, AnalyzeFn afn, class MAP>
      class AsyncCompositeFactoryT : public BroadcastFactory<MAP>
      {
      protected:
        CCMI::ConnectionManager::RankBasedConnMgr  _rbconnmgr; //Connection manager
        ExecutorPool                               _execpool;

      public:
        ///
        /// \brief Constructor for broadcast factory implementations.
        ///
        AsyncCompositeFactoryT
        (MAP *map,
         CCMI::MultiSend::OldMulticastInterface *mf, unsigned nconn)
        : BroadcastFactory<MAP>(mf, map, &_rbconnmgr, nconn, cb_head, cb_head_buffered),
        _rbconnmgr(map)
        {
          //--
          TRACE_ADAPTOR((stderr, "Binomial Broadcast Factory Constructor\n"));
        }

        ///Works on all sub-communicators
        virtual bool Analyze(Geometry *geometry)
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
         XMI_Callback_t             cb_done,
         CCMI_Consistency            consistency,
         Geometry                  * geometry,
         unsigned                    root,
         char                      * src,
         unsigned                    bytes) 
        {
          //fprintf (stderr, "Async Broadcast Generate %d, %d\n", sizeof(T), sizeof(CCMI_Executor_t));

          T* a_bcast = NULL;

          XMI_assert(rsize > sizeof(T));

          if(this->_mapping->rank() == root)
          {
            a_bcast = new (request_buf)
                      T ( this->_mapping, &this->_rbconnmgr,
                          cb_done, consistency, this->_minterface,
                          geometry, root, src, bytes, &this->_execpool );
            a_bcast->executor().start();
          }
          else if(this->_isBuffered)
          {
            CCMI::MatchQueue  & mqueue = geometry->asyncBcastUnexpQ();
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
                ((T*)elem->composite())->execpool()->freeAsync (elem, elem->bytes());
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
                        T (this->_mapping, &this->_rbconnmgr,
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
        (const XMIQuad    * info,
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

          CCMI::Adaptor::Geometry *geometry = (CCMI::Adaptor::Geometry *)
	    factory->_cb_geometry(cdata->_comm);
	  
          CCMI::MatchQueue  &mqueue = geometry->asyncBcastPostQ();
          BcastQueueElem *elem = (BcastQueueElem *) mqueue.findAndDelete(cdata->_root);

          T *bcast = NULL;

          if(!elem)
          {
            CCMI_Executor_t *exec_request;
            char *unexpbuf;
            factory->_execpool.allocateAsync (&exec_request, &unexpbuf, sndlen);

            XMI_Callback_t cb_exec_done;
            ///Handler which will have to free the above allocated buffer
            cb_exec_done.function = unexpected_done;
            cb_exec_done.clientdata = exec_request;

            COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T));
            bcast = new (exec_request)
                    T (factory->_mapping, &factory->_rbconnmgr,
                       cb_exec_done, CCMI_MATCH_CONSISTENCY, factory->_minterface,
                       geometry, cdata->_root, unexpbuf, sndlen, &factory->_execpool); 

            elem = bcast->bqelem();
            elem->initUnexpMsg (sndlen, unexpbuf);
            geometry->asyncBcastUnexpQ().pushTail (elem);
          }
          else
          {
            CCMI_assert (elem->bytes() == sndlen);
            bcast = (T *) elem->composite();
          }

          * pipewidth = sndlen + 1;
          //We only support sndlen == rcvlen
          * rcvlen  = sndlen;
          * rcvbuf  = elem->rcvbuf();
          //callback to be called by multisend interface. It will notify executor
          cb_done->function   = T::staticAsyncRecvFn; 
          cb_done->clientdata = bcast;

          return bcast->executor().getRecvRequest();
        }

        static void unexpected_done (void *cd, XMI_Error_t *err)
        {
          TRACE_ADAPTOR ((stderr, "Unexpected bcast done\n"));

          CCMI::Adaptor::Broadcast::BcastQueueElem *bqe = 
          ((T *) cd)->bqelem();   

          bqe->setFinished();
          if(bqe->isPosted())
          {
            bqe->completeUnexpected();    
            ((T*) cd)->execpool()->freeAsync (bqe, bqe->bytes());
          }
        }

        static void posted_done (void *cd, XMI_Error_t *err)
        {
          TRACE_ADAPTOR ((stderr, "Posted bcast done\n"));

          CCMI::Adaptor::Broadcast::BcastQueueElem *bqe = 
          ((T *) cd)->bqelem();   

          bqe->completePosted();
          //nothing needs to be freed as this is an application buffer
        }

        static XMI_Request_t *   cb_head   
        (const XMIQuad    * info,
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

          CCMI::Adaptor::Geometry *geometry = (CCMI::Adaptor::Geometry *)
	    factory->_cb_geometry(cdata->_comm);

          //Application callback
          XMI_Callback_t cb_client_done;  
          CCMI_Executor_t *request =  (CCMI_Executor_t *)
                                      factory->_cb_async (cdata->_root, cdata->_comm, sndlen, rcvlen,
                                                          rcvbuf, &cb_client_done);

          *pipewidth = sndlen+1;

          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T));
          T *bcast = new (request)
                     T (factory->_mapping, &factory->_rbconnmgr,
                        cb_client_done, CCMI_MATCH_CONSISTENCY, factory->_minterface,
                        geometry, cdata->_root, *rcvbuf, *rcvlen, &factory->_execpool);  

          //callback to be called by multisend interface
          cb_done->function = T::staticAsyncRecvFn;
          cb_done->clientdata = bcast;

          ///Support single color async broadcasts
          return bcast->executor().getRecvRequest();
        }

      }; //- Async Composite Factory
    };  //- end namespace Broadcast
  };  //- end namespace Adaptor
};  //- end CCMI


#endif
