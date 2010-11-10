/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/OldMultiColorCompositeT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_broadcast_OldMultiColorCompositeT_h__
#define __algorithms_protocols_broadcast_OldMultiColorCompositeT_h__

#include "algorithms/executor/Broadcast.h"
#include "algorithms/executor/Barrier.h"
#include "algorithms/composite/Composite.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/connmgr/RankBasedConnMgr.h"
#include "algorithms/protocols/broadcast/BroadcastFactory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

#define MAX_BCAST_COLORS  (NUM_TORUS_DIMENSIONS * 2)

      ///
      /// \brief Get pipeline width and optimal colors based on bytes and schedule
      ///
      typedef void      (*PWColorsFn) (PAMI_GEOMETRY_CLASS                  * g,
                                       unsigned                    bytes,
                                       CCMI::Schedule::Color     * colors,
                                       unsigned                  & ncolors,
                                       unsigned                  & pwidth);



      ///
      ///  \brief Base class for synchronous broadcasts
      ///
      template <int NUMCOLORS, class S, PWColorsFn pwcfn, class T_Mcast, class T_ConnectionManager>
      class OldMultiColorCompositeT : public CCMI::Executor::OldComposite
      {
      protected:
        ///
        /// \brief number of done callbacks called
        ///
        unsigned                 _doneCount;
        unsigned                 _nComplete;
        unsigned                 _numColors;
        CCMI::Schedule::Color    _colors [NUMCOLORS];
        unsigned                 _pipewidth;

        ///
        ///  \brief Application callback to call when the broadcast has finished
        ///
        PAMI_Callback_t                               _cb_done;

        CCMI::Executor::OldBroadcast<T_Mcast,T_ConnectionManager>
                                                      _executors  [NUMCOLORS] __attribute__((__aligned__(16)));
        S                                             _schedules  [NUMCOLORS];
        char *                                        _srcbufs    [NUMCOLORS];
        unsigned                                      _bytecounts [NUMCOLORS];

      public:
        OldMultiColorCompositeT () : CCMI::Executor::OldComposite (), _doneCount(0), _nComplete(0)
        {
        }

      ///
      /// \brief Receive the broadcast message and notify the executor
      ///
        static void staticRecvFn(pami_context_t context, void *executor, pami_result_t err)
          {
            pami_quad_t *info = NULL;

            CCMI::Executor::OldBroadcast<T_Mcast,T_ConnectionManager> *exe =
              (CCMI::Executor::OldBroadcast<T_Mcast,T_ConnectionManager> *) executor;

            TRACE_ADAPTOR ((stderr, "<%p>Broadcast::OldMultiColorCompositeT::staticRecvFn() \n",exe));

            exe->notifyRecv ((unsigned)-1, *info, NULL, exe->getPwidth());
          }

        OldMultiColorCompositeT (T_ConnectionManager * cmgr,
                              PAMI_Callback_t                              cb_done,
                              pami_consistency_t                      consistency,
                              T_Mcast                              * mf,
                              PAMI_GEOMETRY_CLASS                                   * geometry,
                              unsigned                                     root,
                              char                                       * src,
                              unsigned                                     bytes) :
        CCMI::Executor::OldComposite (), _doneCount(0), _numColors(0), _cb_done(cb_done)
        {
          pwcfn (geometry, bytes, _colors, _numColors, _pipewidth);

          _srcbufs [0] = src;
          _bytecounts[0] = bytes;
          _nComplete = _numColors + 1;

          if(_numColors > 1)
          {
            unsigned aligned_bytes = (bytes/_numColors) & (0xFFFFFFF0);
            _bytecounts[0] =  aligned_bytes;
            for(unsigned c = 1; c < _numColors; ++c)
            {
              _bytecounts[c] = aligned_bytes;
              _srcbufs[c]    = (char *)((unsigned long)_srcbufs[c-1] + _bytecounts[c-1]);
            }
            _bytecounts[_numColors-1]  = bytes -  (aligned_bytes * ( _numColors - 1));
          }

          for(unsigned c = 0; c < _numColors; c++)
          {
            CCMI_assert (c < NUMCOLORS);
            CCMI::Executor::OldBroadcast<T_Mcast,T_ConnectionManager> *bcast  =
            new (& _executors[c]) CCMI::Executor::OldBroadcast<T_Mcast,T_ConnectionManager> (geometry->comm(),
                                                                                                   cmgr,
                                                                                                   _colors[c],
                                                                                                   true);

            bcast->setMulticastInterface (mf);
            bcast->setInfo (root, _pipewidth, _srcbufs[c], _bytecounts[c]);
            bcast->setConsistency (consistency);
            bcast->setDoneCallback (cb_bcast_done, this);

            addExecutor (bcast);
            COMPILE_TIME_ASSERT(sizeof(_schedules[0]) >= sizeof(S));
            create_schedule(&_schedules[c], sizeof(_schedules[c]), geometry, _colors[c]);
            bcast->setSchedule (&_schedules[c]);
          }
        }

        ///
        /// \brief initialize the schedule based on input geometry.
        ///   Template implementation must specialize this function.
        ///

        void      create_schedule(void                      * buf,
                                 unsigned                    size,
                                 PAMI_GEOMETRY_CLASS                  * g,
                                 CCMI::Schedule::Color       color) {CCMI_abort();};
        void setDoneCallback (PAMI_Callback_t  cb_done) { _cb_done = cb_done;}

        void SyncBcastPost(PAMI_GEOMETRY_CLASS    * geometry,
                           unsigned                root,
                           T_ConnectionManager   * cmgr,
                           T_Mcast               * minterface)
        {
          if(__global.mapping.task() != root)
          { //post receive on non root nodes
            //posts a receive on connection given by connection
            //mgr, bcast connmgrs shouldnt care about phases

            pami_oldmulticast_recv_t  mrecv;
            mrecv.cb_done.function = staticRecvFn;
            mrecv.pipelineWidth = _pipewidth;
            mrecv.opcode = PAMI_PT_TO_PT_SUBTASK;

            for(unsigned c = 0; c < _numColors; c++)
            {
              mrecv.bytes   = _bytecounts[c];
              mrecv.rcvbuf  = _srcbufs[c];
              mrecv.request = (pami_quad_t*)_executors[c].getRecvRequest();
              mrecv.connection_id = cmgr->getConnectionId (geometry->comm(), root, _colors[c],
                                                           (unsigned)-1, (unsigned)-1);
              mrecv.cb_done.clientdata = &_executors[c];
              minterface->postRecv (&mrecv);
            }
          }
        }

        ///
        /// \brief For sync broadcasts, the done call back to be called
        ///        when barrier finishes
        ///
        static void cb_barrier_done(pami_context_t context, void *me, pami_result_t err)
        {
          OldMultiColorCompositeT * bcast_composite = (OldMultiColorCompositeT *) me;
          CCMI_assert (bcast_composite != NULL);

          for(unsigned i=0; i < bcast_composite->_numColors; ++i)
          {
            bcast_composite->getExecutor(i)->start();
          }

          CCMI_assert (bcast_composite->_doneCount <  bcast_composite->_nComplete);
          ++bcast_composite->_doneCount;
          if(bcast_composite->_doneCount == bcast_composite->_nComplete) // call users done function
          {
            bcast_composite->_cb_done.function(NULL, bcast_composite->_cb_done.clientdata,PAMI_SUCCESS);
          }
        }

        static void cb_bcast_done(pami_context_t context, void *me, pami_result_t err)
        {
          OldMultiColorCompositeT * bcast_composite = (OldMultiColorCompositeT *) me;
          CCMI_assert (bcast_composite != NULL);

          CCMI_assert (bcast_composite->_doneCount <  bcast_composite->_nComplete);
          ++bcast_composite->_doneCount;

          if(bcast_composite->_doneCount == bcast_composite->_nComplete) // call users done function
          {
            bcast_composite->_cb_done.function(context, bcast_composite->_cb_done.clientdata, PAMI_SUCCESS);
          }
        }
      };  //-- OldMultiColorCompositeT


      ///
      /// \brief Base factory class for broadcast factory implementations.
      ///
      typedef void      (*MetaDataFn)   (pami_metadata_t *m);
      template <class B, MetaDataFn get_metadata, class T_Mcast, class T_ConnectionManager>
      class OldMultiColorBroadcastFactoryT : public BroadcastFactory<T_Mcast, T_ConnectionManager>
      {
      public:

        ///
        /// \brief Constructor for broadcast factory implementations.
        ///
        OldMultiColorBroadcastFactoryT
        (T_Mcast *mf,
         T_ConnectionManager *cmgr,
         unsigned nconn)
          : BroadcastFactory<T_Mcast, T_ConnectionManager> (mf, cmgr, nconn)
        {
        }

        virtual void metadata(pami_metadata_t *mdata)
        {
            get_metadata(mdata);
          }

        class collObj
         {
        public:
          collObj(pami_xfer_t *xfer):
            _rsize(sizeof(_req)),
            _xfer(*xfer),
            _user_done_fn(xfer->cb_done),
            _user_cookie(xfer->cookie)
            {
              _xfer.cb_done = alloc_done_fn;
              _xfer.cookie  = this;
            }
          PAMI_Request_t                _req[5];
          int                          _rsize;
          pami_xfer_t                   _xfer;
          pami_event_function           _user_done_fn;
          void                       * _user_cookie;
        };

        static void alloc_done_fn( pami_context_t   context,
                                   void          * cookie,
                                   pami_result_t    result )
          {
            collObj *cObj = (collObj*)cookie;
            cObj->_user_done_fn(context,cObj->_user_cookie,result);
            free(cObj);
          }

        virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                               void                      * cmd)

          {
            collObj *obj = (collObj*)malloc(sizeof(*obj));
            new(obj) collObj((pami_xfer_t*)cmd);
            PAMI_Callback_t cb_done;
            cb_done.function   = obj->_xfer.cb_done;
            cb_done.clientdata = obj->_xfer.cookie;
            return this->generate(&obj->_req[0],
                                  obj->_rsize,
                                  cb_done,
                                  PAMI_MATCH_CONSISTENCY,
                                  (PAMI_GEOMETRY_CLASS *)geometry,
                                  obj->_xfer.cmd.xfer_broadcast.root,
                                  obj->_xfer.cmd.xfer_broadcast.buf,
                                  obj->_xfer.cmd.xfer_broadcast.typecount);
        }

        ///
        /// \brief Generate a non-blocking broadcast message.
        ///
        /// \param[in]  request      Opaque memory to maintain
        ///                          internal message state.
        /// \param[in]  cb_done      Callback to invoke when
        ///        message is complete.
        /// \param[in]  consistency  Required consistency level
        /// \param[in]  geometry     Geometry to use for this
        ///        collective operation.
        ///                          \c NULL indicates the global geometry.
        /// \param[in]  root         Rank of the node performing
        ///        the broadcast.
        /// \param[in]  src          Source buffer to broadcast.
        /// \param[in]  bytes        Number of bytes to broadcast.
        ///
        /// \retval     0            Success
        /// \retval     1            Unavailable in geometry
        ///

        virtual CCMI::Executor::Composite * generate
        (void                      * request_buf,
         size_t                      rsize,
         PAMI_Callback_t              cb_done,
         pami_consistency_t           consistency,
         PAMI_GEOMETRY_CLASS        * geometry,
         unsigned                    root,
         char                      * src,
         unsigned                    bytes)
        {
          PAMI_assert(rsize >= sizeof(B));
          PAMI_assert(request_buf);
          B  *composite =
          new (request_buf)
          B (this->_connmgr,
             cb_done,
             consistency,
             this->_minterface,
             geometry,
             root,
             src,
             bytes);
          PAMI_assert(composite);
          composite->SyncBcastPost (geometry, root, this->_connmgr, this->_minterface);
          CCMI::Executor::Composite *barrier = (CCMI::Executor::Composite*)
            geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                             PAMI::Geometry::CKEY_BARRIERCOMPOSITE0);
          CCMI_assert(barrier != NULL);

          barrier->setDoneCallback (B::cb_barrier_done, composite);
          barrier->start();

          return (CCMI::Executor::Composite *) composite;
        }

      };  //- MultiColorCompositeFactory
    };  //- end namespace Broadcast
  };  //- end namespace Adaptor
};  //- end CCMI

#endif
