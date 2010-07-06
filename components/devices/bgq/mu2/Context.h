/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/Context.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_Context_h__
#define __components_devices_bgq_mu2_Context_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <malloc.h>

#include <spi/include/mu/InjFifo.h>
#include <spi/include/mu/RecFifo.h>
#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/MU.h>

#include "Mapping.h"

#include "components/devices/BaseDevice.h"
#include "components/devices/PacketInterface.h"

#include "components/devices/bgq/mu2/InjGroup.h"
#include "components/devices/bgq/mu2/RecChannel.h"

#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

#define CONTEXT_ALLOCATES_RESOURCES   1

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      ///
      /// \todo Eliminate the need for this class to implement
      ///       Interface::BaseDevice and Interface::PacketDevice
      ///
      class Context : public Interface::BaseDevice<Context>, public Interface::PacketDevice<Context>
      {
        public:

          typedef enum
          {
            PINFIFO_ALGORITHM_CACHED = 0,
            PINFIFO_ALGORITHM_RUNTIME
          } pinfifo_algorithm_t;

          ///
          /// \brief Number of bytes available in a packet payload.
          /// \todo Replace with a constant from SPIs somewhere
          ///
          static const size_t packet_payload_size    = 512;

          ///
          /// \brief Number of bytes in each lookaside payload buffer element
          ///
          static const size_t immediate_payload_size = sizeof(MU::InjGroup::immediate_payload_t);


          ///
          /// \brief foo
          ///
          /// \param[in] mapping   The mapping component is used to translate
          ///                      coordinates to task identifers, etc
          /// \param[in] id_base   The base identifier of the MU::Context with
          ///                      offset zero
          /// \param[in] id_offset Offset from the base identifier for this
          ///                      MU::Context.
          /// \param[in] id_count  Number of MU::Context objects in the context
          ///                      set - all context objects in a set share a
          ///                      common base identifier
          ///
          inline Context (PAMI::Mapping   & mapping,
                          size_t            id_base,
                          size_t            id_offset,
                          size_t            id_count) :
              Interface::BaseDevice<Context> (),
              Interface::PacketDevice<Context> (),
              _mapping (mapping),
              _id_base (id_base),
              _id_offset (id_offset),
              _id_count (id_count)
          {
            TRACE_FN_ENTER();
            TRACE_FN_EXIT();
          };

          ///
          /// \brief Initialize the mu context
          ///
          /// Functionally replaces the implementation of the
          /// Interface::BaseDevice::init interface and only specifies
          /// parameters that are necessary for MU operations.
          ///
          /// \param[in] id_client The client identifier
          // \param[in] client    PAMI communication client
          // \param[in] context   PAMI communication context
          // \param[in] mm        Memory manager for this mu context
          ///
          inline void init (size_t                  id_client//,
                            //pami_client_t           client,
                            //pami_context_t          context
                            //,Memory::MemoryManager * mm
                           )
          {
            TRACE_FN_ENTER();
            _id_client = id_client;

            // Need to find a way to break this dependency...
            //_client = client;
            //_context = context;

#if CONTEXT_ALLOCATES_RESOURCES
            uint32_t subgrpid = (64 / _mapping.tSize()) * _mapping.t() + _id_offset;
            TRACE_FORMAT("_mapping.tSize() = %zu, _mapping.t() = %zu, _id_offset = %zu ==> subgrpid = %d", _mapping.tSize(), _mapping.t(), _id_offset, subgrpid);

            _ififoid = 0;
            Kernel_InjFifoAttributes_t injFifoAttrs;
            injFifoAttrs.RemoteGet = 0;
            injFifoAttrs.System    = 0;

            //TRACE(("main(): allocate injection fifos\n"));
            Kernel_AllocateInjFifos (subgrpid,
                                     &_ififo_subgroup,
                                     1,
                                     &_ififoid,
                                     &injFifoAttrs);

            _injFifoBuf = (char *) memalign (64, INJ_MEMORY_FIFO_SIZE + 1);
            assert ((((uint64_t)_injFifoBuf) % 64) == 0);
            _lookAsideBuf = (InjGroup::immediate_payload_t *)
                            malloc ((INJ_MEMORY_FIFO_SIZE + 1) * sizeof(InjGroup::immediate_payload_t));

            memset(_injFifoBuf, 0, INJ_MEMORY_FIFO_SIZE + 1);

            Kernel_MemoryRegion_t  mregionInj, mregionRec;
            Kernel_CreateMemoryRegion ( &mregionInj,
                                        _injFifoBuf,
                                        INJ_MEMORY_FIFO_SIZE + 1 );

            Kernel_CreateMemoryRegion ( &_lookAsideMregion,
                                        _lookAsideBuf,
                                        INJ_MEMORY_FIFO_NDESC*sizeof(InjGroup::immediate_payload_t));


            // ----------------------------------------------------------------
            // Allocate and initialize the "lookaside completion" array
            // ----------------------------------------------------------------
            _lookAsideCompletionFn = (pami_event_function *) malloc ((INJ_MEMORY_FIFO_NDESC + 1) * sizeof(pami_event_function));
            memset(_lookAsideCompletionFn, 0, (INJ_MEMORY_FIFO_NDESC + 1) * sizeof(pami_event_function));
            _lookAsideCompletionCookie = (void **) malloc ((INJ_MEMORY_FIFO_NDESC + 1) * sizeof(void *));
            memset(_lookAsideCompletionCookie, 0, (INJ_MEMORY_FIFO_NDESC + 1) * sizeof(void *));



            //TRACE(("main(): init injection fifo\n"));
            Kernel_InjFifoInit (&_ififo_subgroup,
                                _ififoid,
                                &mregionInj,
                                (uint64_t) _injFifoBuf -
                                (uint64_t)mregionInj.BaseVa,
                                INJ_MEMORY_FIFO_SIZE);

            _rfifoid = 0;
            Kernel_RecFifoAttributes_t recFifoAttrs[1];
            recFifoAttrs[0].System = 0;


            Kernel_AllocateRecFifos (subgrpid,
                                     &_rfifo_subgroup,
                                     1,
                                     &_rfifoid,
                                     recFifoAttrs);
            TRACE_FORMAT("_rfifoid = %d", _rfifoid);

            _recFifoBuf = (char *)memalign (32, REC_MEMORY_FIFO_SIZE + 1);
            PAMI_assert ((((uint64_t)_recFifoBuf) % 32) == 0);
            memset(_recFifoBuf, 0, REC_MEMORY_FIFO_SIZE + 1);

            Kernel_CreateMemoryRegion ( &mregionRec,
                                        _recFifoBuf,
                                        REC_MEMORY_FIFO_SIZE + 1 );

            //TRACE(("main(): init reception fifo\n"));
            Kernel_RecFifoInit    (& _rfifo_subgroup, _rfifoid,
                                   &mregionRec,
                                   (uint64_t)_recFifoBuf - (uint64_t)mregionRec.BaseVa,
                                   REC_MEMORY_FIFO_SIZE);

            //Activate fifos
            Kernel_InjFifoActivate (&_ififo_subgroup, 1, &_ififoid, KERNEL_INJ_FIFO_ACTIVATE);

            uint64_t recFifoEnableBits = 0;
            recFifoEnableBits |= ( 0x0000000000000001ULL << (15 - ((subgrpid & 0x03) << 2)));
            Kernel_RecFifoEnable ( subgrpid >> 2, //0, /* Group ID */
                                   recFifoEnableBits );
            TRACE_FORMAT("group = %d, recFifoEnableBits = 0x%016lx", subgrpid >> 2, recFifoEnableBits);

            _rfifo = MUSPI_IdToRecFifo(_rfifoid, &_rfifo_subgroup);

            // ----------------------------------------------------------------
            // Initialize the reception channel
            // ----------------------------------------------------------------
            receptionChannel.initialize (_rfifoid,
                                         _rfifo,
                                         _mapping.getMuDestinationSelf(),
                                         NULL);  // \todo This should be the pami_context_t

            // ----------------------------------------------------------------
            // Initialize the injection channel(s)
            // ----------------------------------------------------------------
            injectionGroup.initialize (0,
                                       MUSPI_IdToInjFifo(_ififoid, &_ififo_subgroup),
                                       _lookAsideBuf,
                                       (uint64_t)_lookAsideMregion.BasePa,
                                       _lookAsideCompletionFn,
                                       _lookAsideCompletionCookie,
                                       INJ_MEMORY_FIFO_NDESC,
                                       NULL);  // \todo This should be the pami_context_t

#endif

            TRACE_FN_EXIT();
            return;
          }

#if 1
          // ------------------------------------------------------------------
          // ------------------------------------------------------------------
          //
          // Begin device interface implementations.
          //
          // These implementations should be removed when the device interfaces
          // are eliminated, with the remaining device interfaces mirrored with
          // new model interfaces.
          //
          // ------------------------------------------------------------------
          // ------------------------------------------------------------------

          ///
          /// \copydoc Interface::BaseDevice::init
          ///
          inline int init_impl (Memory::MemoryManager * mm,
                                size_t                  clientid,
                                size_t                  num_ctx,
                                pami_context_t          context,
                                size_t                  contextid)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContext
          ///
          inline pami_context_t getContext_impl ()
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return (pami_context_t) 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContextOffset
          ///
          inline size_t getContextOffset_impl ()
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContextCount
          ///
          inline size_t getContextCount_impl ()
          {
            return _id_count;
          }

          ///
          /// \copydoc Interface::BaseDevice::isInit
          ///
          inline bool isInit_impl ()
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
          }

          ///
          /// \copydoc Interface::BaseDevice::peers
          ///
          inline size_t peers_impl ()
          {
            // All tasks are addressable "peers" to the MU, therefore the peer
            // size is the same as the task size;
            return _mapping.size();
          }

          ///
          /// \copydoc Interface::BaseDevice::task2peer
          ///
          inline size_t task2peer_impl (size_t task)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::isPeer
          ///
          inline bool isPeer_impl (size_t task)
          {
            // All tasks are addressable "peers" to the MU
            return true;
          }

          ///
          /// \copydoc Interface::BaseDevice::advance
          ///
          int advance_impl ()
          {
            TRACE_FN_ENTER();

            unsigned events  = injectionGroup.advance ();
            events          += receptionChannel.advance ();

            TRACE_FN_EXIT();
            return events;
          }

          ///
          /// \copydoc Interface::PacketDevice::read
          ///
          inline int read_impl (void * dst, size_t bytes, void * cookie)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return 0;
          }

          // ------------------------------------------------------------------
          // ------------------------------------------------------------------
          //
          // End device interface implementations.
          //
          // ------------------------------------------------------------------
          // ------------------------------------------------------------------
