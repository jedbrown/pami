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
#include "SysDep.h"
#include "util/ccmi_debug.h"
#include "TypeDefs.h"
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
#include "algorithms/protocols/broadcast/BcastMultiColorCompositeT.h"
#include "algorithms/protocols/barrier/BarrierT.h"
#include "algorithms/protocols/broadcast/AsyncBroadcastT.h"
#include "algorithms/protocols/ambcast/AMBroadcastT.h"
#include "algorithms/protocols/allreduce/MultiColorCompositeT.h"
#include "algorithms/protocols/allreduce/ProtocolFactoryT.h"
#include "algorithms/protocols/allreduce/AsyncAllreduceT.h"
#include "p2p/protocols/SendPWQ.h"
#include "common/NativeInterface.h"

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
        CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> >
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
        CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS>,
        get_colors >
      BinomialBroadcastComposite;

      typedef CCMI::Adaptor::CollectiveProtocolFactoryT
      < BinomialBroadcastComposite,
        binomial_broadcast_metadata,
        CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS> >
      BinomialBroadcastFactory;

      typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
      < 1,
        CCMI::Schedule::RingSchedule,
        CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS>,
        get_colors >
      RingBroadcastComposite;

      typedef CCMI::Adaptor::CollectiveProtocolFactoryT
      < RingBroadcastComposite,
        ring_broadcast_metadata,
        CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS> >
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
        CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
        create_schedule>
      AsyncRBBinomialBroadcastComposite;

      typedef CCMI::Adaptor::Broadcast::AsyncBroadcastFactoryT
      < AsyncRBBinomialBroadcastComposite,
        am_rb_broadcast_metadata,
        CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
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
        CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
        create_schedule>
      AMBinomialBroadcastComposite;


      typedef CCMI::Adaptor::AMBroadcast::AMBroadcastFactoryT
      < AMBinomialBroadcastComposite,
        am_broadcast_metadata,
        CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >
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
        < 1, CCMI::Executor::AllreduceBaseExec<CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >,
          CCMI::Schedule::ListMultinomial,
          CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
          get_colors > Composite;

        typedef CCMI::Adaptor::Allreduce::ProtocolFactoryT
        < Composite,
          binomial_allreduce_metadata,
          CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >
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
  }//Adaptor
}//CCMI

#endif
