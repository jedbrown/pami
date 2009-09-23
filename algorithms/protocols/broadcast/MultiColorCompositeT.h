/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/MultiColorCompositeT.h
 * \brief ???
 */

#ifndef __ccmi_adaptor_broadcast_multicolor_sync_composite_h__
#define __ccmi_adaptor_broadcast_multicolor_sync_composite_h__

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
      typedef void      (*PWColorsFn) (XMI_GEOMETRY_CLASS                  * g,
                                       unsigned                    bytes,
                                       CCMI::Schedule::Color     * colors,
                                       unsigned                  & ncolors,
                                       unsigned                  & pwidth);



      ///
      ///  \brief Base class for synchronous broadcasts
      ///
//      <class T_Sysdep, class T_Mcast, class T_ConnectionManager>
      template <int NUMCOLORS, class S, PWColorsFn pwcfn, class T_Sysdep, class T_Mcast, class T_ConnectionManager>
      class MultiColorCompositeT : public CCMI::Executor::Composite
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
        XMI_Callback_t                               _cb_done;

        ///
        /// \brief Pointer to mapping
        ///
        T_Sysdep                              *  _sd;

        CCMI::Executor::Broadcast<T_Sysdep,T_Mcast,T_ConnectionManager>
                                                      _executors  [NUMCOLORS] __attribute__((__aligned__(16)));
        S                                             _schedules  [NUMCOLORS];
        char *                                        _srcbufs    [NUMCOLORS];
        unsigned                                      _bytecounts [NUMCOLORS];

      public:        
        MultiColorCompositeT () : CCMI::Executor::Composite (), _doneCount(0), _nComplete(0)
        {
        }

      ///
      /// \brief Receive the broadcast message and notify the executor
      ///
        static void staticRecvFn (void *ctxt, void *executor, xmi_result_t err)
          {
            xmi_quad_t *info = NULL;
            
            CCMI::Executor::Broadcast<T_Sysdep,T_Mcast,T_ConnectionManager> *exe =
              (CCMI::Executor::Broadcast<T_Sysdep,T_Mcast,T_ConnectionManager> *) executor;
            
            TRACE_ADAPTOR((stderr, "In static notify recv, exe=%p\n",exe));
            
            exe->notifyRecv ((unsigned)-1, *info, NULL, exe->getPwidth());
          }
        
        MultiColorCompositeT (T_Sysdep                              * map,
                              T_ConnectionManager * cmgr,
                              XMI_Callback_t                              cb_done,
                              xmi_consistency_t                      consistency,
                              T_Mcast                              * mf,
                              XMI_GEOMETRY_CLASS                                   * geometry,
                              unsigned                                     root,
                              char                                       * src,
                              unsigned                                     bytes) :
        CCMI::Executor::Composite (), _doneCount(0), _numColors(0), _cb_done(cb_done), _sd(map)
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
            CCMI::Executor::Broadcast<T_Sysdep,T_Mcast,T_ConnectionManager> *bcast  =
            new (& _executors[c]) CCMI::Executor::Broadcast<T_Sysdep,T_Mcast,T_ConnectionManager> (map,
                                                                                                   geometry->comm(),
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
                                 XMI_GEOMETRY_CLASS                  * g,
                                 CCMI::Schedule::Color       color) {CCMI_abort();};
        void setDoneCallback (XMI_Callback_t  cb_done) { _cb_done = cb_done;}

        void SyncBcastPost(XMI_GEOMETRY_CLASS    * geometry,
                           unsigned                root,
                           T_ConnectionManager   * cmgr,
                           T_Mcast               * minterface)
        {
          if(_sd->mapping.task() != root)
          { //post receive on non root nodes
            //posts a receive on connection given by connection
            //mgr, bcast connmgrs shouldnt care about phases

            xmi_oldmulticast_recv_t  mrecv;
            mrecv.cb_done.function = staticRecvFn;
            mrecv.pipelineWidth = _pipewidth;
            mrecv.opcode = XMI_PT_TO_PT_SUBTASK;

            for(unsigned c = 0; c < _numColors; c++)
            {
              mrecv.bytes   = _bytecounts[c];
              mrecv.rcvbuf  = _srcbufs[c];
              mrecv.request = (xmi_quad_t*)_executors[c].getRecvRequest();
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
        static void cb_barrier_done(void *ctxt, void *me, xmi_result_t err)
        {
          MultiColorCompositeT * bcast_composite = (MultiColorCompositeT *) me;
          CCMI_assert (bcast_composite != NULL);

          for(unsigned i=0; i < bcast_composite->_numColors; ++i)
          {
            bcast_composite->getExecutor(i)->start();
          }

          CCMI_assert (bcast_composite->_doneCount <  bcast_composite->_nComplete);
          ++bcast_composite->_doneCount;
          if(bcast_composite->_doneCount == bcast_composite->_nComplete) // call users done function
          {
            bcast_composite->_cb_done.function(NULL, bcast_composite->_cb_done.clientdata,XMI_SUCCESS);
          }
        }

        static void cb_bcast_done(void *ctxt, void *me, xmi_result_t err)
        {
          MultiColorCompositeT * bcast_composite = (MultiColorCompositeT *) me;
          CCMI_assert (bcast_composite != NULL);

          CCMI_assert (bcast_composite->_doneCount <  bcast_composite->_nComplete);
          ++bcast_composite->_doneCount;
          
          if(bcast_composite->_doneCount == bcast_composite->_nComplete) // call users done function
          {
            bcast_composite->_cb_done.function(ctxt, bcast_composite->_cb_done.clientdata, XMI_SUCCESS);
          }
        }
      };  //-- MultiColorCompositeT


      ///
      /// \brief Base factory class for broadcast factory implementations.
      ///
      template <class B, AnalyzeFn afn, class T_Sysdep,class T_Mcast, class T_ConnectionManager>
      class MultiColorBroadcastFactoryT : public BroadcastFactory<T_Sysdep, T_Mcast, T_ConnectionManager>
      {
      public:

        ///
        /// \brief Constructor for broadcast factory implementations.
        ///
        MultiColorBroadcastFactoryT
        (T_Sysdep *map,
         T_Mcast *mf,
         T_ConnectionManager *cmgr,
         unsigned nconn)
          : BroadcastFactory<T_Sysdep, T_Mcast, T_ConnectionManager> (mf, map, cmgr, nconn)
        {
        }

        virtual bool Analyze(XMI_GEOMETRY_CLASS *geometry)
        {
          return  afn(geometry);
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
          XMI_assert(rsize >= sizeof(B));
          B  *composite =
          new (request_buf)
          B (this->_sd,
             this->_connmgr,
             cb_done,
             consistency,
             this->_minterface,
             geometry,
             root,
             src,
             bytes);
          composite->SyncBcastPost (geometry, root, this->_connmgr, this->_minterface);
          CCMI::Executor::OldBarrier<T_Mcast> *barrier = (CCMI::Executor::OldBarrier<T_Mcast>*)
            geometry->getKey(XMI::Geometry::XMI_GKEY_BARRIEREXECUTOR);
          CCMI_assert(barrier != NULL);

          barrier->setDoneCallback (B::cb_barrier_done, composite);
          barrier->setConsistency (consistency);
          barrier->start();

          return composite;
        }

      };  //- MultiColorCompositeFactory
    };  //- end namespace Broadcast
  };  //- end namespace Adaptor
};  //- end CCMI

#endif
