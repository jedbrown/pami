
#ifndef __ccmi_collectives_barrier_factory_template_h__
#define __ccmi_collectives_barrier_factory_template_h__

#include "collectives/algorithms/protocols/ProtocolFactory.h"
#include "./BarrierFactory.h"
#include "collectives/interface/Mapping.h"
#include "collectives/algorithms/executor/Barrier.h"
#include "collectives/interface/Geometry.h"

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
      template <class S, AnalyzeFn afn> class BarrierT : public CCMI::Executor::Barrier
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

        BarrierT  (CCMI::Mapping                     * mapping,
                   CCMI::MultiSend::MulticastInterface    * mInterface,
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
      /// \brief Barrier Factory Base class.
      ///
      template <class T>
      class BarrierFactoryT : private BarrierFactory
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
        BarrierFactoryT (CCMI::MultiSend::MulticastInterface    * minterface,
                         CCMI::Mapping                          * map,
                         CCMI_mapIdToGeometry                     cb_geometry) :
        BarrierFactory (minterface, map, cb_geometry)
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
          return new (request)//, sizeof (CCMI_Executor_t))
          T ((CCMI::Mapping *)_mapping, _mcastInterface, geometry);
        }

      };  //- BarrierFactoryT
    };
  };
};  //namespace CCMI::Adaptor::Barrier

#endif
