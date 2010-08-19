/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/async_impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __algorithms_protocols_allreduce_async_impl_h__
#define __algorithms_protocols_allreduce_async_impl_h__

#include "algorithms/protocols/allreduce/AsyncCompositeT.h"
#include "algorithms/protocols/allreduce/AsyncFactoryT.h"
#include "algorithms/executor/OldAllreduceBase.h"
#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/connmgr/RankBasedConnMgr.h"

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
        <CCMI::Schedule::BinomialTreeSchedule,
         CCMI::Executor::Allreduce<PAMI_COLL_MCAST_CLASS,CCMI::ConnectionManager::RankBasedConnMgr>,
         PAMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr>
          AsyncComposite;

        typedef AsyncFactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,
         CCMI::Adaptor::Allreduce::Binomial::AsyncComposite,
         PAMI_COLL_MCAST_CLASS>
        AsyncFactory;
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      ///
      // Specify the static name in the class (for debug)
      template<> const char* Binomial::AsyncComposite::name="Binomial";
      template<> inline bool Binomial::AsyncComposite::analyze(PAMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void Binomial::AsyncComposite::create_schedule(PAMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color _not_used_)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule(geometry->nranks(), geometry->ranks());
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
        <CCMI::Schedule::BinomialTreeSchedule,
         CCMI::Executor::OldAllreduceBase<PAMI_COLL_MCAST_CLASS,CCMI::ConnectionManager::RankBasedConnMgr>,
         PAMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr>
        AsyncComposite;

        typedef AsyncFactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,
         CCMI::Adaptor::Allreduce::ShortBinomial::AsyncComposite,
         PAMI_COLL_MCAST_CLASS>
        AsyncFactory;
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial short allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      ///
      // Specify the static name in the class (for debug)
      template<> const char* ShortBinomial::AsyncComposite::name="ShortBinomial";
      template<> inline bool ShortBinomial::AsyncComposite::analyze(PAMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void ShortBinomial::AsyncComposite::create_schedule(PAMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color _not_used_)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule(geometry->nranks(), geometry->ranks());
      };

    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
