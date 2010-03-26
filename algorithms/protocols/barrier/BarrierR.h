/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/barrier/BarrierR.h
 * \brief ???
 */

#ifndef __algorithms_protocols_barrier_BarrierR_h__
#define __algorithms_protocols_barrier_BarrierR_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/barrier/BarrierFactory.h"
#include "interface/CollectiveMapping.h"
#include "algorithms/executor/Barrier.h"
#include "interface/Geometry.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {

      ///
      /// \brief Barrier for rectangle schedules
      ///        The R argument chooses a rectangle schedule
      ///
      template <class R, class MAP> class BarrierR : public CCMI::Executor::Barrier
      {
        ///
        /// \brief The schedule for binomial barrier protocol
        ///
        R                              _myschedule;

      public:
        ///
        /// \brief Constructor for non-blocking barrier protocols.
        ///
        /// \param[in] mapping     Pointer to mapping class
        /// \param[in] mInterface  The multicast Interface
        /// \param[in] geometry    Geometry object
        ///
        BarrierR  (MAP                    * mapping,
                   CCMI::MultiSend::MultisyncInterface    * mInterface,
                   Geometry                               * geometry) :
        Barrier (geometry->nranks(), geometry->ranks(), geometry->comm(), 0, mInterface),
        _myschedule ( mapping, *geometry->rectangle())
        {
          TRACE_ADAPTOR((stderr,"<%X>CCMI::Adaptor::Barrier::BarrierR::ctor(%X)\n",
                     (int)this, geometry->comm()));
          setCommSchedule (&_myschedule);
        }

        static bool analyze (Geometry *geometry)
        {
          return geometry->isRectangle() && geometry->nranks() > 1;
        }
      }; //-RectangleBarrier


      ///
      /// \brief Barrier Factory Base class.
      ///
      template <class T, class MAP>
      class BarrierFactoryR : private BarrierFactory<MAP>
      {
      public:
        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        ///
        /// \brief Constructor for barrier factory implementations.
        ///
        BarrierFactoryR (CCMI::MultiSend::MultisyncInterface    * minterface,
                         MAP                          * map,
                         CCMI_mapIdToGeometry                     cb_geometry) :
        BarrierFactory<MAP> (minterface, map, cb_geometry)
        {
        }

        bool Analyze(Geometry *geometry)
        {
          return T::analyze (geometry);
        }

        ///
        /// \brief Generate a non-blocking barrier message.
        ///
        /// \param[in]  request      Opaque memory to maintain internal
        ///                          message state.
        /// \param[in]  geometry     Geometry for the barrier operation
        ///
        /// \retval     executor     Pointer to barrier executor
        ///
        CCMI::Executor::Executor *generate
        (CCMI_Executor_t           * request,
         Geometry                  * geometry)
        {
          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T));
          return new (request) T (this->_mapping, this->_msyncInterface, geometry);
        }

      };  //- BarrierFactoryT

// Old, deprecated interfaces for transition from OldMulticast to Multisync

      ///
      /// \brief Barrier for rectangle schedules
      ///        The R argument chooses a rectangle schedule
      ///
      template <class R, class MAP> class OldBarrierR : public CCMI::Executor::OldBarrier
      {
        ///
        /// \brief The schedule for binomial barrier protocol
        ///
        R                              _myschedule;

      public:
        ///
        /// \brief Constructor for non-blocking barrier protocols.
        ///
        /// \param[in] mapping     Pointer to mapping class
        /// \param[in] mInterface  The multicast Interface
        /// \param[in] geometry    Geometry object
        ///
        OldBarrierR  (MAP                     * mapping,
                   CCMI::MultiSend::OldMulticastInterface    * mInterface,
                   Geometry                               * geometry) :
        OldBarrier (geometry->nranks(), geometry->ranks(), geometry->comm(), 0, mInterface),
        _myschedule ( mapping, *geometry->rectangle())
        {
          TRACE_ADAPTOR((stderr,"<%X>CCMI::Adaptor::Barrier::BarrierR::ctor(%X)\n",
                     (int)this, geometry->comm()));
          setCommSchedule (&_myschedule);
        }

        static bool analyze (Geometry *geometry)
        {
          return geometry->isRectangle() && geometry->nranks() > 1;
        }
      }; //-OldRectangleBarrier


      ///
      /// \brief Barrier Factory Base class.
      ///
      template <class T, class MAP>
      class OldBarrierFactoryR : private OldBarrierFactory<MAP>
      {
      public:
        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        ///
        /// \brief Constructor for barrier factory implementations.
        ///
        OldBarrierFactoryR (CCMI::MultiSend::OldMulticastInterface    * minterface,
                         MAP                          * map,
                         CCMI_mapIdToGeometry                     cb_geometry) :
        OldBarrierFactory<MAP> (minterface, map, cb_geometry)
        {
        }

        bool Analyze(Geometry *geometry)
        {
          return T::analyze (geometry);
        }

        ///
        /// \brief Generate a non-blocking barrier message.
        ///
        /// \param[in]  request      Opaque memory to maintain internal
        ///                          message state.
        /// \param[in]  geometry     Geometry for the barrier operation
        ///
        /// \retval     executor     Pointer to barrier executor
        ///
        CCMI::Executor::Executor *generate
        (CCMI_Executor_t           * request,
         Geometry                  * geometry)
        {
          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T));
          return new (request) T (this->_mapping, this->_mcastInterface, geometry);
        }

      };  //- OldBarrierFactoryR
//////////////////////////////////////////////////////////////////////////////
    };
  };
};  //namespace CCMI::Adaptor::Barrier

#endif
