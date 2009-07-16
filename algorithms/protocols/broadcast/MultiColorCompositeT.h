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
#include "algorithms/composite/Composite.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/connmgr/RankBasedConnMgr.h"
#include "interface/Geometry.h"
#include "./BroadcastFactory.h"
//#include "algorithms/schedule/Rectangle.h"

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
      typedef void      (*PWColorsFn) (Geometry                  * g,
                                       unsigned                    bytes,
                                       CCMI::Schedule::Color     * colors,
                                       unsigned                  & ncolors,
                                       unsigned                  & pwidth);

      ///
      /// \brief Receive the broadcast message and notify the executor
      ///
      static void staticRecvFn (void *executor, CM_Error_t *err)
      {
        CMQuad *info = NULL;

        CCMI::Executor::Broadcast *exe =
        (CCMI::Executor::Broadcast *) executor;

        TRACE_ADAPTOR((stderr, "In static notify recv, exe=%p\n",exe));

        exe->notifyRecv ((unsigned)-1, *info, NULL, exe->getPwidth());
      }

      ///
      ///  \brief Base class for synchronous broadcasts
      ///
      template <int NUMCOLORS, class S, PWColorsFn pwcfn, class MAP>
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
        CM_Callback_t                               _cb_done;

        ///
        /// \brief Pointer to mapping
        ///
        MAP                              *  _mapping;

        CCMI::Executor::Broadcast                     _executors  [NUMCOLORS] __attribute__((__aligned__(16)));
        S                                             _schedules  [NUMCOLORS];
        char *                                        _srcbufs    [NUMCOLORS];
        unsigned                                      _bytecounts [NUMCOLORS];

      public:
        MultiColorCompositeT () : CCMI::Executor::Composite (), _doneCount(0), _nComplete(0)
        {
        }

        MultiColorCompositeT (MAP                              * map,
                              CCMI::ConnectionManager::ConnectionManager * cmgr,
                              CM_Callback_t                              cb_done,
                              CCMI_Consistency                             consistency,
                              CCMI::MultiSend::OldMulticastInterface        * mf,
                              Geometry                                   * geometry,
                              unsigned                                     root,
                              char                                       * src,
                              unsigned                                     bytes) : 
        CCMI::Executor::Composite (), _doneCount(0), _numColors(0), _cb_done(cb_done), _mapping(map)
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
            CCMI::Executor::Broadcast *bcast  = 
            new (& _executors[c]) CCMI::Executor::Broadcast (map, 
                                                             geometry->comm(), 
                                                             cmgr, 
                                                             _colors[c], 
                                                             true);

            bcast->setMulticastInterface (mf);
            bcast->setInfo (root, _pipewidth, _srcbufs[c], _bytecounts[c]);
            bcast->setConsistency (consistency);
            bcast->setDoneCallback (cb_bcast_done, this);

            addExecutor (bcast);
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
                                 Geometry                  * g,
                                 CCMI::Schedule::Color       color) {CCMI_abort();};
        void setDoneCallback (CM_Callback_t  cb_done) { _cb_done = cb_done;}

        void SyncBcastPost(Geometry                                     * geometry, 
                           unsigned                                       root,
                           CCMI::ConnectionManager::ConnectionManager   * cmgr,
                           CCMI::MultiSend::OldMulticastInterface          * minterface)
        {
          if(_mapping->rank() != root)
          { //post receive on non root nodes
            //posts a receive on connection given by connection
            //mgr, bcast connmgrs shouldnt care about phases

            CCMI::MultiSend::CCMI_OldMulticastRecv_t  mrecv;
            mrecv.cb_done.function = staticRecvFn;
            mrecv.pipelineWidth = _pipewidth;
            mrecv.opcode = CCMI_PT_TO_PT_SUBTASK;

            for(unsigned c = 0; c < _numColors; c++)
            {
              mrecv.bytes   = _bytecounts[c];
              mrecv.rcvbuf  = _srcbufs[c];
              mrecv.request = _executors[c].getRecvRequest();       
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
        static void cb_barrier_done(void *me, CM_Error_t *err)
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
            bcast_composite->_cb_done.function(bcast_composite->_cb_done.clientdata,NULL);
          }
        }

        static void cb_bcast_done(void *me, CM_Error_t *err)
        {
          MultiColorCompositeT * bcast_composite = (MultiColorCompositeT *) me;
          CCMI_assert (bcast_composite != NULL);    

          CCMI_assert (bcast_composite->_doneCount <  bcast_composite->_nComplete);   
          ++bcast_composite->_doneCount;
          if(bcast_composite->_doneCount == bcast_composite->_nComplete) // call users done function
          {
            bcast_composite->_cb_done.function(bcast_composite->_cb_done.clientdata, NULL);
          }
        } 
      };  //-- MultiColorCompositeT


      ///
      /// \brief Base factory class for broadcast factory implementations.
      ///
      template <class B, AnalyzeFn afn, class MAP>
      class MultiColorBroadcastFactoryT : public BroadcastFactory<MAP> 
      {
      public:

        ///
        /// \brief Constructor for broadcast factory implementations.
        ///
        MultiColorBroadcastFactoryT
        (MAP *map,
         CCMI::MultiSend::OldMulticastInterface *mf,
         CCMI::ConnectionManager::ConnectionManager *cmgr,
         unsigned nconn)
        : BroadcastFactory<MAP> (mf, map, cmgr, nconn)
        {
        }

        virtual bool Analyze(Geometry *geometry)
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
         CM_Callback_t             cb_done,
         CCMI_Consistency            consistency,
         Geometry                  * geometry,
         unsigned                    root,
         char                      * src,
         unsigned                    bytes)
        {
          B  *composite = 
          new (request_buf, rsize)
          B (this->_mapping,
             this->_connmgr,
             cb_done,
             consistency, 
             this->_minterface,
             geometry, 
             root, 
             src, 
             bytes);

          composite->SyncBcastPost (geometry, root, this->_connmgr, this->_minterface); 

          CCMI::Executor::Executor *barrier = geometry->getBarrierExecutor();
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
