/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/tree_impl.h
 * \brief Composite template implementations
 */

#ifndef __algorithms_protocols_allreduce_tree_impl_h__
#define __algorithms_protocols_allreduce_tree_impl_h__

#include "./TreeCompositeT.h"
#include "./TreeFactoryT.h"

#include "algorithms/executor/AllreduceBase.h"
#include "algorithms/executor/PipelinedAllreduce.h"

#include "algorithms/schedule/TreeBwSchedule.h"
#include "algorithms/schedule/TreeSchedule.h"

#include "algorithms/connmgr/RankBasedConnMgr.h"
#include "algorithms/connmgr/PhaseBasedConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      namespace Tree
      {

        typedef CompositeT
        <CCMI::Schedule::TreeSchedule,CCMI::Executor::AllreduceBase> Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::Tree::Composite> Factory;

        namespace Pipelined
        {

          typedef CompositeT
          <CCMI::Schedule::TreeBwSchedule,CCMI::Executor::PipelinedAllreduce> Composite;

          typedef FactoryT
          <CCMI::ConnectionManager::PhaseBasedConnMgr,CCMI::Adaptor::Allreduce::Tree::Pipelined::Composite> Factory;

        };
      };
      // Specify the static name in the class (for debug)
      template<> const char* Tree::Composite::name="Tree";
      // Specify the static name in the class (for debug)
      template<> const char* Tree::Pipelined::Composite::name="PipelinedTree";
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
