/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/async_impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __ccmi_adaptor_allreduce_async_impl_h__
#define __ccmi_adaptor_allreduce_async_impl_h__

#include "./AsyncCompositeT.h"
#include "./AsyncFactoryT.h"

#include "algorithms/executor/AllreduceBase.h"

#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/schedule/Rectangle.h"
//#include "algorithms/schedule/TreeBwSchedule.h"

#include "algorithms/connmgr/RankBasedConnMgr.h"
//#include "algorithms/connmgr/ShortTorusConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {

      // class Binomial::AsyncComposite and Binomial::AsyncFactory
      ///
      /// \brief Binomial allreduce protocol
      ///
      /// Use the BinomialTreeSchedule
      /// 
      namespace Binomial
      {
        typedef AsyncCompositeT 
        <CCMI::Schedule::BinomialTreeSchedule,CCMI::Executor::Allreduce,CCMI::CollectiveMapping> AsyncComposite;      

        typedef AsyncFactoryT 
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::Binomial::AsyncComposite,CCMI::CollectiveMapping> AsyncFactory;      
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// 
      // Specify the static name in the class (for debug)
      template<> const char* Binomial::AsyncComposite::name="Binomial";
      template<> bool Binomial::AsyncComposite::analyze(Geometry *geometry){ return true;};
      template<> void Binomial::AsyncComposite::create_schedule(CCMI::CollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color _not_used_)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule(map, geometry->nranks(), geometry->ranks());
      };

      // class ShortBinomial::AsyncComposite and ShortBinomial::AsyncFactory
      ///
      /// \brief Binomial short allreduce protocol
      ///
      /// Use the BinomialTreeSchedule
      /// 
      namespace ShortBinomial
      {
        typedef AsyncCompositeT 
        <CCMI::Schedule::BinomialTreeSchedule,CCMI::Executor::AllreduceBase,CCMI::CollectiveMapping> AsyncComposite;      

        typedef AsyncFactoryT 
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::ShortBinomial::AsyncComposite,CCMI::CollectiveMapping> AsyncFactory;      
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial short allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// 
      // Specify the static name in the class (for debug)
      template<> const char* ShortBinomial::AsyncComposite::name="ShortBinomial";
      template<> bool ShortBinomial::AsyncComposite::analyze(Geometry *geometry){ return true;};
      template<> void ShortBinomial::AsyncComposite::create_schedule(CCMI::CollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color _not_used_)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule(map, geometry->nranks(), geometry->ranks());
      };

    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
