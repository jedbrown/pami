/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/barrier/torus_impl.h
 * \brief ???
 */
#ifndef   __ccmi_adaptor_torus_multi_color_impl__
#define   __ccmi_adaptor_torus_multi_color_impl__

#include "algorithms/schedule/GiSchedule.h"
#include "algorithms/schedule/LockboxBarrierSchedule.h"
#include "algorithms/schedule/TreeSchedule.h"
#include "algorithms/schedule/ShortRectangle.h"
#include "interface/Geometry.h"
#include "BarrierT.h"
#include "BarrierR.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {
      extern inline bool global_analyze(Geometry *geometry)
      {
        return(geometry->isGlobalContext() && geometry->isGI()); 
      }      

      typedef BarrierT <CCMI::Schedule::GiSchedule, global_analyze, CCMI::TorusCollectiveMapping>             GiBarrier;      
      typedef BarrierT <CCMI::Schedule::LockboxBarrierSchedule, global_analyze, CCMI::TorusCollectiveMapping> LockboxBarrier;
      typedef OldBarrierT <CCMI::Schedule::TreeSchedule, global_analyze, CCMI::TorusCollectiveMapping>        TreeBarrier;
      typedef OldBarrierR <CCMI::Schedule::ShortRectangleSchedule, CCMI::TorusCollectiveMapping>              RectangleBarrier;

      typedef BarrierFactoryT <GiBarrier, CCMI::TorusCollectiveMapping>                  GiBarrierFactory;
      typedef OldBarrierFactoryT <TreeBarrier, CCMI::TorusCollectiveMapping>             TreeBarrierFactory;
      typedef BarrierFactoryT <LockboxBarrier, CCMI::TorusCollectiveMapping>             LockboxBarrierFactory;
      typedef OldBarrierFactoryR <RectangleBarrier, CCMI::TorusCollectiveMapping>        RectangleBarrierFactory;            

      ///
      /// \brief Rectangle lockbox barrier
      ///
      template<class MAP>
      class RectangleLockboxBarrier : public CCMI::Executor::OldBarrier
      {
        ///
        /// \brief The schedule for rectangle lockbox barrier protocol
        ///
        CCMI::Schedule::ShortRectangleSchedule   _myschedule;

      public:
        ///
        /// \brief Constructor for non-blocking barrier protocols.
        ///
        /// \param[in] mapping     Pointer to mapping class
        /// \param[in] mInterface  The multicast Interface
        /// \param[in] geometry    Geometry object
        ///
        RectangleLockboxBarrier  (MAP                          * mapping,
                                  CCMI::MultiSend::OldMulticastInterface    * mInterface,
                                  Geometry                               * geometry) :
        OldBarrier (geometry->nranks(), geometry->ranks(), geometry->comm(), 0, mInterface),
        _myschedule (mapping, 
                     *geometry->rectangle(), 
                     (geometry->rectangle()->ts ==
                      mapping->GetDimLength(CCMI_T_DIM)))
        {
          TRACE_ADAPTOR((stderr,"<%X>CCMI::Adaptor::Barrier::RectangleLockboxBarrier::ctor(%X)\n",
                         (int)this, geometry->comm()));
          setCommSchedule (&_myschedule);
        }

        static bool analyze (Geometry *geometry)
        {
          return geometry->isRectangle() && geometry->nranks() > 1;
        }
      }; //-RectangleLockboxBarrier


      typedef OldBarrierFactoryT<RectangleLockboxBarrier<CCMI::TorusCollectiveMapping>,CCMI::TorusCollectiveMapping> RectangleLockboxBarrierFactory;
    };
  };
};

#endif
