/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/torus_sync_impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __algorithms_protocols_allreduce_torus_sync_impl_h__
#define __algorithms_protocols_allreduce_torus_sync_impl_h__

#include "./CompositeT.h"
#include "./FactoryT.h"

#include "algorithms/executor/PipelinedAllreduce.h"
#include "algorithms/executor/Allreduce.h"

#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/schedule/Rectangle.h"
//#include "algorithms/schedule/TreeBwSchedule.h"

#include "algorithms/connmgr/RankBasedConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {

      // class Rectangle::Composite and Rectangle::Factory
      ///
      /// \brief Rectangle allreduce protocol
      ///
      /// Use the OneColorRectAllredSched with the binomial subschedule
      ///
      namespace Rectangle
      {
        typedef CompositeT
        <CCMI::Schedule::OneColorRectAllredSched,CCMI::Executor::Allreduce,CCMI::TorusCollectiveMapping> Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::Rectangle::Composite,CCMI::TorusCollectiveMapping> Factory;
      };
      // Specializations for Rectangle allreduce templates.
      ///
      /// \brief Rectangle reduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      ///
      // Specify the static name in the class (for debug)
      template<> const char* Rectangle::Composite::name="Rectangle";
      template<> bool Rectangle::Composite::analyze(Geometry *geometry){return(geometry->isRectangle() && (geometry->nranks() > 2));};
      template<> void Rectangle::Composite::create_schedule(CCMI::TorusCollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OneColorRectAllredSched(map, color, *geometry->rectangle(),CCMI::Schedule::OneColorRectRedSched::Binomial);
      };
      template<> CCMI::Schedule::Color Rectangle::Factory::getOneColor(Geometry * geometry)
      {
        int ideal, max;
        CCMI::Schedule::Color colors[CCMI::Schedule::MAX_COLOR];
        CCMI::Schedule::OneColorRectangle::getColors(*geometry->rectangle(),ideal, max, colors);
        return colors[0];
      }

      // class RectangleReduce::Composite and RectangleReduce::Factory
      ///
      /// \brief Rectangle reduce protocol
      ///
      /// Use the OneColorRectRedSched with the binomial subschedule
      ///
      namespace RectangleReduce
      {
        typedef CompositeT
        <CCMI::Schedule::OneColorRectRedSched,CCMI::Executor::Allreduce,CCMI::TorusCollectiveMapping> Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::RectangleReduce::Composite,CCMI::TorusCollectiveMapping> Factory;
      };
      // Specializations for Rectangle reduce templates.
      ///
      /// \brief Rectangle reduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      ///
      // Specify the static name in the class (for debug)
      template<> const char* RectangleReduce::Composite::name="RectangleReduce";
      template<> bool RectangleReduce::Composite::analyze(Geometry *geometry){return(geometry->isRectangle() && (geometry->nranks() > 2));};
      template<> void RectangleReduce::Composite::create_schedule(CCMI::TorusCollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OneColorRectRedSched(map, color, *geometry->rectangle(),CCMI::Schedule::OneColorRectRedSched::Binomial);
      };
      template<> CCMI::Schedule::Color RectangleReduce::Factory::getOneColor(Geometry * geometry)
      {
        int ideal, max;
        CCMI::Schedule::Color colors[CCMI::Schedule::MAX_COLOR];
        CCMI::Schedule::OneColorRectangle::getColors(*geometry->rectangle(),ideal, max, colors);
        return colors[0];
      }

      // class RectangleRing::Composite and RectangleRing::Factory
      ///
      /// \brief Rectangle Ring allreduce protocol
      ///
      /// Use the OneColorRectAllredSched with the ring subschedule
      ///
      namespace RectangleRing
      {
        typedef CompositeT
        <CCMI::Schedule::OneColorRectAllredSched,CCMI::Executor::PipelinedAllreduce,CCMI::TorusCollectiveMapping> Composite;

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::RectangleRing::Composite,CCMI::TorusCollectiveMapping> Factory;
      };
      // Specializations for Rectangle Ring templates.
      ///
      /// \brief Rectangle ring allreduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      ///
      // Specify the static name in the class (for debug)
      template<> const char* RectangleRing::Composite::name="RectangleRing";
      template<> bool RectangleRing::Composite::analyze(Geometry *geometry){return(geometry->isRectangle() && (geometry->nranks() > 2));};
      template<> void RectangleRing::Composite::create_schedule(CCMI::TorusCollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OneColorRectAllredSched(map, color, *geometry->rectangle(),CCMI::Schedule::OneColorRectRedSched::Ring);
      };
      template<> CCMI::Schedule::Color RectangleRing::Factory::getOneColor(Geometry * geometry)
      {
        int ideal, max;
        CCMI::Schedule::Color colors[CCMI::Schedule::MAX_COLOR];
        CCMI::Schedule::OneColorRectangle::getColors(*geometry->rectangle(),ideal, max, colors);
        return colors[0];
      }

      // class RectangleRingReduce::Composite and RectangleRingReduce::Factory
      ///
      /// \brief Rectangle Ring reduce protocol
      ///
      /// Use the OneColorRectRedSched with the ring subschedule
      ///
      namespace RectangleRingReduce
      {
        typedef CompositeT
        <CCMI::Schedule::OneColorRectRedSched,CCMI::Executor::PipelinedAllreduce,CCMI::TorusCollectiveMapping> Composite;      //? pipelined or not?

        typedef FactoryT
        <CCMI::ConnectionManager::RankBasedConnMgr,CCMI::Adaptor::Allreduce::RectangleRingReduce::Composite,CCMI::TorusCollectiveMapping> Factory;
      };
      // Specializations for Rectangle Ring templates.
      ///
      /// \brief Rectangle ring reduce protocol specializations
      /// Implement the correct analyze and schedule ctor.
      /// Implement the factory getOneColor
      // Specify the static name in the class (for debug)
      template<> const char* RectangleRingReduce::Composite::name="RectangleRingReduce";
      template<> bool RectangleRingReduce::Composite::analyze(Geometry *geometry){return(geometry->isRectangle() && (geometry->nranks() > 2));};
      template<> void RectangleRingReduce::Composite::create_schedule(CCMI::TorusCollectiveMapping * map,Geometry * geometry,CCMI::Schedule::Color color)
      {
        new (_schedule) CCMI::Schedule::OneColorRectRedSched(map, color, *geometry->rectangle(),CCMI::Schedule::OneColorRectRedSched::Ring);
      };
      template<> CCMI::Schedule::Color RectangleRingReduce::Factory::getOneColor(Geometry * geometry)
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
