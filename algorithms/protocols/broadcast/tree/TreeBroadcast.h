/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/tree/TreeBroadcast.h
 * \brief ???
 */

#ifndef __ccmi_adaptor_broadcast_tree_h__
#define __ccmi_adaptor_broadcast_tree_h__

#include "algorithms/schedule/TreeSchedule.h"
#include "algorithms/schedule/TreeBwSchedule.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/protocols/broadcast/multi_color_impl.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      namespace Tree
      {
        //This method is for schedules/executors that dont use pipelining
        inline void get_colors_tree (Geometry                  * g,
                                     unsigned                    bytes,
                                     CCMI::Schedule::Color     * colors,
                                     unsigned                  & ncolors,
                                     unsigned                  & pwidth)
        {
          ncolors = 1;
          colors[0] = CCMI::Schedule::NO_COLOR;

          static unsigned min_pipeline_width = MIN_PIPELINE_WIDTH;
          static char query_env = 1;

          //Move to MultiColorCompositeFactoryT class
          if(query_env)
          {
             /// \todo this should be in the glue or some higher level - not DCMF specific
            char* envopts = getenv("DCMF_BCAST_MIN_PW");
            if(envopts) min_pipeline_width = atoi(envopts);
            /// The pipeline width must be a multiple of 240 (DMA) and 256 (Tree)
            min_pipeline_width = min_pipeline_width/MIN_PIPELINE_WIDTH * MIN_PIPELINE_WIDTH;
            CCMI_assert(min_pipeline_width);
            query_env = 0;
          }

          // cts shows these values work well for tree/dput pipelining.
          // Tree/memfifo is not optimized (and is always best with MIN_PIPELINE_WIDTH)
          if((bytes) > 512*1024)
            pwidth = 32 * min_pipeline_width;
          else if((bytes) >= 128*1024)
            pwidth = 8 * min_pipeline_width;
          else if((bytes) >= 32*1024)
            pwidth = 2 * min_pipeline_width;
          else
            pwidth = min_pipeline_width;        
        }

        typedef MultiColorCompositeT<1, CCMI::Schedule::TreeBwSchedule, 
        get_colors_tree, CCMI::TorusCollectiveMapping>  TreeBaseComposite;

        ///
        /// \brief Tree broadcast protocol. We Enhance the MultiColorComposite with a 
        ///        new SyncBcastPost method 
        ///
        class TreeBcastComposite : public TreeBaseComposite
        {
        protected:
          bool                _toDummySend;
          bool                _toDummyRecv;      
          bool                _toAppRecv;
          unsigned            _appHints;

          CM_Callback_t     _cb_app_done;
          Geometry          * _geometry;

          CCMI_Request_t    _tmpRequest __attribute__((__aligned__(16)));

          ///
          /// \brief Post a dummy send to inject 0s into the collective network
          ///
          void postDummySend (CCMI::MultiSend::OldMulticastInterface  * minterface)
          {
            CM_Callback_t newcb = {cb_bcast_done, this};
            unsigned dstrank  = (unsigned)-1;  //Not the root, inject 0s
            unsigned nranks   = 1;
            unsigned opcode   = CCMI_TREE_BCAST;

            // posts a receive on connection given by connection mgr
            minterface->send (&_tmpRequest, &newcb,
                              CCMI_MATCH_CONSISTENCY, NULL, 1, 0, NULL, _bytecounts[0], 
                              &opcode, &dstrank, nranks, CM_UNDEFINED_OP, CM_UNDEFINED_DT);
          }

          ///
          /// \brief Post a dummy recv to pull data on the root's node
          ///
          void postDummyRecv (CCMI::MultiSend::OldMulticastInterface  * minterface)
          {
            CM_Callback_t newcb = {cb_bcast_done, this};
            unsigned  tconnid = 0;

            // posts a receive on connection given by connection mgr
            minterface->postRecv (&_tmpRequest, &newcb,
                                  tconnid, NULL, _bytecounts[0], _bytecounts[0],
                                  CCMI_BCAST_RECV_NOSTORE);
          }

          ///
          /// \brief Post the application receive to receive data and notify executor
          ///
          void postAppRecv (CCMI::MultiSend::OldMulticastInterface  * minterface)
          {
            //Support only broadcast 
            CM_Callback_t newcb = {staticRecvFn, getExecutor(0)};
            //bcast connection manger should be phase independent
            unsigned  tconnid = 0; 
            //cmgr->getConnectionId (_comm, _root, _color, (unsigned)-1, (unsigned)-1); 
            TRACE_ADAPTOR ((stderr, "Sync Broadcast post recv with bcast %x, len %d, connid %d\n",
                        (int)this, _buflen, tconnid));

            // posts a receive on connection given by connection mgr
            minterface->postRecv (_executors[0].getRecvRequest(), 
                                  &newcb, tconnid, _srcbufs[0], 
                                  _bytecounts[0], _pipewidth, _appHints);
          }


        public:
          ///
          /// \brief Constructor
          ///
          TreeBcastComposite (CCMI::TorusCollectiveMapping             * map,
                              CCMI::ConnectionManager::ConnectionManager *cmgr,
                              CM_Callback_t             cb_done,
                              CM_Callback_t             cb_app_done,
                              CCMI_Consistency            consistency,
                              CCMI::MultiSend::OldMulticastInterface *mf,
                              Geometry                  * geometry,
                              unsigned                    root,
                              char                      * src,
                              unsigned                    bytes)
          : TreeBaseComposite (map, cmgr, cb_done, consistency, mf, geometry, root,
                               src, bytes), 
          _toDummySend(false), _toDummyRecv(false), _toAppRecv(false), 
          _appHints(CCMI_PT_TO_PT_SUBTASK), _cb_app_done(cb_app_done), _geometry(geometry)
          {
          }       

          ///
          /// \brief Initialize flags to indicate if dummy sends/recvs have to be posted
          ///
          void initSMP (unsigned root)
          {
            _pipewidth = _bytecounts[0];  //Disable pipelning 
            _executors[0].setPipelineWidth (_bytecounts[0]);

            //I am the root	      
            if(_mapping->rank() == root)
              _toDummyRecv = true;
            else
            {
              _toDummySend = true;
              _toAppRecv   = true;
              _appHints    = CCMI_BCAST_RECV_STORE;
            }
            //Wait for the dummy message as well
            _nComplete ++;        
          }

          ///
          /// \brief Initialize flags to indicate if dummy sends/recvs have to be posted
          ///
          void initVnDual (unsigned root)
          {
            unsigned coords[CCMI_TORUS_NDIMS];
            _mapping->Rank2Torus (&(coords[0]), root);

            //Designated core that does extra work of communication threads in other modes
            if( _mapping->GetCoord(CCMI_T_DIM) ==
                ((coords[CCMI_T_DIM] + 1) & (_mapping->GetDimLength(CCMI_T_DIM) - 1)) )
            {
              //I am on root's node
              if(coords[CCMI_X_DIM] == _mapping->GetCoord(CCMI_X_DIM) &&
                 coords[CCMI_Y_DIM] == _mapping->GetCoord(CCMI_Y_DIM) &&
                 coords[CCMI_Z_DIM] == _mapping->GetCoord(CCMI_Z_DIM))
                _toDummyRecv = true;
              else
                _toDummySend = true;
              _nComplete ++;
            }

            if(_mapping->rank() != root)
            {
              _toAppRecv = true;  //need src hint from schedule????

              CCMI::Schedule::Schedule *schedule = &_schedules[0];
              int startphase = _executors[0].startphase();

              // Don't post for the tree - that's handled in TreeBroadcast,
              // so get the src pe and check the hints
              unsigned numSrcPes, srcPes[16], srcHints[16];
              schedule->getSrcPeList(startphase, srcPes, numSrcPes, srcHints);
              CCMI_assert(numSrcPes == 1); // bcast always receives data in startPhase
              _appHints = srcHints[0];
            }
          }

          ///
          /// \brief Post tree data and dummy receives
          ///
          void SyncBcastTreePost(CCMI::MultiSend::OldMulticastInterface          * minterface)
          {
            CCMI_assert (!_toDummySend || !_toDummyRecv); //Both tosend and torecv can't be true

            if(_toDummySend)  //Do dummy send
              postDummySend (minterface);

            if(_toDummyRecv) //do dummy recv
              postDummyRecv (minterface);

            if(_toAppRecv && _appHints == CCMI_BCAST_RECV_STORE)
              postAppRecv (minterface);
          }


          ///
          /// \brief Post dma data receives 
          ///
          void SyncBcastPost(Geometry                                     * geometry, 
                             unsigned                                       root,
                             CCMI::ConnectionManager::ConnectionManager   * cmgr,
                             CCMI::MultiSend::OldMulticastInterface          * minterface)
          {
            if(_toAppRecv && _appHints == CCMI_PT_TO_PT_SUBTASK)
              postAppRecv (minterface);
          }

          static void second_barrier_start (void *me, CM_Error_t *err)
          {
            TreeBcastComposite *composite = ( TreeBcastComposite *) me;

            CCMI::Executor::Executor *barrier = composite->_geometry->getBarrierExecutor();
            CCMI_assert(barrier != NULL);
            barrier->setDoneCallback (composite->_cb_app_done.function, composite->_cb_app_done.clientdata);
            barrier->setConsistency (CCMI_MATCH_CONSISTENCY);
            barrier->start();
          }
        };

        ///
        /// \brief TreeBcastFactory class for broadcast factory implementations.
        ///
        class TreeBcastFactory : public BroadcastFactory<CCMI::TorusCollectiveMapping>
        {
        public:
          ///
          /// \brief Constructor for broadcast factory implementations.
          ///
          TreeBcastFactory
          (CCMI::TorusCollectiveMapping                  * map,
           CCMI::MultiSend::OldMulticastInterface        * mf,
           CCMI::ConnectionManager::ConnectionManager * cmgr,
           unsigned                                       nconn)
          : BroadcastFactory <CCMI::TorusCollectiveMapping>(mf, map, cmgr, nconn)
          {
            //--
            TRACE_ADAPTOR ((stderr, "Tree Broadcast Factory Constructor\n"));
          }

          ///\brief Tree only works on comm_world
          virtual bool Analyze(Geometry *geometry)
          {
            return global_analyze (geometry);
          }

          ///
          /// \brief Generate a non-blocking broadcast message.
          ///
          /// \param[in]  request      Opaque memory to maintain
          ///                          internal message state.
          /// \param[in]  cb_done      Callback to invoke when
          ///                message is complete.
          /// \param[in]  consistency  Required consistency level
          /// \param[in]  geometry     Geometry to use for this
          ///                collective operation.
          ///                          \c NULL indicates the global geometry.
          /// \param[in]  root         Rank of the node performing
          ///                the broadcast.
          /// \param[in]  src          Source buffer to broadcast.
          /// \param[in]  bytes        Number of bytes to broadcast.
          ///
          /// \retval     0            Success
          /// \retval     1            Unavailable in geometry
          ///

          virtual CCMI::Executor::Composite * generate
          (void                      * request,
           size_t                      rsize,
           CM_Callback_t             cb_done,
           CCMI_Consistency            consistency,
           Geometry                  * geometry,
           unsigned                    root,
           char                      * src,
           unsigned                    bytes)
          {
            TRACE_ADAPTOR ((stderr, "Tree Broadcast Generate\n"));      
            CM_Callback_t  cb_app_done = cb_done;     

            if(_mapping->GetDimLength(CCMI_T_DIM) > 1)  //Enable second barrier after bcast
            {
              cb_done.function   = TreeBcastComposite::second_barrier_start;
              cb_done.clientdata = request;  //Pass pointer before its constructed
            }

            CM_assert(rsize >= sizeof(TreeBcastComposite));
            TreeBcastComposite  *composite = 
            new (request)
            TreeBcastComposite (_mapping,
                                _connmgr,
                                cb_done,
                                cb_app_done,
                                consistency, 
                                _minterface,
                                geometry, 
                                root, 
                                src, 
                                bytes);     

            if(_mapping->GetDimLength(CCMI_T_DIM) > 1)
            {
              composite->initVnDual(root);
              composite->SyncBcastPost (geometry, root, _connmgr, _minterface); 
              CCMI::Executor::Executor *barrier = geometry->getLocalBarrierExecutor();
              CCMI_assert(barrier != NULL);
              barrier->setDoneCallback (TreeBcastComposite::cb_barrier_done, composite);
              barrier->setConsistency (consistency);
              barrier->start();
              composite->SyncBcastTreePost (_minterface); 
            }
            else
            {
              composite->initSMP(root);       
              TreeBcastComposite::cb_barrier_done (composite, NULL);
              //Post recvs after send
              composite->SyncBcastTreePost (_minterface); 
            }

            return composite;
          }
        };

      };  
        template<> void Tree::TreeBaseComposite::create_schedule ( void                      * buf,
                                               unsigned                    size,
                                               Geometry                  * g,
                                               CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::TreeBwSchedule (_mapping, g->nranks(), g->ranks());
      }
    };  
  };  
};  

#endif
