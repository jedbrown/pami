/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/barrier/BarrierT.h
 * \brief ???
 */

#ifndef __ccmi_collectives_barrier_factory_template_h__
#define __ccmi_collectives_barrier_factory_template_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "./BarrierFactory.h"
#include "interface/CollectiveMapping.h"
#include "algorithms/executor/Barrier.h"
#include "interface/Geometry.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {
      typedef bool (*AnalyzeFn) (Geometry *g);

      ///
      /// \brief Binomial barrier
      ///  
      template <class S, AnalyzeFn afn, class MAP> class BarrierT : public CCMI::Executor::Barrier
      {
        ///
        /// \brief The schedule for binomial barrier protocol
        ///
        S                             _myschedule;

      public:
        ///
        /// \brief Constructor for non-blocking barrier protocols.
        ///
        /// \param[in] mapping     Pointer to mapping class
        /// \param[in] mInterface  The multicast Interface
        /// \param[in] geometry    Geometry object
        ///

        BarrierT  (MAP                     * mapping,
                   CCMI::MultiSend::MultisyncInterface    * mInterface,
                   Geometry                               * geometry) :
        Barrier (geometry->nranks(), geometry->ranks(), geometry->comm(), 0, mInterface),
        _myschedule (mapping, geometry->nranks(), geometry->ranks())
        {
          TRACE_INIT((stderr,"<%#.8X>CCMI::Adaptors::Barrier::BarrierT::ctor(%X)\n",
                     (int)this, geometry->comm()));
          setCommSchedule (&_myschedule);
        }

        static bool analyze (Geometry *geometry)
        {
          return((AnalyzeFn) afn)(geometry);
        }

      }; //-BarrierT


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
      class BarrierFactoryT : private BarrierFactory<MAP>
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
        BarrierFactoryT (CCMI::MultiSend::MultisyncInterface    * minterface,
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
          return new (request, sizeof (CCMI_Executor_t))
          T (this->_mapping, this->_msyncInterface, geometry);
        }

      };  //- BarrierFactoryT

// Old, deprecated interfaces for transition from OldMulticast to Multisync

      ///
      /// \brief Binomial barrier
      ///  
      template <class S, AnalyzeFn afn, class MAP> class OldBarrierT : public CCMI::Executor::OldBarrier
      {
        ///
        /// \brief The schedule for binomial barrier protocol
        ///
        S                             _myschedule;

      public:
        ///
        /// \brief Constructor for non-blocking barrier protocols.
        ///
        /// \param[in] mapping     Pointer to mapping class
        /// \param[in] mInterface  The multicast Interface
        /// \param[in] geometry    Geometry object
        ///

        OldBarrierT  (MAP                     * mapping,
                   CCMI::MultiSend::OldMulticastInterface    * mInterface,
                   Geometry                               * geometry) :
        OldBarrier (geometry->nranks(), geometry->ranks(), geometry->comm(), 0, mInterface),
        _myschedule (mapping, geometry->nranks(), geometry->ranks())
        {
          TRACE_INIT((stderr,"<%#.8X>CCMI::Adaptors::Barrier::BarrierT::ctor(%X)\n",
                     (int)this, geometry->comm()));
          setCommSchedule (&_myschedule);
        }

        static bool analyze (Geometry *geometry)
        {
          return((AnalyzeFn) afn)(geometry);
        }

      }; //- OldBarrierT


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
      class OldBarrierFactoryT : private OldBarrierFactory<MAP>
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
        OldBarrierFactoryT (CCMI::MultiSend::OldMulticastInterface    * minterface,
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
          return new (request, sizeof (CCMI_Executor_t))
          T (this->_mapping, this->_mcastInterface, geometry);
        }

      };  //- OldBarrierFactoryT
//////////////////////////////////////////////////////////////////////////////
    };
  };
};  //namespace CCMI::Adaptor::Barrier

#endif
