/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file collectives/bgp/protocols/allreduce/sync_impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __ccmi_collectives_sync_impl_h__
#define __ccmi_collectives_sync_impl_h__

#include "../allreduce/CompositeT.h"
#include "../allreduce/FactoryT.h"

#include "../../../executor/PipelinedAllreduce.h"
#include "../../../executor/Allreduce.h"

#include "../../../schedule/BinomialTree.h"
#include "../../../schedule/Rectangle.h"
#include "../../../schedule/TreeBwSchedule.h"

#include "../../../connmgr/RankBasedConnMgr.h"

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

    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
