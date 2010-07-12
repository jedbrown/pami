/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/model/DmaModelBase.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_model_DmaModelBase_h__
#define __components_devices_bgq_mu2_model_DmaModelBase_h__

#include <spi/include/mu/DescriptorBaseXX.h>

#include "components/devices/DmaInterface.h"
#include "components/devices/bgq/mu2/Context.h"

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

          template <unsigned T_State, unsigned T_Desc>
          inline void processCompletion (uint8_t                (&state)[T_State],
                                         InjChannel           & channel,
                                         pami_event_function    fn,
                                         void                 * cookie,
                                         MUSPI_DescriptorBase   (&desc)[T_Desc]);

        public:

          inline size_t initializeRemoteGetPayload (void                * vaddr,
                                                    uint64_t              local_dst_pa,
                                                    uint64_t              remote_src_pa,
                                                    size_t                bytes,
                                                    size_t                from_task,
                                                    size_t                from_offset,
                                                    pami_event_function   local_fn,
                                                    void                * cookie);


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
          MUSPI_DescriptorBase   _dput; // "direct put" used for postDmaPut_impl()
          MUSPI_DescriptorBase   _rget; // "remote get" used for postDmaGet_impl()
          MUSPI_DescriptorBase   _rput; // "remote put" _rget payload descriptor

          MU::Context          & _context;
      };

      template <class T_Model>
      DmaModelBase<T_Model>::DmaModelBase (MU::Context & device, pami_result_t & status) :
          Interface::DmaModel < MU::DmaModelBase<T_Model>, MU::Context, 128 > (device, status),
          _context (device)
      {
        COMPILE_TIME_ASSERT(sizeof(MUSPI_DescriptorBase) <= MU::Context::immediate_payload_size);

        // Zero-out the descriptor models before initialization
        memset((void *)&_dput, 0, sizeof(_dput));
        memset((void *)&_rget, 0, sizeof(_rget));
        memset((void *)&_rput, 0, sizeof(_rput));

        // --------------------------------------------------------------------
        // Set the common base descriptor fields
        // --------------------------------------------------------------------
        MUSPI_BaseDescriptorInfoFields_t base;
        memset((void *)&base, 0, sizeof(base));

        base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
        base.Payload_Address = 0;
        base.Message_Length  = 0;
        base.Torus_FIFO_Map  = 0;
        base.Dest.Destination.Destination = 0;

        _dput.setBaseFields (&base);
        _rget.setBaseFields (&base);
        _rput.setBaseFields (&base);


        // --------------------------------------------------------------------
        // Set the common point-to-point descriptor fields
        // --------------------------------------------------------------------
        MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
        memset((void *)&pt2pt, 0, sizeof(pt2pt));

        pt2pt.Hints_ABCD = 0;
        pt2pt.Skip       = 0;
        pt2pt.Misc1 =
          MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
          MUHWI_PACKET_DO_NOT_DEPOSIT |
          MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
        pt2pt.Misc2 =
          MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;

        _dput.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
        _dput.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
        _dput.setPt2PtFields (&pt2pt);
        _rget.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
        _rget.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
        _rget.setPt2PtFields (&pt2pt);
        _rput.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
        _rput.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
        _rput.setPt2PtFields (&pt2pt);


        // --------------------------------------------------------------------
        // Set the remote get descriptor fields
        // --------------------------------------------------------------------
        MUSPI_RemoteGetDescriptorInfoFields_t rget;
        memset((void *)&rget, 0, sizeof(rget));

        rget.Type             = MUHWI_PACKET_TYPE_GET;
        rget.Rget_Inj_FIFO_Id = 0;
        _rget.setRemoteGetFields (&rget);


        // --------------------------------------------------------------------
        // Set the direct put descriptor fields
        // --------------------------------------------------------------------
        MUSPI_DirectPutDescriptorInfoFields dput;
        memset((void *)&dput, 0, sizeof(dput));

        //dput.Rec_Payload_Base_Address_Id = ResourceManager::BAT_DEFAULT_ENTRY_NUMBER;
        dput.Rec_Payload_Offset          = 0;
        //dput.Rec_Counter_Base_Address_Id = ResourceManager::BAT_SHAREDCOUNTER_ENTRY_NUMBER;
        dput.Rec_Counter_Offset          = 0;
        dput.Pacing                      = MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED;

        _dput.setDirectPutFields (&dput);
        _dput.setRecCounterBaseAddressInfo (1, 0); // shared reception counter


        // --------------------------------------------------------------------
        // Set the remote put descriptor fields
        // --------------------------------------------------------------------
        _rput.setDirectPutFields (&dput);
        _rput.setRecCounterBaseAddressInfo (1, 0); // shared reception counter
        _rput.setDestination (*(_context.getMuDestinationSelf()));
      };

      template <class T_Model>
      DmaModelBase<T_Model>::~DmaModelBase ()
      {
      };

      template <class T_Model>
      template <unsigned T_State, unsigned T_Desc>
      void DmaModelBase<T_Model>::processCompletion (uint8_t                (&state)[T_State],
                                                     InjChannel           & channel,
                                                     pami_event_function    fn,
                                                     void                 * cookie,
                                                     MUSPI_DescriptorBase   (&desc)[T_Desc])
      {
        static_cast<T_Model*>(this)->processCompletion_impl (state, channel, fn, cookie, desc);
      };

      template <class T_Model>
      inline size_t DmaModelBase<T_Model>::initializeRemoteGetPayload (
        void                * vaddr,
        uint64_t              local_dst_pa,
        uint64_t              remote_src_pa,
        size_t                bytes,
        size_t                from_task,
        size_t                from_offset,
        pami_event_function   local_fn,
        void                * cookie)
      {
        // Retreive the route information back to mu context "self"
        uint64_t map;
        uint8_t  hintsABCD;
        uint8_t  hintsE;

        _context.pinInformation (from_task,
                                 from_offset,
                                 map,
                                 hintsABCD,
                                 hintsE);

        // Clone the remote direct put model descriptor into the payload
        MUSPI_DescriptorBase * rput = (MUSPI_DescriptorBase *) vaddr;
        _rput.clone (*rput);

        //MUSPI_DescriptorBase * rput = (MUSPI_DescriptorBase *) & clone->desc[0];

        // Set the payload of the direct put descriptor to be the physical
        // address of the source buffer on the remote node (from the user's
        // memory region).
        rput->setPayload (remote_src_pa, bytes);

        // Set the destination buffer address for the remote direct put.
        rput->setRecPayloadBaseAddressInfo (0, local_dst_pa);

        rput->setTorusInjectionFIFOMap (map);
        rput->setHints (hintsABCD, hintsE);
// !!!!
// Allocate completion counter, set counter in rput descriptor, set completion function and cookie
// !!!!

        return sizeof(MUHWI_Descriptor_t);
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
        uint16_t              rfifo; // not needed for direct put ?
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _context.pinFifo (target_task,
                                        target_offset,
                                        dest,
                                        rfifo,
                                        map,
                                        hintsABCD,
                                        hintsE);

        InjChannel & channel = _context.injectionGroup.channel[fnum];
        size_t ndesc = channel.getFreeDescriptorCountWithUpdate ();

        if (likely(channel.isSendQueueEmpty() && ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            MUHWI_Descriptor_t * desc = channel.getNextDescriptor ();

            // Clone the direct-put model descriptor into the injection fifo
            MUSPI_DescriptorBase * dput = (MUSPI_DescriptorBase *) desc;
            _dput.clone (*dput);

            // Initialize the injection fifo descriptor in-place.
            uint64_t local_pa  = (uint64_t) local_memregion->getBasePhysicalAddress ();
            uint64_t remote_pa = (uint64_t) remote_memregion->getBasePhysicalAddress ();

            dput->setDestination (dest);
            dput->setTorusInjectionFIFOMap (map);
            dput->setHints (hintsABCD, hintsE);
            dput->setPayload (local_pa + local_offset, bytes);
            dput->setRecPayloadBaseAddressInfo (0, remote_pa + remote_offset);

            // Finish the completion processing and inject the descriptor(s)
            array_t<MUSPI_DescriptorBase, 1> * resized =
              (array_t<MUSPI_DescriptorBase, 1> *) desc;

            processCompletion (state, channel, local_fn, cookie, resized->array);

#if 0
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
                    _context.addToDoneQ (target_task, msg->getWrapper());
#else
                    PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
#endif
                  }
              }

#endif
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
        uint16_t              rfifo; // not needed for direct put ?
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _context.pinFifo (target_task,
                                        target_offset,
                                        dest,
                                        rfifo,
                                        map,
                                        hintsABCD,
                                        hintsE);

        InjChannel & channel = _context.injectionGroup.channel[fnum];
        size_t ndesc = channel.getFreeDescriptorCountWithUpdate ();

        if (likely(channel.isSendQueueEmpty() && ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            MUHWI_Descriptor_t * desc = channel.getNextDescriptor ();

            void * vaddr;
            uint64_t paddr;

            channel.getDescriptorPayload (desc, vaddr, paddr);

            size_t pbytes = static_cast<T_Model*>(this)->
                            initializeRemoteGetPayload (vaddr, local_dst_pa,
                                                        remote_src_pa, bytes,
                                                        local_fn, cookie);

            // Clone the remote inject model descriptor into the injection fifo
            MUSPI_DescriptorBase * rget = (MUSPI_DescriptorBase *) desc;
            _rget.clone (*rget);

            // Initialize the injection fifo descriptor in-place.
            rget->setDestination (dest);
            rget->setTorusInjectionFIFOMap (map);
            rget->setHints (hintsABCD, hintsE);
            rget->setPayload (paddr, bytes);

            // Finally, advance the injection fifo tail pointer. This action
            // completes the injection operation.
            channel.injFifoAdvanceDesc();
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