/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file ccmi/adaptor/protocols/allreduce/sync_impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __ccmi_adaptor_sync_impl_h__
#define __ccmi_adaptor_sync_impl_h__

#include "./CompositeT.h"
#include "./FactoryT.h"

#include "collectives/algorithms/executor/PipelinedAllreduce.h"
#include "collectives/algorithms/executor//Allreduce.h"

#include "collectives/algorithms/schedule/BinomialTree.h"
#include "collectives/algorithms/schedule/Rectangle.h"
#include "collectives/algorithms/schedule/TreeBwSchedule.h"

#include "collectives/algorithms/connmgr/RankBasedConnMgr.h"

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
        <CCMI::Schedule::BinomialTreeSchedule,CCMI::Executor::Allreduce> Composite;      
        // Specify the static name in the class (for debug)
        template<> const char* Composite::name="Binomial";

        typedef FactoryT 
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::Binomial::Composite> Factory;      
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// 
      template<> bool Binomial::Composite::analyze(Geometry *geometry){ return true;};
      template<> void Binomial::Composite::create_schedule(CCMI::Mapping * map,Geometry * geometry)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule(map, geometry->nranks(), geometry->ranks());
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
        <CCMI::Schedule::BinomialTreeSchedule,CCMI::Executor::AllreduceBase> Composite;      
        // Specify the static name in the class (for debug)
        template<> const char* Composite::name="ShortBinomial";

        typedef FactoryT 
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::ShortBinomial::Composite> Factory;      
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial short allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// 
      template<> bool ShortBinomial::Composite::analyze(Geometry *geometry){ return true;};
      template<> void ShortBinomial::Composite::create_schedule(CCMI::Mapping * map,Geometry * geometry)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule(map, geometry->nranks(), geometry->ranks());
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
        <CCMI::Schedule::RingSchedule,CCMI::Executor::PipelinedAllreduce> Composite;      
        // Specify the static name in the class (for debug)
        template<> const char* Composite::name="Ring";

        typedef FactoryT 
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::Ring::Composite> Factory;      
      };
      // Specializations for Ring templates.
      ///
      /// \brief Ring allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      /// 
      template<> bool Ring::Composite::analyze(Geometry *geometry){ return true;};
      template<> void Ring::Composite::create_schedule(CCMI::Mapping * map,Geometry * geometry)
      {
        new (_schedule) CCMI::Schedule::RingSchedule(map, geometry->nranks(), geometry->ranks());
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
        <CCMI::Schedule::RingSchedule,CCMI::Executor::Allreduce> Composite;      //? pipelined or not?
        // Specify the static name in the class (for debug)
        template<> const char* Composite::name="RingReduce";

        typedef FactoryT 
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::RingReduce::Composite> Factory;      
      };
      // Specializations for Ring templates.
      ///
      /// \brief Ring reduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      template<> bool RingReduce::Composite::analyze(Geometry *geometry){ return true;};
      template<> void RingReduce::Composite::create_schedule(CCMI::Mapping * map,Geometry * geometry)
      {
        new (_schedule) CCMI::Schedule::RingSchedule(map, geometry->nranks(), geometry->ranks());
      };

    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
