/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file ccmi/adaptor/protocols/allreduce/async_impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __ccmi_adaptor_async_impl_h__
#define __ccmi_adaptor_async_impl_h__

#include "AsyncCompositeT.h"
#include "AsyncFactoryT.h"

#include "collectives/algorithms/executor/AllreduceBase.h"

#include "collectives/algorithms/schedule/BinomialTree.h"

#include "collectives/algorithms/connmgr/RankBasedConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {

      // class ShortBinomial::AsyncComposite and ShortBinomial::AsyncFactory
      ///
      /// \brief Binomial short allreduce protocol
      ///
      /// Use the BinomialTreeSchedule
      /// 
      namespace ShortBinomial
      {
        typedef AsyncCompositeT 
        <CCMI::Schedule::BinomialTreeSchedule,CCMI::Executor::AllreduceBase> AsyncComposite;      
        // Specify the static name in the class (for debug)
        template<> const char* AsyncComposite::name="ShortBinomial";

        typedef AsyncFactoryT 
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::ShortBinomial::AsyncComposite> AsyncFactory;      
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial short allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// 
      template<> bool ShortBinomial::AsyncComposite::analyze(Geometry *geometry){ return true;};
      template<> void ShortBinomial::AsyncComposite::create_schedule(CCMI::Mapping * map,Geometry * geometry,CCMI::Schedule::Color _not_used_)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule(map, geometry->nranks(), geometry->ranks());
      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
