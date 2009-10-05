/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/torus_short_async_impl.h
 * \brief Composite template implementations
 */

#ifndef __ccmi_adaptor_allreduce_torus_short_async_impl_h__
#define __ccmi_adaptor_allreduce_torus_short_async_impl_h__

#include "./ShortAsyncCompositeT.h"
#include "./ShortAsyncFactoryT.h"

#include "algorithms/executor/AllreduceBase.h"

#include "algorithms/schedule/ShortRectangle.h"

#include "algorithms/connmgr/ShortTorusConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {

      namespace ShortRectangle
      {
        typedef AsyncCompositeT
        <CCMI::Schedule::ShortRectangleSchedule,CCMI::Executor::AllreduceBase,CCMI::TorusCollectiveMapping> AsyncComposite;

        typedef AsyncFactoryT
        <CCMI::ConnectionManager::ShortTorusConnMgr,CCMI::Adaptor::Allreduce::ShortRectangle::AsyncComposite,CCMI::TorusCollectiveMapping> AsyncFactory;
      };

      // Specializations for Short Async Rectangle templates.
      ///
      /// \brief Short Rectangle allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      // Specify the static name in the class (for debug)
      template<> const char* ShortRectangle::AsyncComposite::name="ShortRectangle";
      template<> bool ShortRectangle::AsyncComposite::analyze(Geometry *geometry){return(geometry->isRectangle() && (geometry->nranks() > 2));};
      template<> void ShortRectangle::AsyncComposite::create_schedule(CCMI::TorusCollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::ShortRectangleSchedule(map, *geometry->rectangle());
      };

      // class ShortRectangle::ShortAsyncComposite and ShortRectangle::ShortAsyncFactory
      ///
      /// \brief Short Rectangle allreduce protocol specifications
      ///
      /// Use the ShortRectangleSchedule
      ///
      namespace ShortRectangle
      {
        typedef ShortAsyncCompositeT
        <CCMI::Schedule::ShortRectangleSchedule,CCMI::Executor::AllreduceBase,CCMI::TorusCollectiveMapping> ShortAsyncComposite;

        typedef ShortAsyncFactoryT
        <CCMI::ConnectionManager::ShortTorusConnMgr,CCMI::Adaptor::Allreduce::ShortRectangle::ShortAsyncComposite,CCMI::TorusCollectiveMapping> ShortAsyncFactory;
      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