#endif

          ///
          /// \copydoc MU::RecChannel::registerPacketHandler
          ///
          inline bool registerPacketHandler (size_t                      set,
                                             Interface::RecvFunction_t   fn,
                                             void                      * cookie,
                                             uint16_t                  & id)
          {
            TRACE_FN_ENTER();
            bool status = receptionChannel.registerPacketHandler (set, fn, cookie, id);
            TRACE_FN_EXIT();
            return status;
          }

          /// \copydoc Mapping::getMuDestinationSelf
          inline MUHWI_Destination_t * getMuDestinationSelf ()
          {
            return _mapping.getMuDestinationSelf();
          };

          ///
          /// \brief
          ///
          /// The pinFifo method is used for two purposes: to retrieve the
          /// context-relative injection fifo identification number of the
          /// injection fifo to which communication with the destination
          /// task+offset is pinned, and to provide MUSPI information needed
          /// to initialize and inject a descriptor.
          ///
          /// \see MUHWI_MessageUnitHeader.Memory_FIFO.Rec_FIFO_Id
          /// \see MUHWI_Descriptor_t.Torus_FIFO_Map
          ///
          /// \param[in]  task    Destination task identifier
          /// \param[in]  offset  Destination task context offset identifier
          /// \param[out] dest    Destination task node coordinates
          /// \param[out] rfifo   Reception fifo id to address the task+offset
          /// \param[out] map     Pinned MUSPI torus injection fifo map
          /// \param[out] hintsABCD Pinned ABCD torus hints
          /// \param[out] hintsE  Pinned E torus hints
          ///
          /// \return Context-relative injection fifo number pinned to the
          ///         task+offset destination
          ///
          //template <pinfifo_algorithm_t T>
          inline size_t pinFifo (size_t                task,
                                 size_t                offset,
                                 MUHWI_Destination_t & dest,
                                 uint16_t            & rfifo,
                                 uint64_t            & map,
                                 uint8_t             & hintsABCD,
                                 uint8_t             & hintsE)
          {
            TRACE_FN_ENTER();

            // Calculate the destination recpetion fifo identifier based on
            // the destination task+offset.  This is important for
            // multi-context support.
            size_t tcoord = 0;
            _mapping.getMuDestinationTask(task, dest, tcoord);
            //rfifo = _rfifoid + tcoord * 4 /*number of rec fifos per subgrp*/;
            rfifo = (64 / _mapping.tSize()) * tcoord + (offset << 2);
            TRACE_FORMAT("rfifo = %d", rfifo);

            // In loopback we send only on AM
            map =  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM;
            hintsABCD = MUHWI_PACKET_HINT_AM;
            hintsE    = MUHWI_PACKET_HINT_E_NONE;

            TRACE_FORMAT("(%zu,%zu) -> dest = %08x, rfifo = %d, map = %016lx, hintsABCD = %08x, hintsE = %08x", task, offset, *((uint32_t *) &dest), rfifo, map, hintsABCD, hintsE);
            TRACE_FN_EXIT();
            return  0;
          }

          ///
          /// \param[in]  from_task   Origin task identifier
          /// \param[in]  from_offset Origin task context offset identifier
          /// \param[out] map         Pinned MUSPI torus injection fifo map
          /// \param[out] hintsABCD   Pinned ABCD torus hints
          /// \param[out] hintsE      Pinned E torus hints
          ///
          inline void pinInformation (size_t     from_task,
                                      size_t     from_offset,
                                      uint64_t & map,
                                      uint8_t  & hintsABCD,
                                      uint8_t  & hintsE)
          {
            TRACE_FN_ENTER();
            // In loopback we send only on AM
            map =  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM;
            hintsABCD = MUHWI_PACKET_HINT_AM |
                        MUHWI_PACKET_HINT_B_NONE |
                        MUHWI_PACKET_HINT_C_NONE |
                        MUHWI_PACKET_HINT_D_NONE;
            hintsE    = MUHWI_PACKET_HINT_E_NONE;

            TRACE_FORMAT("(%zu,%zu) -> map = %016lx, hintsABCD = %08x, hintsE = %08x", from_task, from_offset, map, hintsABCD, hintsE);
            TRACE_FN_EXIT();
          }

          RecChannel   receptionChannel; // Reception resources, public access
          InjGroup     injectionGroup;   // Injection resources, public access

        protected:

