/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/P2PCCMIRegInfo.h
 * \brief ???
 */

#ifndef __algorithms_geometry_P2PCCMIRegInfo_h__
#define __algorithms_geometry_P2PCCMIRegInfo_h__

#include <map>
#include <vector>
#include "util/ccmi_debug.h"
#include "TypeDefs.h"
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "algorithms/schedule/GenericTreeT.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/connmgr/ColorConnMgr.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
#include "algorithms/protocols/broadcast/BcastMultiColorCompositeT.h"
#include "algorithms/protocols/barrier/BarrierT.h"
#include "algorithms/protocols/broadcast/AsyncBroadcastT.h"
#include "algorithms/protocols/ambcast/AMBroadcastT.h"
#include "algorithms/protocols/allreduce/MultiColorCompositeT.h"
#include "algorithms/protocols/allreduce/ProtocolFactoryT.h"
#include "algorithms/protocols/allreduce/AsyncAllreduceT.h"
#include "algorithms/protocols/allreduce/AsyncReduceScatterT.h"
#include "algorithms/protocols/scatter/AsyncScatterT.h"
#include "algorithms/protocols/gather/AsyncGatherT.h"
#include "algorithms/protocols/gather/AsyncLongGatherT.h"
#include "algorithms/protocols/allgather/AsyncAllgatherT.h"
#include "algorithms/protocols/allgather/AsyncAllgathervT.h"
#include "algorithms/protocols/alltoall/AsyncAlltoallvT.h"
#include "algorithms/protocols/scan/AsyncScanT.h"
#include "p2p/protocols/SendPWQ.h"
#include "common/NativeInterface.h"
#include "algorithms/protocols/alltoall/All2All.h"
#include "algorithms/protocols/alltoall/All2Allv.h"
#include "algorithms/schedule/TorusRect.h"

// CCMI Template implementations for P2P
namespace CCMI
{
  namespace Adaptor
  {
    namespace P2PBarrier
    {
      void binomial_barrier_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "P2P_CCMI Binom_Bar");
      }

      bool binomial_analyze (PAMI_GEOMETRY_CLASS *geometry)
      {
        return true;
      }

      typedef CCMI::Adaptor::Barrier::BarrierT
      < CCMI::Schedule::ListMultinomial,
        binomial_analyze >
      BinomialBarrier;

