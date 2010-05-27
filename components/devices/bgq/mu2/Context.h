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

#include "components/devices/bgq/mu2/MemoryFifoPacketHeader.h"
#include "components/devices/bgq/mu2/msg/MessageQueue.h"


#define CONTEXT_ALLOCATES_RESOURCES   1

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      //
      /// \todo Eliminate the need for this class to implement
      ///       Interface::BaseDevice and Interface::PacketDevice
      ///
      class Context : public Interface::BaseDevice<Context>, public Interface::PacketDevice<Context>
      {
        public:
          typedef struct
          {
            pami_event_function fn;
            void *              cookie;
          } notify_t;

          static const size_t LOOKASIDE_PAYLOAD_SIZE = 0x80;

        protected:

          typedef struct
          {
            Interface::RecvFunction_t   f;
            void                      * p;
          } mu_dispatch_t;

          /// Total number of dispatch sets
          static const size_t dispatch_set_count = 256;

          /// Number of dispatch functions in a dispatch set
          static const size_t dispatch_set_size  = 16;

          /// \see PAMI::Device::Interface::RecvFunction_t
          static int noop (void   * metadata,
                           void   * payload,
                           size_t   bytes,
                           void   * recv_func_parm,
                           void   * cookie)
          {
            fprintf (stderr, "Error. Dispatch to unregistered id (%zu).\n", (size_t) recv_func_parm);
            abort();
            return 0;
          };

          /// \see PAMI::Device::Interface::RecvFunction_t
          static int notify (void   * metadata,
                             void   * payload,
                             size_t   bytes,
                             void   * recv_func_parm,
                             void   * cookie)
          {
            notify_t * n = (notify_t *) payload;

            n->fn (recv_func_parm, // a.k.a. "pami_context_t"
                   n->cookie,
                   PAMI_SUCCESS);

            return 0;
          };

#if CONTEXT_ALLOCATES_RESOURCES
          MUSPI_InjFifoSubGroup_t       _ififo_subgroup;
          MUSPI_RecFifoSubGroup_t       _rfifo_subgroup;
          char                        * _injFifoBuf;
          char                        * _recFifoBuf;
          char                        * _lookAsideBuf;
          Kernel_MemoryRegion_t         _lookAsideMregion;
          unsigned                      _ififoid;
          unsigned                      _rfifoid;

          static const size_t INJ_MEMORY_FIFO_SIZE   = 0xFFFFUL;
          static const size_t REC_MEMORY_FIFO_SIZE   = 0xFFFFUL;
          static const size_t INJ_MEMORY_FIFO_NDESC  = 0x400;
#endif

        public:

          /// Number of payload bytes available in a packet.
          /// \todo replace with a constant from SPIs somewhere
          static const size_t payload_size          = 512;

          static const uint16_t dispatch_system_notify = dispatch_set_count * dispatch_set_size - 1;

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
            // Initialize the dispatch table. This 'noop' function will be
            // replaced with an 'unexpected packet' function and queue.
            size_t i;

            size_t n = MU::Context::dispatch_set_count * MU::Context::dispatch_set_size;

            for (i = 0; i < n; i++)
              {
                _dispatch[i].f = noop;
                _dispatch[i].p = (void *) i;
              }

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
            _id_client = id_client;

            // Need to find a way to break this dependency...
            //_client = client;
            //_context = context;

            // ----------------------------------------------------------------
            // Initialize any mu "system" dispatch functions
            _dispatch[dispatch_system_notify].f = notify;
            _dispatch[dispatch_system_notify].p = (void *) NULL; //_context;

            //_dispatch[DISPATCH_SYSTEM_AVAILABLE].f = ;
            //_dispatch[DISPATCH_SYSTEM_AVAILABLE].p = ;
            // ----------------------------------------------------------------


#if CONTEXT_ALLOCATES_RESOURCES
            _ififoid = 0;
            Kernel_InjFifoAttributes_t injFifoAttrs;
            injFifoAttrs.RemoteGet = 0;
            injFifoAttrs.System    = 0;

            //TRACE(("main(): allocate injection fifos\n"));
            Kernel_AllocateInjFifos (_mapping.t(),
                                     &_ififo_subgroup,
                                     1,
                                     &_ififoid,
                                     &injFifoAttrs);

            _injFifoBuf = (char *) memalign (64, INJ_MEMORY_FIFO_SIZE + 1);
            assert ((((uint64_t)_injFifoBuf) % 64) == 0);
            _lookAsideBuf = (char *) malloc ((INJ_MEMORY_FIFO_SIZE + 1) * 2); //lookaside buffer 2xdesc_size

            memset(_injFifoBuf, 0, INJ_MEMORY_FIFO_SIZE + 1);

            Kernel_MemoryRegion_t  mregionInj, mregionRec;
            Kernel_CreateMemoryRegion ( &mregionInj,
                                        _injFifoBuf,
                                        INJ_MEMORY_FIFO_SIZE + 1 );

            Kernel_CreateMemoryRegion ( &_lookAsideMregion,
                                        _lookAsideBuf,
                                        INJ_MEMORY_FIFO_NDESC*LOOKASIDE_PAYLOAD_SIZE);

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

            Kernel_AllocateRecFifos (_mapping.t(),
                                     &_rfifo_subgroup,
                                     1,
                                     &_rfifoid,
                                     recFifoAttrs);

            _recFifoBuf = (char *)memalign (32, REC_MEMORY_FIFO_SIZE + 1);
            assert ((((uint64_t)_recFifoBuf) % 32) == 0);
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
            recFifoEnableBits |= ( 0x0000000000000001ULL <<
                                   ( 15 - ((_mapping.t()/*sgid*/*BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP) + _rfifoid/*RecFifoId*/ )) );
            Kernel_RecFifoEnable ( 0, /* Group ID */
                                   recFifoEnableBits );
#endif

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
            abort();
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContext
          ///
          inline pami_context_t getContext_impl ()
          {
            abort();
            return (pami_context_t) 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContextOffset
          ///
          inline size_t getContextOffset_impl ()
          {
            abort();
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::isInit
          ///
          inline bool isInit_impl ()
          {
            abort();
            return false;
          }

          ///
          /// \copydoc Interface::BaseDevice::peers
          ///
          inline size_t peers_impl ()
          {
            abort();
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::task2peer
          ///
          inline size_t task2peer_impl (size_t task)
          {
            abort();
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::isPeer
          ///
          inline bool isPeer_impl (size_t task)
          {
            abort();
            return false;
          }

          ///
          /// \copydoc Interface::BaseDevice::advance
          ///
          int advance_impl ()
          {
            unsigned events = advanceRecv();

            //abort();
            return events;
          }

          ///
          /// \copydoc Interface::PacketDevice::read
          ///
          inline int read_impl (void * dst, size_t bytes, void * cookie)
          {
            abort();
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

          unsigned advanceRecv();


          ///
          /// \brief
          ///
          ///
          ///
          inline bool registerPacketHandler (size_t                      set,
                                             Interface::RecvFunction_t   fn,
                                             void                      * cookie,
                                             uint16_t                  & id)
          {
            // There are DISPATCH_SET_COUNT sets of dispatch functions.
            // There are DISPATCH_SET_SIZE  dispatch functions in each dispatch set.

            if (set >= MU::Context::dispatch_set_count) return false;

            unsigned i;

            for (i = 0; i < MU::Context::dispatch_set_size; i++)
              {
                id = set * MU::Context::dispatch_set_size + i;

                if (_dispatch[id].f == noop)
                  {
                    _dispatch[id].f = fn;
                    _dispatch[id].p = cookie;

                    return true;
                  }
              }

            return false;
          }

          /// \copydoc Mapping::getMuDestinationSelf
          inline MUHWI_Destination_t * getMuDestinationSelf ()
          {
            //return _global.mapping.getMuDestinationSelf();
            return _mapping.getMuDestinationSelf();
          };

          inline uint16_t getRecptionFifoIdSelf ()
          {
            return _rfifoid;
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
          /// \note The reception fifo id field of the packet header is only
          ///       9 bits. Perhaps a reference to the descriptor to be pinned
          ///       should be passed in and initialized instead of returning
          ///       the reception fifo id and torus fifo map as output.
          ///
          /// \see MUHWI_MessageUnitHeader.Memory_FIFO.Rec_FIFO_Id
          /// \see MUHWI_Descriptor_t.Torus_FIFO_Map
          ///
          /// \param[in]  task   Destination task identifier
          /// \param[in]  offset Destination task context offset identifier
          /// \param[out] dest   Destination task node coordinates
          /// \param[out] ififo  Pinned MUSPI injection fifo structure
          /// \param[out] rfifo  Reception fifo id to address the task+offset
          /// \param[out] map    Pinned MUSPI torus injection fifo map
          /// \param[out] hintsABCD Pinned ABCD torus hints
          /// \param[out] hintsE Pinned E torus hints
          ///
          /// \return Context-relative injection fifo number pinned to the
          ///         task+offset destination
          ///
          inline size_t pinFifo (size_t                task,
                                 size_t                offset,
                                 MUHWI_Destination_t & dest,
                                 MUSPI_InjFifo_t    ** ififo,
                                 uint16_t            & rfifo,
                                 uint64_t            & map,
                                 uint8_t             & hintsABCD,
                                 uint8_t             & hintsE)
          {
#if CONTEXT_ALLOCATES_RESOURCES
            *ififo = MUSPI_IdToInjFifo(_ififoid, &_ififo_subgroup);
#endif

            // Calculate the destination recpetion fifo identifier based on
            // the destination task+offset.  This is important for
            // multi-context support.
///!!!! Works only on single node
            rfifo = _rfifoid + task * 4 /*number of rec fifos per subgrp*/;
///!!!!
            // In loopback we send only on AM
            map =  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM;
            hintsABCD = MUHWI_PACKET_HINT_AM;
            hintsE    = MUHWI_PACKET_HINT_E_NONE;

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
            // In loopback we send only on AM
            map =  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM;
            hintsABCD = MUHWI_PACKET_HINT_AM |
                        MUHWI_PACKET_HINT_B_NONE |
                        MUHWI_PACKET_HINT_C_NONE |
                        MUHWI_PACKET_HINT_D_NONE;
            hintsE    = MUHWI_PACKET_HINT_E_NONE;
          }

          ///
          /// \brief
          ///
          /// This method obtains the next available descriptor slot in the
          /// injection fifo. MU model components may copy, or clone, a
          /// partially initialized descriptor into the injection fifo, then
          /// complete the descriptor initialization in the injection fifo, and
          /// finally update the injection fifo tail pointer.
          ///
          /// \todo Implement a version of this method that does not return
          ///       \c vaddr and \c paddr as some usage scenarios do not need
          ///       this information
          ///
          /// \param[in]  fnum  Context-relative injection fifo number
          /// \param[out] desc  Next available descriptor slot in the fifo
          /// \param[out] vaddr Virtual address of the look-aside payload
          ///                   buffer associated with the descriptor slot
          /// \param[out] paddr Physical address of \c vaddr
          ///
          /// \return   Number of available descriptor slots in the injection
          ///           fifo before a fifo wrap event will occur
          /// \retval 0 The injection fifo is full
          ///
          inline size_t nextInjectionDescriptor (size_t                fnum,
                                                 MUHWI_Descriptor_t ** desc,
                                                 void               ** vaddr,
                                                 uint64_t            * paddr)
          {
#if CONTEXT_ALLOCATES_RESOURCES
            MUSPI_InjFifo_t *ififo = MUSPI_IdToInjFifo(_ififoid, &_ififo_subgroup);
            uint32_t seqno = MUSPI_InjFifoNextDesc(ififo, (void **)desc);
            uint32_t slotid = seqno % (INJ_MEMORY_FIFO_NDESC);
            *vaddr = (void *)(_lookAsideBuf + slotid * LOOKASIDE_PAYLOAD_SIZE);
            *paddr = (uint64_t)(*vaddr) - (uint64_t)_lookAsideMregion.BaseVa + (uint64_t)_lookAsideMregion.BasePa;
            return 1;
#else
            PAMI_abort();
            return 0;
#endif
          }

          ///
          /// \brief
          ///
          /// \param[in] fnum Context-relative injection fifo number
          /// \param[in] msg  Message object to be added to the send queue
          ///                 associated with the injection fifo number
          ///
          inline void post (size_t fnum, MU::MessageQueue::Element * msg)
          {
            _sendq[fnum].post(msg);
            return;
          }

        protected:

          PAMI::Mapping   & _mapping;
          size_t            _id_base;
          size_t            _id_offset;
          size_t            _id_count;
          size_t            _id_client;

          mu_dispatch_t     _dispatch[dispatch_set_count * dispatch_set_size];
          MU::MessageQueue  _sendq[4];

      }; // class     PAMI::Device::MU::Context
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI



inline unsigned PAMI::Device::MU::Context::advanceRecv ()
{
  unsigned packets = 0;
#if CONTEXT_ALLOCATES_RESOURCES
  uint32_t wrap = 0;
  uint32_t cur_bytes = 0;
  uint32_t total_bytes = 0;
  uint32_t cumulative_bytes = 0;
  MemoryFifoPacketHeader *hdr = NULL;

  //TRACE((stderr, ">> RecFifoSubGroup::recFifoPoll(%p)\n", rfifo));
  MUSPI_RecFifo_t * rfifo = MUSPI_IdToRecFifo (_rfifoid, &_rfifo_subgroup);

  while ((total_bytes = MUSPI_getAvailableBytes (rfifo, &wrap)) != 0)
    {
      if (wrap)   //Extra branch over older packet loop
        {
          hdr = (MemoryFifoPacketHeader *) MUSPI_getNextPacketWrap (rfifo, &cur_bytes);
          _dispatch[hdr->getDispatchId()].f(hdr->getMetaData(), hdr + 1, cur_bytes - 32, _dispatch[hdr->getDispatchId()].p, hdr + 1);
          packets++;

          //fprintf(stderr, "Received packet wrap of size %d, total bytes %d\n",
          //      cur_bytes, total_bytes);
        }
      else
        {
          cumulative_bytes = 0;

          while (cumulative_bytes < total_bytes )
            {
              hdr = (MemoryFifoPacketHeader *) MUSPI_getNextPacketOptimized (rfifo, &cur_bytes);
              cumulative_bytes += cur_bytes;
              _dispatch[hdr->getDispatchId()].f(hdr->getMetaData(), hdr + 1, cur_bytes - 32, _dispatch[hdr->getDispatchId()].p, hdr + 1);
              packets++;
              // Touch head for next packet
              //	fprintf(stderr, "Received packet of size %d, cum bytes %d, total bytes %d\n",
              //cur_bytes, cumulative_bytes, total_bytes);
            }
        }

      MUSPI_syncRecFifoHwHead (rfifo);
    }

  //TRACE((stderr, "<< RecFifoSubGroup::recFifoPoll(%p) .. packets = %d\n", rfifo, packets));
#endif
  return packets;
}

#endif // __components_devices_bgq_mu2_Context_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

