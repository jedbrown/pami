/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/sync_impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __algorithms_protocols_allreduce_sync_impl_h__
#define __algorithms_protocols_allreduce_sync_impl_h__

#include "algorithms/protocols/allreduce/CompositeT.h"
#include "algorithms/protocols/allreduce/FactoryT.h"
#include "algorithms/executor/PipelinedAllreduce.h"
#include "algorithms/executor/Allreduce.h"
#include "algorithms/schedule/BinomialTree.h"
//#include "algorithms/schedule/Rectangle.h"
//#include "algorithms/schedule/TreeBwSchedule.h"

#include "algorithms/connmgr/RankBasedConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {

      // class Binomial::Composite and Binomial::Factory
      ///
      /// \brief Binomial allreduce protocol
      ///
      /// Use the BinomialTreeSchedule
      ///
      namespace Binomial
      {

        void binomial_ar_md(pami_metadata_t *m)
         {
           // \todo:  fill in other metadata
           strcpy(&m->name[0],"OldCCMIBinomialAllreduce");
         }

        typedef CompositeT
        <CCMI::Schedule::BinomialTreeSchedule<PAMI_SYSDEP_CLASS>,
         CCMI::Executor::Allreduce<PAMI_COLL_MCAST_CLASS,PAMI_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >,
         PAMI_SYSDEP_CLASS,
         PAMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::Binomial::Composite,
         PAMI_SYSDEP_CLASS,
         PAMI_COLL_MCAST_CLASS,
         binomial_ar_md>
        Factory;
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      ///
      // Specify the static name in the class (for debug)
      template<> const char* Binomial::Composite::name="Binomial";
      template<> inline bool Binomial::Composite::analyze(PAMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void Binomial::Composite::create_schedule(PAMI_SYSDEP_CLASS * map,PAMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule<PAMI_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
      };

      // class ShortBinomial::Composite and ShortBinomial::Factory
      ///
      /// \brief Binomial short allreduce protocol
      ///
      /// Use the BinomialTreeSchedule
      ///
      namespace ShortBinomial
      {
        void short_ar_md(pami_metadata_t *m)
         {
           // \todo:  fill in other metadata
           strcpy(&m->name[0],"OldCCMIShortAllreduce");
         }

        typedef CompositeT
        <CCMI::Schedule::BinomialTreeSchedule<PAMI_SYSDEP_CLASS>,
         CCMI::Executor::AllreduceBase<PAMI_COLL_MCAST_CLASS,PAMI_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >,
         PAMI_SYSDEP_CLASS,
         PAMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >
        Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::ShortBinomial::Composite,
         PAMI_SYSDEP_CLASS,
         PAMI_COLL_MCAST_CLASS,
         short_ar_md>
        Factory;
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial short allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      ///
      // Specify the static name in the class (for debug)
      template<> const char* ShortBinomial::Composite::name="ShortBinomial";
      template<> inline bool ShortBinomial::Composite::analyze(PAMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void ShortBinomial::Composite::create_schedule(PAMI_SYSDEP_CLASS * map,PAMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule<PAMI_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
      };

      // class Ring::Composite and Ring::Factory
      ///
      /// \brief Ring allreduce protocol
      ///
      /// Use the RingSchedule
      ///
      namespace Ring
      {

         void ring_ar_md(pami_metadata_t *m)
         {
           // \todo:  fill in other metadata
           strcpy(&m->name[0],"OldCCMIRingAllreduce");
         }

        typedef CompositeT
        <CCMI::Schedule::OldRingSchedule<PAMI_SYSDEP_CLASS>,
         CCMI::Executor::PipelinedAllreduce<PAMI_COLL_MCAST_CLASS,PAMI_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >,
         PAMI_SYSDEP_CLASS,
         PAMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >
        Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::Ring::Composite,
         PAMI_SYSDEP_CLASS,
         PAMI_COLL_MCAST_CLASS,
         ring_ar_md>
        Factory;
      };
      // Specializations for Ring templates.
      ///
      /// \brief Ring allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      ///
      // Specify the static name in the class (for debug)
      template<> const char* Ring::Composite::name="Ring";
      template<> inline bool Ring::Composite::analyze(PAMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void Ring::Composite::create_schedule(PAMI_SYSDEP_CLASS * map,PAMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OldRingSchedule<PAMI_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
      };

      // class RingReduce::Composite and RingReduce::Factory
      ///
      /// \brief Ring reduce protocol
      ///
      /// Use the OneColorRectRedSched with the ring subschedule
      ///
      namespace RingReduce
      {
         void ring_r_md(pami_metadata_t *m)
         {
           // \todo:  fill in other metadata
           strcpy(&m->name[0],"OldCCMIRingReduce");
         }

        typedef CompositeT
        <CCMI::Schedule::OldRingSchedule<PAMI_SYSDEP_CLASS>,
         CCMI::Executor::Allreduce<PAMI_COLL_MCAST_CLASS,PAMI_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >,
         PAMI_SYSDEP_CLASS,
         PAMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >
          Composite;      //? pipelined or not?

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::RingReduce::Composite,
         PAMI_SYSDEP_CLASS,
         PAMI_COLL_MCAST_CLASS,
         ring_r_md> Factory;
      };
      // Specializations for Ring templates.
      ///
      /// \brief Ring reduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      // Specify the static name in the class (for debug)
      template<> const char* RingReduce::Composite::name="RingReduce";
      template<> inline bool RingReduce::Composite::analyze(PAMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void RingReduce::Composite::create_schedule(PAMI_SYSDEP_CLASS * map,PAMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OldRingSchedule<PAMI_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
