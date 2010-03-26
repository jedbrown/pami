/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/torus_async_impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __algorithms_protocols_allreduce_torus_async_impl_h__
#define __algorithms_protocols_allreduce_torus_async_impl_h__

#include "algorithms/protocols/allreduce/AsyncCompositeT.h"
#include "algorithms/protocols/allreduce/AsyncFactoryT.h"

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

      // class Rectangle::AsyncComposite and Rectangle::AsyncFactory
      ///
      /// \brief Rectangle allreduce protocol
      ///
      /// Use the OneColorRectAllredSched with the binomial subschedule
      ///
      namespace Rectangle
      {
        typedef AsyncCompositeT
        <CCMI::Schedule::OneColorRectAllredSched,CCMI::Executor::Allreduce,CCMI::TorusCollectiveMapping> AsyncComposite;

        typedef AsyncFactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::Rectangle::AsyncComposite,CCMI::TorusCollectiveMapping> AsyncFactory;
      };
      // Specializations for Rectangle allreduce templates.
      ///
      /// \brief Rectangle reduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      ///
      // Specify the static name in the class (for debug)
      template<> const char* Rectangle::AsyncComposite::name="Rectangle";
      template<> bool Rectangle::AsyncComposite::analyze(Geometry *geometry){return(geometry->isRectangle() && (geometry->nranks() > 2));};
      template<> void Rectangle::AsyncComposite::create_schedule(CCMI::TorusCollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OneColorRectAllredSched(map, color, *geometry->rectangle(),CCMI::Schedule::OneColorRectRedSched::Binomial);
      };
      template<> CCMI::Schedule::Color Rectangle::AsyncFactory::getOneColor(Geometry * geometry)
      {
        int ideal, max;
        CCMI::Schedule::Color colors[CCMI::Schedule::MAX_COLOR];
        CCMI::Schedule::OneColorRectangle::getColors(*geometry->rectangle(),ideal, max, colors);
        return colors[0];
      }

      // class RectangleReduce::AsyncComposite and RectangleReduce::AsyncFactory
      ///
      /// \brief Rectangle reduce protocol
      ///
      /// Use the OneColorRectRedSched with the binomial subschedule
      ///
      namespace RectangleReduce
      {
        typedef AsyncCompositeT
        <CCMI::Schedule::OneColorRectRedSched,CCMI::Executor::Allreduce,CCMI::TorusCollectiveMapping> AsyncComposite;

        typedef AsyncFactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::RectangleReduce::AsyncComposite,CCMI::TorusCollectiveMapping> AsyncFactory;
      };
      // Specializations for Rectangle reduce templates.
      ///
      /// \brief Rectangle reduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      ///
      // Specify the static name in the class (for debug)
      template<> const char* RectangleReduce::AsyncComposite::name="RectangleReduce";
      template<> bool RectangleReduce::AsyncComposite::analyze(Geometry *geometry){return(geometry->isRectangle() && (geometry->nranks() > 2));};
      template<> void RectangleReduce::AsyncComposite::create_schedule(CCMI::TorusCollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OneColorRectRedSched(map, color, *geometry->rectangle(),CCMI::Schedule::OneColorRectRedSched::Binomial);
      };
      template<> CCMI::Schedule::Color RectangleReduce::AsyncFactory::getOneColor(Geometry * geometry)
      {
        int ideal, max;
        CCMI::Schedule::Color colors[CCMI::Schedule::MAX_COLOR];
        CCMI::Schedule::OneColorRectangle::getColors(*geometry->rectangle(),ideal, max, colors);
        return colors[0];
      }

      // class RectangleRing::AsyncComposite and RectangleRing::AsyncFactory
      ///
      /// \brief Rectangle Ring allreduce protocol
      ///
      /// Use the OneColorRectAllredSched with the ring subschedule
      ///
      namespace RectangleRing
      {
        typedef AsyncCompositeT
        <CCMI::Schedule::OneColorRectAllredSched,CCMI::Executor::PipelinedAllreduce,CCMI::TorusCollectiveMapping> AsyncComposite;

        typedef AsyncFactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::RectangleRing::AsyncComposite,CCMI::TorusCollectiveMapping> AsyncFactory;
      };
      // Specializations for Rectangle Ring templates.
      ///
      /// \brief Rectangle ring allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      ///
      // Specify the static name in the class (for debug)
      template<> const char* RectangleRing::AsyncComposite::name="RectangleRing";
      template<> bool RectangleRing::AsyncComposite::analyze(Geometry *geometry){return(geometry->isRectangle() && (geometry->nranks() > 2));};
      template<> void RectangleRing::AsyncComposite::create_schedule(CCMI::TorusCollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OneColorRectAllredSched(map, color, *geometry->rectangle(),CCMI::Schedule::OneColorRectRedSched::Ring);
      };
      template<> CCMI::Schedule::Color RectangleRing::AsyncFactory::getOneColor(Geometry * geometry)
      {
        int ideal, max;
        CCMI::Schedule::Color colors[CCMI::Schedule::MAX_COLOR];
        CCMI::Schedule::OneColorRectangle::getColors(*geometry->rectangle(),ideal, max, colors);
        return colors[0];
      }

      // class RectangleRingReduce::AsyncComposite and RectangleRingReduce::AsyncFactory
      ///
      /// \brief Rectangle Ring reduce protocol
      ///
      /// Use the OneColorRectRedSched with the ring subschedule
      ///
      namespace RectangleRingReduce
      {
        typedef AsyncCompositeT
        <CCMI::Schedule::OneColorRectRedSched,CCMI::Executor::PipelinedAllreduce,CCMI::TorusCollectiveMapping> AsyncComposite;      //? pipelined or not?

        typedef AsyncFactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::RectangleRingReduce::AsyncComposite,CCMI::TorusCollectiveMapping> AsyncFactory;
      };
      // Specializations for Rectangle Ring templates.
      ///
      /// \brief Rectangle ring reduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      // Specify the static name in the class (for debug)
      template<> const char* RectangleRingReduce::AsyncComposite::name="RectangleRingReduce";
      template<> bool RectangleRingReduce::AsyncComposite::analyze(Geometry *geometry){return(geometry->isRectangle() && (geometry->nranks() > 2));};
      template<> void RectangleRingReduce::AsyncComposite::create_schedule(CCMI::TorusCollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OneColorRectRedSched(map, color, *geometry->rectangle(),CCMI::Schedule::OneColorRectRedSched::Ring);
      };
      template<> CCMI::Schedule::Color RectangleRingReduce::AsyncFactory::getOneColor(Geometry * geometry)
      {
        int ideal, max;
        CCMI::Schedule::Color colors[CCMI::Schedule::MAX_COLOR];
        CCMI::Schedule::OneColorRectangle::getColors(*geometry->rectangle(),ideal, max, colors);
        return colors[0];
      }

    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
