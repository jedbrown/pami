/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/rectanglering_dput/Factory.h
 * \brief  Factory for rectangle allreduce composite using
 * a ring sub-schedule and dput multisend.
 */

#ifndef __ccmi_adaptor_allreduce_rectanglering_dput_factory_h__
#define __ccmi_adaptor_allreduce_rectanglering_dput_factory_h__

#include "algorithms/protocols/allreduce/Factory.h"
#include "algorithms/protocols/allreduce/rectanglering_dput/Composite.h"
#include "algorithms/connmgr/TorusConnMgr.h"
#include "algorithms/connmgr/ColorConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      namespace RectangleRingDput
      {
        /// class Factory
        ///
        /// \brief Factory class for rectangle ring allreduce implementation.
        ///
        /// This factory will generate a rectangle ring allreduce.  It differs from
        /// the rectangle ring reduce only in what schedule it uses.
        ///
        /// It uses unexpected message multisend processing.
        ///
        /// It uses a barrier to synchronize the ranks so that receive processing is ready
        ///
        class Factory : public CCMI::Adaptor::Allreduce::Factory<CCMI::TorusCollectiveMapping>
        {
        protected:
          CCMI::ConnectionManager::TorusConnMgr       _tconnmgr;
          CCMI::ConnectionManager::ColorConnMgr       _cconnmgr;

        public:
          ///
          /// \brief Constructor for allreduce factory implementations.
          ///
          Factory(CCMI::TorusCollectiveMapping *mapping,
                  CCMI::MultiSend::OldMulticastInterface *mof,
                  CCMI_mapIdToGeometry cb_geometry,
                  ConfigFlags flags ) :
          CCMI::Adaptor::Allreduce::Factory<CCMI::TorusCollectiveMapping>(mapping, mof, NULL, cb_geometry, flags),
          _tconnmgr(mapping), _cconnmgr()
          {
            TRACE_ALERT((stderr,"<%#.8X>Allreduce::RectangleRingDput::Factory::ctor() ALERT:\n",(int)this));
            TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::RectangleRingDput::Factory::ctor mf<%#X>\n",(int)this,(int) mf));
            setConnectionManager(&_tconnmgr);
            mof->setCallback (cb_receiveHead, this);
          }

          /// NOTE: This is required to make "C" programs link successfully with virtual destructors
          void operator delete(void * p)
          {
            CCMI_abort();
          }

          ///
          /// \brief Generate a non-blocking allreduce message.
          ///
          virtual CCMI::Executor::Composite * generate
          (XMI_CollectiveRequest_t * request,
           XMI_Callback_t            cb_done,
           CCMI_Consistency           consistency,
           Geometry                 * geometry,
           char                     * srcbuf,
           char                     * dstbuf,
           unsigned                   count,
           XMI_Dt                    dtype,
           XMI_Op                    op,
           int                        root = -1 )
          {
            TRACE_ALERT((stderr,"<%#.8X>Allreduce::RectangleRingDput::Factory::generate() ALERT:\n",(int)this));
            TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::RectangleRingDput::Factory::generate() %#X, geometry %#X comm %#X\n",(int)this,
                            sizeof(*this),(int) geometry, (int) geometry->comm()));

            int ideal, max;
            CCMI::Schedule::Color colors[6] = { CCMI::Schedule::NO_COLOR, CCMI::Schedule::NO_COLOR,
              CCMI::Schedule::NO_COLOR, CCMI::Schedule::NO_COLOR,
              CCMI::Schedule::NO_COLOR, CCMI::Schedule::NO_COLOR};

            CCMI::Schedule::OneColorRectangle::getColors(*geometry->rectangle(), ideal, max, colors);

            unsigned numcolors = (geometry->getNumColors() > 0) ? (geometry->getNumColors()) : ideal;
            if(numcolors > (unsigned) ideal)
              numcolors = ideal;

            CCMI_Executor_t *c_request = geometry->getAllreduceCompositeStorage();

            COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >=
                                sizeof(CCMI::Adaptor::Allreduce::RectangleRingDput::Composite));
            CCMI::Adaptor::Allreduce::RectangleRingDput::Composite *allreduce =
            new (c_request)
            Composite(request,
                      _mapping, &_tconnmgr,
                      &_cconnmgr, cb_done,
                      consistency, _moldinterface, geometry,
                      srcbuf, dstbuf, 0, count, dtype, op,
                      _flags, this, numcolors, colors, root);

            geometry->setAllreduceComposite (allreduce);
            allreduce->startBarrier (consistency);

            return allreduce;
          }

          bool Analyze( Geometry * geometry )
          {
            TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::RectangleRingDput::Factory::analyze()%#X,%#X\n",(int)this,
                            geometry->isRectangle(),geometry->nranks()));
            return(geometry->isRectangle() && (geometry->nranks() > 2));
          }

          static inline void _compile_time_assert_ ()
          {
            // Compile time assert
            COMPILE_TIME_ASSERT(sizeof(CCMI::Adaptor::Allreduce::RectangleRingDput::Composite) <= sizeof(CCMI_Executor_t));
          }
        }; // class Factory
      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce::RectangleRingDput

#endif
