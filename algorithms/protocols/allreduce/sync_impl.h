/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/sync_impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __ccmi_adaptor_allreduce_sync_impl_h__
#define __ccmi_adaptor_allreduce_sync_impl_h__

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
        typedef CompositeT
        <CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Executor::Allreduce<XMI_COLL_MCAST_CLASS,XMI_COLL_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> > Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::Binomial::Composite,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS>
        Factory;
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      ///
      // Specify the static name in the class (for debug)
      template<> const char* Binomial::Composite::name="Binomial";
      template<> inline bool Binomial::Composite::analyze(XMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void Binomial::Composite::create_schedule(XMI_COLL_SYSDEP_CLASS * map,XMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
      };

      // class ShortBinomial::Composite and ShortBinomial::Factory
      ///
      /// \brief Binomial short allreduce protocol
      ///
      /// Use the BinomialTreeSchedule
      ///
      namespace ShortBinomial
      {
        typedef CompositeT
        <CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Executor::AllreduceBase<XMI_COLL_MCAST_CLASS,XMI_COLL_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >
        Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::ShortBinomial::Composite,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS>
        Factory;
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial short allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      ///
      // Specify the static name in the class (for debug)
      template<> const char* ShortBinomial::Composite::name="ShortBinomial";
      template<> inline bool ShortBinomial::Composite::analyze(XMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void ShortBinomial::Composite::create_schedule(XMI_COLL_SYSDEP_CLASS * map,XMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
      };

      // class Ring::Composite and Ring::Factory
      ///
      /// \brief Ring allreduce protocol
      ///
      /// Use the RingSchedule
      ///
      namespace Ring
      {
        typedef CompositeT
        <CCMI::Schedule::RingSchedule<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Executor::PipelinedAllreduce<XMI_COLL_MCAST_CLASS,XMI_COLL_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >
        Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::Ring::Composite,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS>
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
      template<> inline bool Ring::Composite::analyze(XMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void Ring::Composite::create_schedule(XMI_COLL_SYSDEP_CLASS * map,XMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::RingSchedule<XMI_COLL_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
      };

      // class RingReduce::Composite and RingReduce::Factory
      ///
      /// \brief Ring reduce protocol
      ///
      /// Use the OneColorRectRedSched with the ring subschedule
      ///
      namespace RingReduce
      {
        typedef CompositeT
        <CCMI::Schedule::RingSchedule<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Executor::Allreduce<XMI_COLL_MCAST_CLASS,XMI_COLL_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >
          Composite;      //? pipelined or not?

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::RingReduce::Composite,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS> Factory;
      };
      // Specializations for Ring templates.
      ///
      /// \brief Ring reduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      // Specify the static name in the class (for debug)
      template<> const char* RingReduce::Composite::name="RingReduce";
      template<> inline bool RingReduce::Composite::analyze(XMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void RingReduce::Composite::create_schedule(XMI_COLL_SYSDEP_CLASS * map,XMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::RingSchedule<XMI_COLL_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
