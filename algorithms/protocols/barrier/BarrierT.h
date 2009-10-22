/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/barrier/BarrierT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_barrier_BarrierT_h__
#define __algorithms_protocols_barrier_BarrierT_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/barrier/BarrierFactory.h"
#include "algorithms/executor/Barrier.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {
      typedef bool (*AnalyzeFn) (XMI_GEOMETRY_CLASS *g);

      ///
      /// \brief Binomial barrier
      ///
      template <class T_Schedule, AnalyzeFn afn, class T_Sysdep, class T_Msync, class T_Topology>
      class BarrierT : public CCMI::Executor::Barrier<T_Msync, T_Topology>
      {
        ///
        /// \brief The schedule for binomial barrier protocol
        ///
        T_Schedule                             _myschedule;

      public:
        ///
        /// \brief Constructor for non-blocking barrier protocols.
        ///
        /// \param[in] mapping     Pointer to mapping class
        /// \param[in] mInterface  The multicast Interface
        /// \param[in] geometry    Geometry object
        ///
        BarrierT  (T_Sysdep                     * mapping,
                   T_Msync                      * mInterface,
                   XMI_GEOMETRY_CLASS           * geometry) :
          CCMI::Executor::Barrier<T_Msync, T_Topology> (geometry->nranks(),
                                                        geometry->ranks(),
                                                        geometry->comm(),
                                                        0,
                                                        mInterface),
          _myschedule (mapping, geometry->nranks(), geometry->ranks())
        {
          TRACE_INIT((stderr,"<%#.8X>CCMI::Adaptors::Barrier::BarrierT::ctor(%X)\n",
                     (int)this, geometry->comm()));
          setCommSchedule (&_myschedule);
        }

        static bool analyze (XMI_GEOMETRY_CLASS *geometry)
        {
          return((AnalyzeFn) afn)(geometry);
        }

      }; //-BarrierT

      ///
      /// \brief Barrier Factory Base class.
      ///
      template <class T, class T_Sysdep, class T_Msync>
      class BarrierFactoryT : private BarrierFactory<T_Sysdep, T_Msync>
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
        BarrierFactoryT (T_Msync              * minterface,
                         T_Sysdep             * map,
                         xmi_mapidtogeometry_fn   cb_geometry) :
          BarrierFactory<T_Sysdep, T_Msync> (minterface, map, cb_geometry)
        {
        }

        bool Analyze(XMI_GEOMETRY_CLASS *geometry)
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
         XMI_GEOMETRY_CLASS                  * geometry)
        {
          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T));
          return new (request) T (this->_mapping, this->_msyncInterface, geometry);
        }

      };  //- BarrierFactoryT

// Old, deprecated interfaces for transition from OldMulticast to Multisync

      ///
      /// \brief Binomial barrier
      ///
      template <class T_Schedule, AnalyzeFn afn, class T_Sysdep, class T_Mcast>
      class OldBarrierT : public CCMI::Executor::OldBarrier<T_Mcast>
      {
        ///
        /// \brief The schedule for binomial barrier protocol
        ///
        T_Schedule                             _myschedule;

      public:
        ///
        /// \brief Constructor for non-blocking barrier protocols.
        ///
        /// \param[in] mapping     Pointer to mapping class
        /// \param[in] mInterface  The multicast Interface
        /// \param[in] geometry    Geometry object
        ///

        OldBarrierT  (T_Sysdep            * mapping,
                      T_Mcast             * mInterface,
                      XMI_GEOMETRY_CLASS  * geometry) :
          CCMI::Executor::OldBarrier<T_Mcast> (geometry->nranks(),
                                               geometry->ranks(),
                                               geometry->comm(),
                                               0U,
                                               mInterface),
          _myschedule (mapping, geometry->nranks(), geometry->ranks())
        {
          TRACE_INIT((stderr,"<%#.8X>CCMI::Adaptors::Barrier::BarrierT::ctor(%X)\n",
                     (int)this, geometry->comm()));
          setCommSchedule (&_myschedule);
        }

        static bool analyze (XMI_GEOMETRY_CLASS *geometry)
        {
          return((AnalyzeFn) afn)(geometry);
        }

      }; //- OldBarrierT


      ///
      /// \brief Barrier Factory Base class.
      ///
      template <class T, class T_Sysdep, class T_Mcast>
      class OldBarrierFactoryT : private OldBarrierFactory<T_Sysdep, T_Mcast>
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
        OldBarrierFactoryT (T_Mcast                *minterface,
                            T_Sysdep               *map,
                            xmi_mapidtogeometry_fn  cb_geometry) :
          OldBarrierFactory<T_Sysdep, T_Mcast> (minterface, map, cb_geometry)
        {
        }

        bool Analyze(XMI_GEOMETRY_CLASS *geometry)
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
         XMI_GEOMETRY_CLASS                  * geometry)
        {
          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T));
          return new (request) T (this->_mapping, this->_mcastInterface, geometry);
        }

      };  //- OldBarrierFactoryT
//////////////////////////////////////////////////////////////////////////////
    };
  };
};  //namespace CCMI::Adaptor::Barrier

#endif