      typedef CCMI::Adaptor::Barrier::BarrierFactoryT
      < BinomialBarrier,
        binomial_barrier_md,
        CCMI::ConnectionManager::SimpleConnMgr>
      BinomialBarrierFactory;
    };//Barrier

    namespace P2PBroadcast
    {
      void get_colors (PAMI::Topology             * t,
                       unsigned                    bytes,
                       unsigned                  * colors,
                       unsigned                  & ncolors)
      {
        TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
        ncolors = 1;
        colors[0] = CCMI::Schedule::NO_COLOR;
      }
    void get_rect_colors (PAMI::Topology             * t,
                          unsigned                    bytes,
                          unsigned                  * colors,
                          unsigned                  & ncolors)
    {

      unsigned max = 0, ideal = 0;
      unsigned _colors[PAMI_MAX_DIMS*2];
      PAMI_assert(ncolors <= (PAMI_MAX_DIMS*2));
      CCMI::Schedule::TorusRect::getColors (t, ideal, max, _colors);
      TRACE_INIT((stderr, "get_rect_colors() bytes %u, ncolors %u, ideal %u, max %u\n", bytes, ncolors, ideal, max));
      
      if (bytes <= 8192) //16 packets
        ideal = 1;
      else if (bytes <= 65536 && ideal >= 2)
        ideal = 2;
      else if (bytes <= 262144 && ideal >= 3)
        ideal = 3;
      else if (bytes <= 1048576 && ideal >= 5)
        ideal = 5;
      else if (bytes <= 4194304 && ideal >= 8)
        ideal = 8;

      if (ideal < ncolors)
        ncolors = ideal;  //Reduce the number of colors to the relavant colors
      
      TRACE_INIT((stderr, "get_rect_colors() ncolors %u, ideal %u\n", ncolors, ideal));
      memcpy (colors, _colors, ncolors * sizeof(int));
    }


      void rectangle_broadcast_metadata(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "RectangleP2PBroadcast");
      }

      typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
      < 5,
        CCMI::Schedule::TorusRect,
        CCMI::ConnectionManager::ColorConnMgr,
        get_rect_colors,
        PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX>
      RectangleBroadcastComposite;

      typedef CCMI::Adaptor::CollectiveProtocolFactoryT
      < RectangleBroadcastComposite,
        rectangle_broadcast_metadata,
        CCMI::ConnectionManager::ColorConnMgr>
      RectangleBroadcastFactory;

      void rectangle_1color_broadcast_metadata(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "Rectangle1ColorP2PBroadcast");
      }

      typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
      < 1,
        CCMI::Schedule::TorusRect,
        CCMI::ConnectionManager::ColorConnMgr,
        get_colors,
        PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX>
      RectangleBroadcastComposite1Color;

      typedef CCMI::Adaptor::CollectiveProtocolFactoryT
      < RectangleBroadcastComposite1Color,
        rectangle_1color_broadcast_metadata,
        CCMI::ConnectionManager::ColorConnMgr>
      Rectangle1ColorBroadcastFactory;

      void binomial_broadcast_metadata(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "P2P_CCMI Binom_Bcast");
      }

      void ring_broadcast_metadata(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "P2P_CCMI Ring_Bcast");
      }

      typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
      < 1,
        CCMI::Schedule::ListMultinomial,
        CCMI::ConnectionManager::ColorGeometryConnMgr,
        get_colors >
      BinomialBroadcastComposite;

      typedef CCMI::Adaptor::CollectiveProtocolFactoryT
      < BinomialBroadcastComposite,
        binomial_broadcast_metadata,
        CCMI::ConnectionManager::ColorGeometryConnMgr>
      BinomialBroadcastFactory;

      typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
      < 1,
        CCMI::Schedule::RingSchedule,
        CCMI::ConnectionManager::ColorGeometryConnMgr,
        get_colors >
      RingBroadcastComposite;

      typedef CCMI::Adaptor::CollectiveProtocolFactoryT
      < RingBroadcastComposite,
        ring_broadcast_metadata,
        CCMI::ConnectionManager::ColorGeometryConnMgr>
      RingBroadcastFactory;

      void am_rb_broadcast_metadata(pami_metadata_t *m)
      {
        TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "P2P_CCMI AS_RB_Binom_Bcast");
      }

      void am_cs_broadcast_metadata(pami_metadata_t *m)
      {
        TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "P2P_CCMI AS_CS_Binom_Bcast");
      }

      void create_schedule(void                        * buf,
                           unsigned                      size,
                           unsigned                      root,
                           Interfaces::NativeInterface * native,
                           PAMI_GEOMETRY_CLASS         * g)
      {
        TRACE_INIT((stderr, "<%p>AsyncRBBinomialBroadcastComposite::create_schedule()\n",(void*)NULL));
        new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
      }

      unsigned getKey(unsigned                                                root,
                      unsigned                                                connid,
                      PAMI_GEOMETRY_CLASS                                    *geometry,
                      ConnectionManager::BaseConnectionManager              **connmgr)
      {
        TRACE_INIT((stderr, "<%p>AsyncRBBinomialBroadcastFactory::getKey()\n",(void*)NULL));
        return root;
      }

      void create_schedule_as(void                        * buf,
                              unsigned                      size,
                              unsigned                      root,
                              Interfaces::NativeInterface * native,
                              PAMI_GEOMETRY_CLASS          * g)
      {
        TRACE_INIT((stderr, "<%p>AsyncCSBinomialBroadcastComposite::create_schedule()\n",(void*)NULL));
        new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
      }

      unsigned getKey_as(unsigned                                   root,
                         unsigned                                   connid,
                         PAMI_GEOMETRY_CLASS                      * geometry,
                         ConnectionManager::BaseConnectionManager **connmgr)
      {
        TRACE_INIT((stderr, "<%p>AsyncCSBinomialBroadcastFactory::getKey\n",(void*)NULL));
        ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
        if (connid != (unsigned) - 1)
          {
            *connmgr = NULL; //use this key as connection id
            return connid;
          }

        return cm->updateConnectionId( geometry->comm() );
      }

      typedef CCMI::Adaptor::Broadcast::AsyncBroadcastT
      < CCMI::Schedule::ListMultinomial,
        CCMI::ConnectionManager::RankBasedConnMgr,
        create_schedule>
      AsyncRBBinomialBroadcastComposite;

      typedef CCMI::Adaptor::Broadcast::AsyncBroadcastFactoryT
      < AsyncRBBinomialBroadcastComposite,
        am_rb_broadcast_metadata,
        CCMI::ConnectionManager::RankBasedConnMgr,
        getKey >
      AsyncRBBinomialBroadcastFactory;


      typedef CCMI::Adaptor::Broadcast::AsyncBroadcastT
      < CCMI::Schedule::ListMultinomial,
        CCMI::ConnectionManager::CommSeqConnMgr,
        create_schedule_as > AsyncCSBinomialBroadcastComposite;

      typedef CCMI::Adaptor::Broadcast::AsyncBroadcastFactoryT
      < AsyncCSBinomialBroadcastComposite,
        am_cs_broadcast_metadata,
        CCMI::ConnectionManager::CommSeqConnMgr,
        getKey_as> AsyncCSBinomialBroadcastFactory;

    }//Broadcast

    namespace P2PAMBroadcast
    {
      void am_broadcast_metadata(pami_metadata_t *m)
      {
        TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "P2P_CCMI Binom_AMBcast");
      }

      void create_schedule(void                        * buf,
                           unsigned                      size,
                           unsigned                      root,
                           Interfaces::NativeInterface * native,
                           PAMI_GEOMETRY_CLASS          * g)
      {
        TRACE_INIT((stderr, "<%p>AMBinomialBroadcastComposite::create_schedule()\n",(void*)NULL));
        new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
      }

      typedef CCMI::Adaptor::AMBroadcast::AMBroadcastT
      < CCMI::Schedule::ListMultinomial,
        CCMI::ConnectionManager::RankBasedConnMgr,
        create_schedule>
      AMBinomialBroadcastComposite;


      typedef CCMI::Adaptor::AMBroadcast::AMBroadcastFactoryT
      < AMBinomialBroadcastComposite,
        am_broadcast_metadata,
        CCMI::ConnectionManager::RankBasedConnMgr>
      AMBinomialBroadcastFactory;

    }//AMBroadcast

    namespace P2PAllreduce
    {
      /// New Binomial algorithms
      /// class Binomial::Composite and Binomial::Factory
      ///
      /// \brief Binomial allreduce protocol
      ///
      /// Use the BinomialTreeSchedule
      ///
      namespace Binomial
      {
        void get_colors (PAMI::Topology             * t,
                         unsigned                    bytes,
                         unsigned                  * colors,
                         unsigned                  & ncolors)
        {
          ncolors = 1;
          colors[0] = CCMI::Schedule::NO_COLOR;
        }

        void binomial_allreduce_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Binom_Allred");
        }

        typedef CCMI::Adaptor::Allreduce::MultiColorCompositeT
        < 1, CCMI::Executor::AllreduceBaseExec<CCMI::ConnectionManager::RankBasedConnMgr>,
          CCMI::Schedule::ListMultinomial,
          CCMI::ConnectionManager::RankBasedConnMgr,
          get_colors > Composite;

        typedef CCMI::Adaptor::Allreduce::ProtocolFactoryT
        < Composite,
          binomial_allreduce_metadata,
          CCMI::ConnectionManager::RankBasedConnMgr>
        Factory;

       unsigned getKey(unsigned                                   root,
                       unsigned                                   connid,
                       PAMI_GEOMETRY_CLASS                      * geometry,
                       ConnectionManager::BaseConnectionManager **connmgr)
       {
         if (connid != (unsigned)-1)
         {
           *connmgr = NULL; //use this key as connection id
           return connid;
         }
         ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
         return cm->updateConnectionId( geometry->comm() );
       }

       /// New asynchronous CommSeq binomial allreduce algorithms
       /// class Binomial::AsyncAllreduceT and Binomial::AsyncAllreduceFactoryT
       ///
       /// \brief Binomial allreduce protocol
       ///
       /// Use the BinomialTreeSchedule
       ///
       void ascs_binomial_allreduce_metadata(pami_metadata_t *m)
       {
          // \todo:  fill in other metadata
          strcpy(&m->name[0],"CCMI_ASCS_BinomialAllreduce");
       }

       typedef CCMI::Adaptor::Allreduce::AsyncAllreduceT<CCMI::Schedule::ListMultinomial,
         CCMI::ConnectionManager::CommSeqConnMgr, pami_allreduce_t>
         AsyncCSBinomAllreduceComposite;
       typedef CCMI::Adaptor::Allreduce::AsyncAllreduceFactoryT<AsyncCSBinomAllreduceComposite,
         ascs_binomial_allreduce_metadata, CCMI::ConnectionManager::CommSeqConnMgr,
         pami_allreduce_t, getKey> AsyncCSBinomAllreduceFactory;

       void ascs_binomial_reduce_metadata(pami_metadata_t *m)
       {
         // \todo:  fill in other metadata
         strcpy(&m->name[0],"CCMI_ASCS_BinomialReduce");
       }

       typedef CCMI::Adaptor::Allreduce::AsyncAllreduceT<CCMI::Schedule::ListMultinomial,
         CCMI::ConnectionManager::CommSeqConnMgr, pami_reduce_t>
         AsyncCSBinomReduceComposite;
       typedef CCMI::Adaptor::Allreduce::AsyncAllreduceFactoryT<AsyncCSBinomReduceComposite,
         ascs_binomial_reduce_metadata, CCMI::ConnectionManager::CommSeqConnMgr,
         pami_reduce_t, getKey> AsyncCSBinomReduceFactory;

      };//Binomial
    };//Allreduce

    namespace P2PScatter
    {

      unsigned getKey(unsigned                                   root,
                      unsigned                                   connid,
                      PAMI_GEOMETRY_CLASS                      * geometry,
                      ConnectionManager::BaseConnectionManager **connmgr)
      {
        if (connid != (unsigned)-1)
        {
          *connmgr = NULL; //use this key as connection id
          return connid;
        }
        ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
        return cm->updateConnectionId( geometry->comm() );
      }

      namespace Binomial
      {

        void create_schedule(void                        * buf,
                             unsigned                      size,
                             unsigned                      root,
                             Interfaces::NativeInterface * native,
                             PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncBinomialScatterComposite::create_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<>(native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void binomial_scatter_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Binomial Scatter");
        }

        typedef CCMI::Adaptor::Scatter::AsyncScatterT
        < CCMI::Schedule::GenericTreeSchedule<>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_schedule, pami_scatter_t > Composite;

        typedef CCMI::Adaptor::Scatter::AsyncScatterFactoryT
        < Composite,
          binomial_scatter_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey > Factory;

      } // Binomial

      namespace Flat
      {

        void create_schedule(void                        * buf,
                             unsigned                      size,
                             unsigned                      root,
                             Interfaces::NativeInterface * native,
                             PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncFlatScatterComposite::create_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<1,1,1> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void flat_scatter_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Flat Scatter");
        }

        typedef CCMI::Adaptor::Scatter::AsyncScatterT
        < CCMI::Schedule::GenericTreeSchedule<1, 1, 1>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_schedule, pami_scatter_t > Composite;

        typedef CCMI::Adaptor::Scatter::AsyncScatterFactoryT
        < Composite,
          flat_scatter_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        Factory;

      } // Flat
    } // P2PScatter

    namespace P2PScatterv
      {

      unsigned getKey(unsigned                                   root,
                      unsigned                                   connid,
                      PAMI_GEOMETRY_CLASS                      * geometry,
                      ConnectionManager::BaseConnectionManager **connmgr)
      {
        if (connid != (unsigned)-1)
        {
          *connmgr = NULL; //use this key as connection id
          return connid;
        }
        ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
        return cm->updateConnectionId( geometry->comm() );
      }


        void create_schedule(void                        * buf,
                             unsigned                      size,
                             unsigned                      root,
                             Interfaces::NativeInterface * native,
                             PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncScattervComposite::create_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<1,1,1> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void scatterv_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Scatterv");
        }

        typedef CCMI::Adaptor::Scatter::AsyncScatterT
        < CCMI::Schedule::GenericTreeSchedule<1, 1, 1>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_schedule, pami_scatterv_t > Composite;

        typedef CCMI::Adaptor::Scatter::AsyncScatterFactoryT
        < Composite,
          scatterv_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        Factory;

        void scatterv_int_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Scatterv_int");
        }

        typedef CCMI::Adaptor::Scatter::AsyncScatterT
        < CCMI::Schedule::GenericTreeSchedule<1, 1, 1>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_schedule, pami_scatterv_int_t > IntComposite;

        typedef CCMI::Adaptor::Scatter::AsyncScatterFactoryT
        < Composite,
          scatterv_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        IntFactory;

    } // P2PScatterv

    namespace P2PReduceScatter
    {

      unsigned getKey(unsigned                                   root,
                      unsigned                                   connid,
                      PAMI_GEOMETRY_CLASS                      * geometry,
                      ConnectionManager::BaseConnectionManager **connmgr)
      {
        if (connid != (unsigned)-1)
        {
          *connmgr = NULL; //use this key as connection id
          return connid;
        }
        ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
        return cm->updateConnectionId( geometry->comm() );
      }


      void create_schedule(void                        * buf,
                           unsigned                      size,
                           unsigned                      root,
                           Interfaces::NativeInterface * native,
                           PAMI_GEOMETRY_CLASS          * g)
      {
        TRACE_INIT((stderr, "<%p>AsyncReduceScatterComposite::create_schedule()\n",(void*)NULL));
        new (buf) CCMI::Schedule::GenericTreeSchedule<1,1,1> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
      }


       void ascs_reduce_scatter_metadata(pami_metadata_t *m)
       {
         // \todo:  fill in other metadata
         strcpy(&m->name[0],"CCMI_ASCS_ReduceScatter");
       }

       typedef CCMI::Adaptor::Allreduce::AsyncReduceScatterT<CCMI::Schedule::ListMultinomial,
         CCMI::Schedule::GenericTreeSchedule<1, 1, 1>,
         CCMI::ConnectionManager::CommSeqConnMgr, create_schedule>
         AsyncCSReduceScatterComposite;

       typedef CCMI::Adaptor::Allreduce::AsyncReduceScatterFactoryT<AsyncCSReduceScatterComposite,
         ascs_reduce_scatter_metadata, CCMI::ConnectionManager::CommSeqConnMgr,getKey>
         AsyncCSReduceScatterFactory;

    } // P2PReduceScatter

    namespace P2PGather
    {

      unsigned getKey(unsigned                                   root,
                      unsigned                                   connid,
                      PAMI_GEOMETRY_CLASS                      * geometry,
                      ConnectionManager::BaseConnectionManager **connmgr)
      {
        if (connid != (unsigned)-1)
        {
          *connmgr = NULL; //use this key as connection id
          return connid;
        }
        ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
        return cm->updateConnectionId( geometry->comm() );
      }

      namespace Binomial
      {

        void create_schedule(void                        * buf,
                             unsigned                      size,
                             unsigned                      root,
                             Interfaces::NativeInterface * native,
                             PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncBinomialGatherComposite::create_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void binomial_gather_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Binomial Gather");
        }

        typedef CCMI::Adaptor::Gather::AsyncGatherT
        < CCMI::Schedule::GenericTreeSchedule<>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_schedule, pami_gather_t > Composite;

        typedef CCMI::Adaptor::Gather::AsyncGatherFactoryT
        < Composite,
          binomial_gather_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        Factory;

      }// Binomial

      namespace Flat
      {

        void create_gather_schedule(void                        * buf,
                                    unsigned                      size,
                                    unsigned                      root,
                                    Interfaces::NativeInterface * native,
                                    PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncFlatGatherComposite::create_gather_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<1,1,1> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void create_bcast_schedule(void                        * buf,
                                   unsigned                      size,
                                   unsigned                      root,
                                   Interfaces::NativeInterface * native,
                                   PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncFlatGatherComposite::create_bcast_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<1,1,2> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void flat_gather_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Flat Gather");
        }

        typedef CCMI::Adaptor::Gather::AsyncLongGatherT
        < pami_gather_t,
          CCMI::Schedule::GenericTreeSchedule<1, 1, 2>,
          CCMI::Schedule::GenericTreeSchedule<1, 1, 1>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_bcast_schedule,
          create_gather_schedule > Composite;

        typedef CCMI::Adaptor::Gather::AsyncLongGatherFactoryT
        < Composite,
          flat_gather_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        Factory;

      } // Flat
    }// P2PGather

    namespace P2PGatherv
    {

      unsigned getKey(unsigned                                   root,
                      unsigned                                   connid,
                      PAMI_GEOMETRY_CLASS                      * geometry,
                      ConnectionManager::BaseConnectionManager **connmgr)
      {
        if (connid != (unsigned)-1)
        {
          *connmgr = NULL; //use this key as connection id
          return connid;
        }
        ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
        return cm->updateConnectionId( geometry->comm() );
      }


        void create_gatherv_schedule(void                        * buf,
                                     unsigned                      size,
                                     unsigned                      root,
                                     Interfaces::NativeInterface * native,
                                     PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncGathervComposite::create_gatherv_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<1,1,1> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void create_bcast_schedule(void                        * buf,
                                   unsigned                      size,
                                   unsigned                      root,
                                   Interfaces::NativeInterface * native,
                                   PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncGathervComposite::create_bcast_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<1,1,2> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void gatherv_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Gatherv");
        }

        typedef CCMI::Adaptor::Gather::AsyncLongGatherT
        < pami_gatherv_t,
          CCMI::Schedule::GenericTreeSchedule<1, 1, 2>,
          CCMI::Schedule::GenericTreeSchedule<1, 1, 1>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_bcast_schedule,
          create_gatherv_schedule > Composite;

        typedef CCMI::Adaptor::Gather::AsyncLongGatherFactoryT
        < Composite,
          gatherv_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        Factory;

        void gatherv_int_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Gatherv_int");
        }

        typedef CCMI::Adaptor::Gather::AsyncLongGatherT
        < pami_gatherv_int_t,
          CCMI::Schedule::GenericTreeSchedule<1, 1, 2>,
          CCMI::Schedule::GenericTreeSchedule<1, 1, 1>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_bcast_schedule,
          create_gatherv_schedule > IntComposite;

        typedef CCMI::Adaptor::Gather::AsyncLongGatherFactoryT
        < Composite,
          gatherv_int_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        IntFactory;

     } // P2PGatherv

    namespace P2PScan
    {
      namespace Binomial
      {

        unsigned getKey(unsigned                                   root,
                        unsigned                                   connid,
                        PAMI_GEOMETRY_CLASS                      * geometry,
                        ConnectionManager::BaseConnectionManager **connmgr)
        {
          if (connid != (unsigned)-1)
          {
            *connmgr = NULL; //use this key as connection id
            return connid;
          }
          ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
          return cm->updateConnectionId( geometry->comm() );
        }

        void create_schedule(void                        * buf,
                             unsigned                      size,
                             unsigned                      root,
                             Interfaces::NativeInterface * native,
                             PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncBinomialScanComposite::create_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void binomial_scan_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Binomial Scan");
        }

        typedef CCMI::Adaptor::Scan::AsyncScanT
        < CCMI::Schedule::GenericTreeSchedule<>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_schedule > Composite;

        typedef CCMI::Adaptor::Scan::AsyncScanFactoryT
        < Composite,
          binomial_scan_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        Factory;

      }// Binomial
    }// P2PScan

    namespace P2PAllgather
    {

      namespace Binomial
      {

        unsigned getKey(unsigned                                   root,
                        unsigned                                   connid,
                        PAMI_GEOMETRY_CLASS                      * geometry,
                        ConnectionManager::BaseConnectionManager **connmgr)
        {
          if (connid != (unsigned)-1)
          {
            *connmgr = NULL; //use this key as connection id
            return connid;
          }
          ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
          return cm->updateConnectionId( geometry->comm() );
        }

        void create_schedule(void                        * buf,
                             unsigned                      size,
                             unsigned                      root,
                             Interfaces::NativeInterface * native,
                             PAMI_GEOMETRY_CLASS          * g)
        {
          TRACE_INIT((stderr, "<%p>AsyncBinomialAllgatherComposite::create_schedule()\n",(void*)NULL));
          new (buf) CCMI::Schedule::GenericTreeSchedule<> (native->myrank(), (PAMI::Topology *)g->getTopology(0));
        }

        void binomial_allgather_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Binomial Allgather");
        }

        typedef CCMI::Adaptor::Allgather::AsyncAllgatherT
        < CCMI::Schedule::GenericTreeSchedule<>,
          CCMI::ConnectionManager::CommSeqConnMgr,
          create_schedule > Composite;

        typedef CCMI::Adaptor::Allgather::AsyncAllgatherFactoryT
        < Composite,
          binomial_allgather_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        Factory;

      }// Binomial
    }// P2PAllgather

    namespace P2PAllgatherv
    {

      namespace Ring
      {

        unsigned getKey(unsigned                                   root,
                        unsigned                                   connid,
                        PAMI_GEOMETRY_CLASS                      * geometry,
                        ConnectionManager::BaseConnectionManager **connmgr)
        {
          if (connid != (unsigned)-1)
          {
            *connmgr = NULL; //use this key as connection id
            return connid;
          }
          ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
          return cm->updateConnectionId( geometry->comm() );
        }

        void ring_allgather_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Ring Allgather");
        }

        void ring_allgatherv_int_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Ring Allgatherv_int");
        }

        void ring_allgatherv_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Ring Allgatherv");
        }

        typedef CCMI::Adaptor::Allgatherv::AsyncAllgathervT
        < CCMI::ConnectionManager::CommSeqConnMgr,
          pami_allgather_t > AllgatherComposite;

        typedef CCMI::Adaptor::Allgatherv::AsyncAllgathervFactoryT
        < pami_allgather_t,
          AllgatherComposite,
          ring_allgather_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        AllgatherFactory;

        typedef CCMI::Adaptor::Allgatherv::AsyncAllgathervT
        < CCMI::ConnectionManager::CommSeqConnMgr,
          pami_allgatherv_int_t > AllgathervIntComposite;

        typedef CCMI::Adaptor::Allgatherv::AsyncAllgathervFactoryT
        < pami_allgatherv_int_t,
          AllgathervIntComposite,
          ring_allgatherv_int_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        AllgathervIntFactory;

        typedef CCMI::Adaptor::Allgatherv::AsyncAllgathervT
        < CCMI::ConnectionManager::CommSeqConnMgr,
          pami_allgatherv_t > AllgathervComposite;

        typedef CCMI::Adaptor::Allgatherv::AsyncAllgathervFactoryT
        < pami_allgatherv_t,
          AllgathervComposite,
          ring_allgatherv_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        AllgathervFactory;


      }// Ring
    }// P2PAllgatherv

    namespace P2PAlltoallv
    {

      namespace Pairwise
      {

        unsigned getKey(unsigned                                   root,
                        unsigned                                   connid,
                        PAMI_GEOMETRY_CLASS                      * geometry,
                        ConnectionManager::BaseConnectionManager **connmgr)
        {
          if (connid != (unsigned)-1)
          {
            *connmgr = NULL; //use this key as connection id
            return connid;
          }
          ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
          return cm->updateConnectionId( geometry->comm() );
        }

        void pairwise_alltoall_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Pairwise Alltoall");
        }

        void pairwise_alltoallv_int_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0], "P2P_CCMI Pairwise Alltoallv_int");
        }

        typedef CCMI::Adaptor::Alltoallv::AsyncAlltoallvT
        < CCMI::ConnectionManager::CommSeqConnMgr,
          pami_alltoall_t > AlltoallComposite;

        typedef CCMI::Adaptor::Alltoallv::AsyncAlltoallvFactoryT
        < pami_alltoall_t,
          AlltoallComposite,
          pairwise_alltoall_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        AlltoallFactory;

        typedef CCMI::Adaptor::Alltoallv::AsyncAlltoallvT
        < CCMI::ConnectionManager::CommSeqConnMgr,
          pami_alltoallv_int_t > AlltoallvIntComposite;

        typedef CCMI::Adaptor::Alltoallv::AsyncAlltoallvFactoryT
        < pami_alltoallv_int_t,
          AlltoallvIntComposite,
          pairwise_alltoallv_int_metadata,
          CCMI::ConnectionManager::CommSeqConnMgr,
          getKey >
        AlltoallvIntFactory;

      }// Pairwise
    }// P2PAlltoallv

    namespace P2PAlltoall
    {
      void getAlltoallMetaData(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"P2P Alltoall");
      }
      typedef CCMI::Adaptor::All2AllProtocol All2AllProtocol;
      typedef CCMI::Adaptor::All2AllFactoryT <All2AllProtocol, getAlltoallMetaData, CCMI::ConnectionManager::CommSeqConnMgr> All2AllFactory;

    };//P2PAlltoall
    namespace P2PAlltoallv
    {
      void getAlltoallvMetaData(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"P2P Alltoallv");
      }
      typedef CCMI::Adaptor::All2AllvProtocol All2AllProtocolv;
      typedef CCMI::Adaptor::All2AllvFactoryT <All2AllProtocolv, getAlltoallvMetaData, CCMI::ConnectionManager::CommSeqConnMgr> All2AllvFactory;

    };//P2PAlltoallv


  }//Adaptor
}//CCMI

#endif
