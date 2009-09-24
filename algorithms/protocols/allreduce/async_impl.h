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
//#include "algorithms/schedule/Rectangle.h"
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
        <CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Executor::Allreduce<XMI_COLL_MCAST_CLASS,XMI_COLL_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >
          AsyncComposite;
        
        typedef AsyncFactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::Binomial::AsyncComposite,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS>
        AsyncFactory;
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      ///
      // Specify the static name in the class (for debug)
      template<> const char* Binomial::AsyncComposite::name="Binomial";
      template<> inline bool Binomial::AsyncComposite::analyze(XMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void Binomial::AsyncComposite::create_schedule(XMI_COLL_SYSDEP_CLASS * map,XMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color _not_used_)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
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
        <CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Executor::AllreduceBase<XMI_COLL_MCAST_CLASS,XMI_COLL_SYSDEP_CLASS,CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS,
         CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> >
        AsyncComposite;

        typedef AsyncFactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS>,
         CCMI::Adaptor::Allreduce::ShortBinomial::AsyncComposite,
         XMI_COLL_SYSDEP_CLASS,
         XMI_COLL_MCAST_CLASS>
        AsyncFactory;
      };
      // Specializations for Binomial templates.
      ///
      /// \brief Binomial short allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      ///
      // Specify the static name in the class (for debug)
      template<> const char* ShortBinomial::AsyncComposite::name="ShortBinomial";
      template<> inline bool ShortBinomial::AsyncComposite::analyze(XMI_GEOMETRY_CLASS *geometry){ return true;};
      template<> inline void ShortBinomial::AsyncComposite::create_schedule(XMI_COLL_SYSDEP_CLASS * map,XMI_GEOMETRY_CLASS * geometry,CCMI::Schedule::Color _not_used_)
      {
        new (_schedule) CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>(map, geometry->nranks(), geometry->ranks());
      };

    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