#if CONTEXT_ALLOCATES_RESOURCES
          MUSPI_InjFifoSubGroup_t       _ififo_subgroup;
          MUSPI_RecFifoSubGroup_t       _rfifo_subgroup;
          char                        * _injFifoBuf;
          char                        * _recFifoBuf;
          InjGroup::immediate_payload_t * _lookAsideBuf;
          Kernel_MemoryRegion_t         _lookAsideMregion;
          pami_event_function         * _lookAsideCompletionFn;
          void                       ** _lookAsideCompletionCookie;
          unsigned                      _ififoid;
          unsigned                      _rfifoid;
          MUSPI_RecFifo_t             * _rfifo;

          static const size_t INJ_MEMORY_FIFO_SIZE   = 0xFFFFUL;
          static const size_t REC_MEMORY_FIFO_SIZE   = 0xFFFFUL;
          static const size_t INJ_MEMORY_FIFO_NDESC  = 0x400;
#endif

          PAMI::Mapping   & _mapping;
          size_t            _id_base;
          size_t            _id_offset;
          size_t            _id_count;
          size_t            _id_client;

      }; // class     PAMI::Device::MU::Context
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#if 0
///
/// \brief pinFifo template specialization for runtime pin calculation
///
/// \todo Hook in to the MU::ResourceManager
///
/// \see MU::Context::pinFifo
///
template <>
size_t PAMI::Device::MU::Context::pinFifo<PAMI::Device::MU::Context::pinfifo_algorithm_t::PINFIFO_ALGORITHM_RUNTIME> (size_t                task,
    size_t                offset,
    MUHWI_Destination_t & dest,
    uint16_t            & rfifo,
    uint64_t            & map,
    uint8_t             & hintsABCD,
    uint8_t             & hintsE)
{
  // return _resource_manager.pinFifo (...);
  return 0;
};
#endif

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_Context_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
