/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/DmaModelBase.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_DmaModelBase_h__
#define __components_devices_bgq_mu2_DmaModelBase_h__

#include <spi/include/mu/DescriptorBaseXX.h>

#include "components/devices/DmaInterface.h"
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/MultiDirectPutDescriptor.h"
#include "components/devices/bgq/mu2/model/MultiRemoteInjectDescriptor.h"



namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <class T_Model>
      class DmaModelBase : public Interface::DmaModel < MU::DmaModelBase<T_Model>, MU::Context, 128 >
      {
        protected :

          /// \see PAMI::Device::Interface::DmaModel::DmaModel
          DmaModelBase (MU::Context & device, pami_result_t & status);

          /// \see PAMI::Device::Interface::DmaModel::DmaModel
          ~DmaModelBase ();

        public:

          /// \see Device::Interface::DmaModel::getVirtualAddressSupported
          static const size_t dma_model_va_supported = false;

          /// \see Device::Interface::DmaModel::getMemoryRegionSupported
          static const size_t dma_model_mr_supported = true;

          /// \see Device::Interface::DmaModel::getDmaTransferStateBytes
          static const size_t dma_model_state_bytes  = 128;


          inline bool postDmaPut_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote);

          inline bool postDmaPut_impl (uint8_t               (&state)[dma_model_state_bytes],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote);

          inline bool postDmaPut_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset);

          inline bool postDmaPut_impl (uint8_t               (&state)[dma_model_state_bytes],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset);

          inline bool postDmaGet_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote);

          inline bool postDmaGet_impl (uint8_t               (&state)[dma_model_state_bytes],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote);

          inline bool postDmaGet_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset);

          inline bool postDmaGet_impl (uint8_t               (&state)[dma_model_state_bytes],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset);


        protected:
          DirectPutDescriptor            _dput;
          RemoteInjectDescriptor         _rget;

          MU::Context                  & _device;
      };

      template <class T_Model>
      DmaModelBase<T_Model>::DmaModelBase (MU::Context & device, pami_result_t & status) :
          Interface::DmaModel < MU::DmaModelBase<T_Model>, MU::Context, 128 > (device, status),
          _device (device)
      {
      };

      template <class T_Model>
      bool DmaModelBase<T_Model>::postDmaPut_impl (size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   void                * local,
                                                   void                * remote)
      {
        PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        return false;
      }

      template <class T_Model>
      bool DmaModelBase<T_Model>::postDmaPut_impl (uint8_t               (&state)[dma_model_state_bytes],
                                                   pami_event_function   local_fn,
                                                   void                * cookie,
                                                   size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   void                * local,
                                                   void                * remote)
      {
        PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        return false;
      }

      template <class T_Model>
      bool DmaModelBase<T_Model>::postDmaPut_impl (size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   Memregion           * local_memregion,
                                                   size_t                local_offset,
                                                   Memregion           * remote_memregion,
                                                   size_t                remote_offset)
      {
        // The MU device *could* immediately complete the put operation
        // *if* the source data is copied into a payload lookaside buffer
        // as is done for immediate packet operations.
        //
        // For now return 'false' until this optimization is implemented.
        return false;
      }

      template <class T_Model>
      bool DmaModelBase<T_Model>::postDmaPut_impl (uint8_t               (&state)[dma_model_state_bytes],
                                                   pami_event_function   local_fn,
                                                   void                * cookie,
                                                   size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   Memregion           * local_memregion,
                                                   size_t                local_offset,
                                                   Memregion           * remote_memregion,
                                                   size_t                remote_offset)
      {
        if (unlikely(bytes == 0)) // eliminate this branch with a template parameter?
          {
            // A zero-byte put is defined to be complete after a dma pingpong. This
            // is accomplished via a zero-byte get operation.
#if 0
            return postDmaGet_impl (state, local_fn, cookie, target_task, target_offset, 0,
                                    local_memregion, local_offset,
                                    remote_memregion, remote_offset);
#else
            return false;
#endif
          }

        MUHWI_Destination_t   dest;
        MUSPI_InjFifo_t     * ififo;
        uint16_t              rfifo; // not needed for direct put ?
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _device.pinFifo ((size_t) target_task, target_offset, dest,
                                       &ififo, rfifo, map, hintsABCD, hintsE);

        MUHWI_Descriptor_t * desc;
        void               * vaddr; // not needed for direct put ?
        uint64_t             paddr; // not needed for direct put ?

        size_t ndesc =
          _device.nextInjectionDescriptor (fnum, &desc, &vaddr, &paddr);

        if (likely(ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            // Clone the single-packet model descriptor into the injection fifo
            DirectPutDescriptor * dput = (DirectPutDescriptor *) desc;
            _dput.clone (dput);

            // Initialize the injection fifo descriptor in-place.
            uint64_t local_pa  = (uint64_t) local_memregion->getBasePhysicalAddress ();
            uint64_t remote_pa = (uint64_t) remote_memregion->getBasePhysicalAddress ();
            dput->initializeDescriptors (dest, map, hintsABCD, hintsE,
                                         local_pa + local_offset, bytes,
                                         0, remote_pa + remote_offset);

            // Finally, advance the injection fifo tail pointer. This action
            // completes the injection operation.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (ififo);

            // Set up completion notification if required
            if (local_fn != NULL)
              {
#ifndef OPTIMIZE_AGGREGATE_LATENCY // replace with a template parameter?
                // Check if the descriptor is done.
                uint32_t rc = MUSPI_CheckDescComplete (ififo, sequenceNum);

                if (likely(rc == 1))
                  {
                    //local_fn (_context, cookie, PAMI_SUCCESS); // Descriptor is done...notify.
                    local_fn (NULL, cookie, PAMI_SUCCESS); // Descriptor is done...notify.
                  }
                else
#endif
                  {
                    // The descriptor is not done (or was not checked). Save state
                    // information so that the progress of the decriptor can be checked
                    // later and the callback will be invoked when the descriptor is
                    // complete.
#if 0
                    InjFifoMessage * msg = (InjFifoMessage *) state;
                    new (msg) InjFifoMessage (local_fn, cookie, _context, sequenceNum);

                    // Queue it.
                    _device.addToDoneQ (target_task, msg->getWrapper());
#else
                    PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
#endif
                  }
              }
          }
        else
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

        return true;
      };

      template <class T_Model>
      bool DmaModelBase<T_Model>::postDmaGet_impl (size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   void                * local,
                                                   void                * remote)
      {
        PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        return false;
      }

      template <class T_Model>
      bool DmaModelBase<T_Model>::postDmaGet_impl (uint8_t               (&state)[dma_model_state_bytes],
                                                   pami_event_function   local_fn,
                                                   void                * cookie,
                                                   size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   void                * local,
                                                   void                * remote)
      {
        PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        return false;
      }

      template <class T_Model>
      bool DmaModelBase<T_Model>::postDmaGet_impl (size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   Memregion           * local_memregion,
                                                   size_t                local_offset,
                                                   Memregion           * remote_memregion,
                                                   size_t                remote_offset)
      {
        // MU remote get can never be completed "immediately".
        return false;
      }

      template <class T_Model>
      bool DmaModelBase<T_Model>::postDmaGet_impl (uint8_t               (&state)[dma_model_state_bytes],
                                                   pami_event_function   local_fn,
                                                   void                * cookie,
                                                   size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   Memregion           * local_memregion,
                                                   size_t                local_offset,
                                                   Memregion           * remote_memregion,
                                                   size_t                remote_offset)
      {
        uint64_t remote_src_pa = (uint64_t) remote_memregion->getBasePhysicalAddress ();
        remote_src_pa += remote_offset;

        uint64_t local_dst_pa = (uint64_t) local_memregion->getBasePhysicalAddress ();
        local_dst_pa += local_offset;

        MUHWI_Destination_t   dest;
        MUSPI_InjFifo_t     * ififo;
        uint16_t              rfifo; // not needed by remote inject
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _device.pinFifo ((size_t) target_task, target_offset, dest,
                                       &ififo, rfifo, map, hintsABCD, hintsE);

        MUHWI_Descriptor_t * desc;
        void               * vaddr;
        uint64_t             paddr;

        size_t ndesc =
          _device.nextInjectionDescriptor (fnum, &desc, &vaddr, &paddr);

        if (likely(ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            size_t pbytes = static_cast<T_Model*>(this)->
                            initializePayloadDescriptors (vaddr, local_dst_pa,
                                                          remote_src_pa, bytes,
                                                          local_fn, cookie);

            // Clone the remote inject model descriptor into the injection fifo
            RemoteInjectDescriptor * rget = (RemoteInjectDescriptor *) desc;
            _rget.clone (rget);

            // Initialize the injection fifo descriptor in-place.
            rget->initializeDescriptors (dest, map, hintsABCD, hintsE,
                                         paddr, pbytes);

            // Finally, advance the injection fifo tail pointer. This action
            // completes the injection operation.
            MUSPI_InjFifoAdvanceDesc (ififo);
          }

        return true;
      };

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_DmaModelBase_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//


