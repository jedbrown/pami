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

#include "Global.h"
#include "components/devices/DmaInterface.h"
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/MemoryFifoRemoteCompletion.h"

#include "util/trace.h"
#include "components/memory/MemoryAllocator.h"

#include <stdlib.h>


#define SPLIT_E_CUTOFF 2048 // 4 packets

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <class T_Model>
      class DmaModelBase : public Interface::DmaModel < MU::DmaModelBase<T_Model> >
      {
        private :

          // This structure is used to track the status when we split an rget
          // into two and send it on opposite E links.
          typedef struct getSplitState
          {
            pami_event_function     finalCallbackFn;
            void                  * finalCallbackFnCookie;
            uint32_t                completionCount;
          } getSplitState_t;

          typedef struct
          {
            uint64_t msg[(sizeof(InjectDescriptorMessage<1>) >> 3) + 1];
          } put_state_t;

          typedef struct
          {
            union
            {
              uint64_t msg1[(sizeof(InjectDescriptorMessage<1,false>) >> 3) + 1];
              uint64_t msg2[(sizeof(InjectDescriptorMessage<2,false>) >> 3) + 1];
            };
            uint8_t e_minus_payload[T_Model::payload_size];
            uint8_t e_plus_payload[T_Model::payload_size];
            getSplitState_t split_e_state;
          } get_state_t;

          typedef union
          {
            MemoryFifoRemoteCompletion::state_t memfifo_remote_completion_state;
            uint8_t injchannel_completion_state[InjChannel::completion_event_state_bytes];
            put_state_t put_state;
            get_state_t get_state;
          } state_t;
        public:

          typedef MU::Context Device;

        protected :

          /////////////////////////////////////////////////////////////////////
          //
          // PAMI::Interface::DmaModel interface implementation -- BEGIN
          //
          /////////////////////////////////////////////////////////////////////

          friend class Interface::DmaModel < MU::DmaModelBase<T_Model> >;

          /// \see PAMI::Device::Interface::DmaModel::DmaModel
          /// \todo set base address table information int he direct put descriptor(s)
          DmaModelBase (MU::Context & device, pami_result_t & status);

          /// \see PAMI::Device::Interface::DmaModel::DmaModel
          ~DmaModelBase ();

          /// \see Device::Interface::DmaModel::getVirtualAddressSupported
          static const bool dma_model_va_supported_impl = false;

          /// \see Device::Interface::DmaModel::getMemoryRegionSupported
          static const bool dma_model_mr_supported_impl = true;

          /// \see Device::Interface::DmaModel::getDmaTransferStateBytes
          static const size_t dma_model_state_bytes_impl  = sizeof(state_t);

          inline bool postDmaPut_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote);

          template <unsigned T_StateBytes>
          inline bool postDmaPut_impl (uint8_t               (&state)[T_StateBytes],
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

          template <unsigned T_StateBytes>
          inline bool postDmaPut_impl (uint8_t               (&state)[T_StateBytes],
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

          template <unsigned T_StateBytes>
          inline bool postDmaGet_impl (uint8_t               (&state)[T_StateBytes],
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

          template <unsigned T_StateBytes>
          inline bool postDmaGet_impl (uint8_t               (&state)[T_StateBytes],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset);

          /////////////////////////////////////////////////////////////////////
          //
          // PAMI::Interface::DmaModel interface implementation -- END
          //
          /////////////////////////////////////////////////////////////////////

          static const size_t payload_size = sizeof(MUSPI_DescriptorBase);

          inline size_t initializeRemoteGetPayload (void                * vaddr,
                                                    uint64_t              local_dst_pa,
                                                    uint64_t              remote_src_pa,
                                                    size_t                bytes,
                                                    size_t                from_task,
                                                    size_t                from_offset,
                                                    pami_event_function   local_fn,
                                                    void                * cookie);

          MUSPI_DescriptorBase   _dput; // "direct put" used for postDmaPut_impl()
          MUSPI_DescriptorBase   _rget; // "remote get" used for postDmaGet_impl()
          MUSPI_DescriptorBase   _rput; // "remote put" _rget payload descriptor
          MemoryFifoRemoteCompletion _remoteCompletion; // Class that does a
          // memory fifo completion
          // ping pong.

          MU::Context          & _context;
          MUHWI_Destination_t  * _myCoords;

          ///
          /// \brief Local get completion event callback for split rgets
          ///
          /// This callback is invoked when completing a split rget.
          /// It will be called twice, once for each half of the rget.
          /// The first time, the counter will be zero, so it just increments
          /// the counter and returns.  The second time, the counter will be
          /// 1, so it invokes the application local completion
          /// callback function and frees the transfer state memory.
          ///
          static void splitComplete (pami_context_t   context,
                                     void           * cookie,
                                     pami_result_t    result)
          {
            getSplitState_t *splitState = (getSplitState_t*) cookie;

            if ( splitState->completionCount == 0 )
              {
                splitState->completionCount++;
              }
            else
              {
                if ( splitState->finalCallbackFn != NULL)
                  {
                    splitState->finalCallbackFn (context,
                                                 splitState->finalCallbackFnCookie,
                                                 PAMI_SUCCESS);
                  }
              }

            return;
          }

          /// \brief Determine if Dest is a Nearest Neighbor in E
          ///
          /// Returns true if the specified dest is only linked with our
          /// node via E links.  No ABCD links.
          ///
          /// \param[in]  dest  ABCDE coords of the destination node
          ///
          /// \retval  true  Dest is nearest neighbor in E
          /// \retval  false Dest is NOT nearest neighbor in E
          ///
          inline bool nearestNeighborInE ( MUHWI_Destination_t dest )
          {
            if ( ( ( dest.Destination.Destination & 0xffffffc0 ) ==
                   ( _myCoords->Destination.Destination & 0xffffffc0 ) ) && // Same ABCD coords?
                 dest.Destination.Destination !=
                 _myCoords->Destination.Destination )      // And different E coords?
              return true;
            else
              return false;
          }

      };

      template <class T_Model>
      DmaModelBase<T_Model>::DmaModelBase (MU::Context & device, pami_result_t & status) :
          Interface::DmaModel < MU::DmaModelBase<T_Model> > (device, status),
          _remoteCompletion (device),
          _context (device)
      {
        COMPILE_TIME_ASSERT(sizeof(MUSPI_DescriptorBase) <= MU::Context::immediate_payload_size);

        TRACE_FORMAT("sizeof(state_t) == %ld, sizeof(get_state_t) = %ld, sizeof(put_state_t) = %ld, T_Model::payload_size = %zu", sizeof(state_t), sizeof(get_state_t), sizeof(put_state_t), T_Model::payload_size);

        _myCoords = __global.mapping.getMuDestinationSelf();

        // Zero-out the descriptor models before initialization
        memset((void *)&_dput, 0, sizeof(_dput));
        memset((void *)&_rget, 0, sizeof(_rget));
        memset((void *)&_rput, 0, sizeof(_rput));

        // --------------------------------------------------------------------
        // Set the common base descriptor fields
        //
        // For the remote get packet, send it using the high priority torus
        // fifo map.  Everything else uses non-priority torus fifos, pinned
        // later based on destination.  This is necessary to avoid deadlock
        // when the remote get fifo fills.  Note that this is in conjunction
        // with using the high priority virtual channel (set elsewhere).
        // --------------------------------------------------------------------
        MUSPI_BaseDescriptorInfoFields_t base;
        memset((void *)&base, 0, sizeof(base));

        base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;

        _dput.setBaseFields (&base);
        _rput.setBaseFields (&base);

        base.Torus_FIFO_Map = MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_PRIORITY;
        _rget.setBaseFields (&base);


        // --------------------------------------------------------------------
        // Set the common point-to-point descriptor fields
        //
        // For the remote get packet, send it on the high priority virtual
        // channel.  Everything else is on the deterministic virtual channel.
        // This is necessary to avoid deadlock when the remote get fifo fills.
        // Note that this is in conjunction with setting the high priority
        // torus fifo map (set elsewhere).
        // --------------------------------------------------------------------
        MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
        memset((void *)&pt2pt, 0, sizeof(pt2pt));

        pt2pt.Misc1 =
          MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
          MUHWI_PACKET_DO_NOT_DEPOSIT |
          MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
        pt2pt.Misc2 =
          MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;

        _dput.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
        _dput.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
        _dput.setPt2PtFields (&pt2pt);
        _rput.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
        _rput.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
        _rput.setPt2PtFields (&pt2pt);
        _rget.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
        _rget.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
        pt2pt.Misc2 =
          MUHWI_PACKET_VIRTUAL_CHANNEL_HIGH_PRIORITY;
        _rget.setPt2PtFields (&pt2pt);


        // --------------------------------------------------------------------
        // Set the remote get descriptor fields
        // --------------------------------------------------------------------
        MUSPI_RemoteGetDescriptorInfoFields_t rget;
        memset((void *)&rget, 0, sizeof(rget));

        rget.Type = MUHWI_PACKET_TYPE_GET;
        _rget.setRemoteGetFields (&rget);


        // --------------------------------------------------------------------
        // Set the direct put descriptor fields
        // --------------------------------------------------------------------
        MUSPI_DirectPutDescriptorInfoFields dput;
        memset((void *)&dput, 0, sizeof(dput));

        dput.Rec_Payload_Base_Address_Id = _context.getGlobalBatId();
        dput.Rec_Payload_Offset          = 0;
        dput.Rec_Counter_Base_Address_Id = _context.getSharedCounterBatId();
        dput.Rec_Counter_Offset          = 0;
        dput.Pacing                      = MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED;

        _dput.setDirectPutFields (&dput);

        _dput.setMessageUnitPacketType (MUHWI_PACKET_TYPE_PUT);


        // --------------------------------------------------------------------
        // Set the remote put descriptor fields
        // --------------------------------------------------------------------
        _rput.setDirectPutFields (&dput);

        _rput.setDestination (*(_context.getMuDestinationSelf()));
        _rput.setMessageUnitPacketType (MUHWI_PACKET_TYPE_PUT);
      };

      template <class T_Model>
      DmaModelBase<T_Model>::~DmaModelBase ()
      {
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

        _context.pinInformation (from_task,
                                 from_offset,
                                 map);

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
// !!!!
// Allocate completion counter, set counter in rput descriptor, set completion function and cookie
// !!!!

        //MUSPI_DescriptorDumpHex((char *)"Remote Put", rput);

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
      template <unsigned T_StateBytes>
      bool DmaModelBase<T_Model>::postDmaPut_impl (uint8_t               (&state)[T_StateBytes],
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
      template <unsigned T_StateBytes>
      bool DmaModelBase<T_Model>::postDmaPut_impl (uint8_t               (&state)[T_StateBytes],
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
        TRACE_FN_ENTER();

        COMPILE_TIME_ASSERT(sizeof(put_state_t) <= T_StateBytes);
        put_state_t * put_state = (put_state_t *) state;

        if (unlikely(bytes == 0)) // eliminate this branch with a template parameter?
          {
#if 0
            // A zero-byte put is defined to be complete after a dma pingpong. This
            // is accomplished via a zero-byte get operation.
            return postDmaGet_impl (state, local_fn, cookie, target_task, target_offset, 0,
                                    local_memregion, local_offset,
                                    remote_memregion, remote_offset);
            // Note: This technique is turned-off because it won't work with the MU.
            // The postDmaGet() implementation is to send a remote-get to the remote
            // node.  When that DmaGet completes, we know the previous put operations
            // have been received at the remote node.  This remote get must be
            // deterministically routed on the same virtual channel as the previous
            // put operations so it flows after the put operations.  But, to avoid
            // deadlocks, the MU requires the remote get to flow on the high-priority
            // virtual channel, which is different from the virtual channel that the
            // data flows on.  Thus, we can't use this remote get tecnhique.
#else
            // A zero-byte put is defined to be complete after a memory fifo pingpong
            // with the remote node.  The ping must be injected into the same injection
            // fifo and torus injection fifo, and must be deterministically routed so it
            // follows behind the previous deterministically-routed put operations.
            // When the ping is received at the remote node, it sends a pong back to our
            // node, causing the local_fn callback to be invoked, indicating completion.

            // Before injecting the ping descriptor, determine the destination,
            // torus fifo map, and injection channel to use to get to the
            // destination.
            MUHWI_Destination_t   dest;
            uint16_t              rfifo;
            uint64_t              map;

            size_t fnum = _context.pinFifo (target_task,
                                            target_offset,
                                            dest,
                                            rfifo,
                                            map);

            InjChannel & channel = _context.injectionGroup.channel[fnum];

            _remoteCompletion.inject( state,
                                      channel,
                                      local_fn,
                                      cookie,
                                      map,
                                      dest.Destination.Destination);

            TRACE_FN_EXIT();
            return true;
#endif
          }

        MUHWI_Destination_t   dest;
        uint16_t              rfifo; // not needed for direct put ?
        uint64_t              map;

        size_t fnum = _context.pinFifo (target_task,
                                        target_offset,
                                        dest,
                                        rfifo,
                                        map);

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
            dput->setPayload (local_pa + local_offset, bytes);

            // The global BAT id is constant .. should only need to set
            // the "offset" (second parameter) here.
            dput->setRecPayloadBaseAddressInfo (_context.getGlobalBatId(), remote_pa + remote_offset);

            TRACE_HEXDATA(dput, sizeof(MUHWI_Descriptor_t));

            // Advance the injection fifo tail pointer. This action
            // completes the injection operation.
            uint64_t sequence = channel.injFifoAdvanceDesc ();

            // Set up completion notification if required
            if (likely(local_fn != NULL))
              {
#ifndef OPTIMIZE_AGGREGATE_LATENCY // replace with a template parameter?
                // Check if the descriptor is done.
                if (likely(channel.checkDescComplete (sequence)))
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

                    // Add a completion event for the sequence number associated with
                    // the descriptor that was injected.
                    channel.addCompletionEvent (state, local_fn, cookie, sequence);
                  }
              }
          }
        else
          {
            // Create a simple single-descriptor message
            InjectDescriptorMessage<1> * msg =
              (InjectDescriptorMessage<1> *) put_state->msg;
            new (msg) InjectDescriptorMessage<1> (channel, local_fn, cookie);

            // Clone the single-packet descriptor model into the message
            _dput.clone (msg->desc[0]);

            // Initialize the injection fifo descriptor in-place.
            uint64_t local_pa  = (uint64_t) local_memregion->getBasePhysicalAddress ();
            uint64_t remote_pa = (uint64_t) remote_memregion->getBasePhysicalAddress ();

            msg->desc[0].setDestination (dest);
            msg->desc[0].setTorusInjectionFIFOMap (map);
            msg->desc[0].setPayload (local_pa + local_offset, bytes);

            // The global BAT id is constant .. should only need to set
            // the "offset" (second parameter) here.
            msg->desc[0].setRecPayloadBaseAddressInfo (_context.getGlobalBatId(), remote_pa + remote_offset);

            TRACE_HEXDATA(&msg->desc[0], sizeof(MUHWI_Descriptor_t));

            // Post the message to the injection channel
            channel.post (msg);
          }

        TRACE_FN_EXIT();
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
      template <unsigned T_StateBytes>
      bool DmaModelBase<T_Model>::postDmaGet_impl (uint8_t               (&state)[T_StateBytes],
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
      template <unsigned T_StateBytes>
      bool DmaModelBase<T_Model>::postDmaGet_impl (uint8_t               (&state)[T_StateBytes],
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
        TRACE_FN_ENTER();

        COMPILE_TIME_ASSERT(sizeof(get_state_t) <= T_StateBytes);
        get_state_t * get_state = (get_state_t *) state;

        uint64_t remote_src_pa = (uint64_t) remote_memregion->getBasePhysicalAddress ();
        remote_src_pa += remote_offset;

        uint64_t local_dst_pa = (uint64_t) local_memregion->getBasePhysicalAddress ();
        local_dst_pa += local_offset;

        MUHWI_Destination_t   dest;
        uint16_t              rfifo; // not needed for direct put ?
        uint64_t              map;

        size_t fnum = _context.pinFifo (target_task,
                                        target_offset,
                                        dest,
                                        rfifo,
                                        map);

        InjChannel & channel = _context.injectionGroup.channel[fnum];

        // When the dest is on a line in the E dimension, split the message into
        // two rgets, and flow one DPut in the E+ and the other in the E- to
        // optimize bandwidth.  This is possible because dest and our node are
        // linked via the two E links.
        if ( unlikely ( nearestNeighborInE ( dest ) &&	( bytes >= SPLIT_E_CUTOFF ) ) )
          { // Special E dimension case

            // Get pointers to 2 descriptor slots, if they are available.
            MUHWI_Descriptor_t * desc[2];
            uint64_t descNum = channel.getNextDescriptorMultiple ( 2, desc );

            // If the send queue is empty and there is space in the fifo for
            // both descriptors, continue.
            if (likely( channel.isSendQueueEmpty() && (descNum != (uint64_t) - 1 )))
              {
                // Clone the remote inject model descriptors into the injection fifo slots.
                MUSPI_DescriptorBase * rgetMinus = (MUSPI_DescriptorBase *) desc[0];
                MUSPI_DescriptorBase * rgetPlus  = (MUSPI_DescriptorBase *) desc[1];
                _rget.clone (*rgetMinus);
                _rget.clone (*rgetPlus);

                // Initialize the destination in the rget descriptors.
                rgetMinus->setDestination (dest);
                rgetPlus ->setDestination (dest);

                // Set the remote injection fifo identifiers to E- and E+ respectively.
                uint32_t *rgetInjFifoIds = _context.getRgetInjFifoIds ();
                rgetMinus->setRemoteGetInjFIFOId ( rgetInjFifoIds[8] );
                rgetPlus ->setRemoteGetInjFIFOId ( rgetInjFifoIds[9] );

                // The "split e state" contains
                // - A completion counter that will count how many completion messages
                //   have been received (we are done when two have been received),
                // - The original callback function and cookie.
                getSplitState_t *splitCookie       = & get_state->split_e_state;
                splitCookie->finalCallbackFn       = local_fn;
                splitCookie->finalCallbackFnCookie = cookie;
                splitCookie->completionCount       = 0;

                // Initialize the rget payload descriptor(s) for the E- rget
                void * vaddr;
                uint64_t paddr;
                size_t bytes0 = bytes >> 1;
                size_t bytes1 = bytes - bytes0;

                channel.getDescriptorPayload (desc[0], vaddr, paddr);
                size_t pbytes = static_cast<T_Model*>(this)->
                                initializeRemoteGetPayload (vaddr,
                                                            local_dst_pa,
                                                            remote_src_pa,
                                                            bytes0,
                                                            MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
                                                            MUHWI_PACKET_HINT_EM,
                                                            splitComplete,
                                                            splitCookie);
                rgetMinus->setPayload (paddr, pbytes);

                /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Rget0",desc[0]); */
                /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Dput0",(MUHWI_Descriptor_t*)vaddr); */
                /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Memfifo-Completion0",((MUHWI_Descriptor_t*)vaddr)+1); */

                // Initialize the rget payload descriptor(s) for the E+ rget
                channel.getDescriptorPayload (desc[1], vaddr, paddr);
                pbytes = static_cast<T_Model*>(this)->
                         initializeRemoteGetPayload (vaddr,
                                                     local_dst_pa + bytes0,
                                                     remote_src_pa + bytes0,
                                                     bytes1,
                                                     MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
                                                     MUHWI_PACKET_HINT_EP,
                                                     splitComplete,
                                                     splitCookie);
                rgetPlus->setPayload (paddr, pbytes);

                /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Rget1",desc[1]); */
                /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Dput1",(MUHWI_Descriptor_t*)vaddr); */
                /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Memfifo-Completion1",((MUHWI_Descriptor_t*)vaddr)+1); */

                // Finally, advance the injection fifo tail pointer. This action
                // completes the injection operation.
                channel.injFifoAdvanceDescMultiple(2);

                // The completion callback was not invoked; return false.
                TRACE_FN_EXIT();
                return false;
              }

            // Construct and post a message
            // Create a simple dual-descriptor message
            InjectDescriptorMessage<2, false> * msg =
              (InjectDescriptorMessage<2, false> *) get_state->msg2;
            new (msg) InjectDescriptorMessage<2, false> (channel);

            // Clone the remote inject model descriptors into the injection fifo slots.
            MUSPI_DescriptorBase * rgetMinus = (MUSPI_DescriptorBase *) & msg->desc[0];
            MUSPI_DescriptorBase * rgetPlus  = (MUSPI_DescriptorBase *) & msg->desc[1];
            _rget.clone (*rgetMinus);
            _rget.clone (*rgetPlus);

            // Initialize the destination in the rget descriptors.
            rgetMinus->setDestination (dest);
            rgetPlus ->setDestination (dest);

            // Set the remote injection fifo identifiers to E- and E+ respectively.
            uint32_t *rgetInjFifoIds = _context.getRgetInjFifoIds ();
            rgetMinus->setRemoteGetInjFIFOId ( rgetInjFifoIds[8] );
            rgetPlus ->setRemoteGetInjFIFOId ( rgetInjFifoIds[9] );

            // The "split e state" contains
            // - A completion counter that will count how many completion messages
            //   have been received (we are done when two have been received),
            // - The original callback function and cookie.
            getSplitState_t *splitCookie       = & get_state->split_e_state;
            splitCookie->finalCallbackFn       = local_fn;
            splitCookie->finalCallbackFnCookie = cookie;
            splitCookie->completionCount       = 0;

            // Initialize the rget payload descriptor(s) for the E- rget
            void * e_minus_payload_vaddr = (void *) get_state->e_minus_payload;
            size_t bytes0 = bytes >> 1;
            size_t bytes1 = bytes - bytes0;

            size_t pbytes = static_cast<T_Model*>(this)->
                            initializeRemoteGetPayload (e_minus_payload_vaddr,
                                                        local_dst_pa,
                                                        remote_src_pa,
                                                        bytes0,
                                                        MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
                                                        MUHWI_PACKET_HINT_EM,
                                                        splitComplete,
                                                        splitCookie);
            // Determine the physical address of the rget payload buffer from
            // the model state memory.
            Kernel_MemoryRegion_t memRegion;
            uint32_t rc;
            rc = Kernel_CreateMemoryRegion (&memRegion, e_minus_payload_vaddr, bytes);
            PAMI_assert_debug ( rc == 0 );
            uint64_t paddr = (uint64_t)memRegion.BasePa +
                             ((uint64_t)e_minus_payload_vaddr - (uint64_t)memRegion.BaseVa);
            TRACE_FORMAT("e_minus_payload_vaddr = %p, paddr = %ld (%p)", e_minus_payload_vaddr, paddr, (void *)paddr);

            rgetMinus->setPayload (paddr, T_Model::payload_size);

            /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Rget0",desc[0]); */
            /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Dput0",(MUHWI_Descriptor_t*)vaddr); */
            /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Memfifo-Completion0",((MUHWI_Descriptor_t*)vaddr)+1); */

            // Initialize the rget payload descriptor(s) for the E+ rget
            void * e_plus_payload_vaddr =  (void *) get_state->e_plus_payload;

            pbytes = static_cast<T_Model*>(this)->
                     initializeRemoteGetPayload (e_plus_payload_vaddr,
                                                 local_dst_pa + bytes0,
                                                 remote_src_pa + bytes0,
                                                 bytes1,
                                                 MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
                                                 MUHWI_PACKET_HINT_EP,
                                                 splitComplete,
                                                 splitCookie);
            rgetPlus->setPayload (paddr + T_Model::payload_size, T_Model::payload_size);

            /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Rget1",desc[1]); */
            /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Dput1",(MUHWI_Descriptor_t*)vaddr); */
            /* 		MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Memfifo-Completion1",((MUHWI_Descriptor_t*)vaddr)+1); */

            TRACE_HEXDATA(&msg->desc[0], sizeof(MUHWI_Descriptor_t));
            TRACE_HEXDATA(&msg->desc[1], sizeof(MUHWI_Descriptor_t));

            // Post the message to the injection channel
            channel.post (msg);

            // The completion callback was not invoked; return false.
            TRACE_FN_EXIT();
            return false;
          } // End: Special E dimension case
        else
          { // Not special E dimension case
            size_t ndesc = channel.getFreeDescriptorCountWithUpdate ();

            if (likely(channel.isSendQueueEmpty() && ndesc > 0))
              {
                // There is at least one descriptor slot available in the injection
                // fifo before a fifo-wrap event.
                MUHWI_Descriptor_t * desc = channel.getNextDescriptor ();

                // Clone the remote inject model descriptor into the injection fifo
                MUSPI_DescriptorBase * rget = (MUSPI_DescriptorBase *) desc;
                _rget.clone (*rget);

                // Initialize the injection fifo descriptor in-place.
                rget->setDestination (dest);

                // Determine the remote pinning information
                size_t rfifo =
                  _context.pinFifoToSelf (target_task, map);

                // Set the remote injection fifo identifier
                rget->setRemoteGetInjFIFOId (rfifo);

                //MUSPI_DescriptorDumpHex((char *)"Remote Get", desc);

                // Initialize the rget payload descriptor(s)
                void * vaddr;
                uint64_t paddr;
                channel.getDescriptorPayload (desc, vaddr, paddr);
                size_t pbytes = static_cast<T_Model*>(this)->
                                initializeRemoteGetPayload (vaddr,
                                                            local_dst_pa,
                                                            remote_src_pa,
                                                            bytes,
                                                            map,
                                                            MUHWI_PACKET_HINT_E_NONE,
                                                            local_fn,
                                                            cookie);

                rget->setPayload (paddr, pbytes);
                /*             MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Rget",desc); */
                /*             MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Dput",(MUHWI_Descriptor_t*)vaddr); */
                /*             MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Memfifo-Completion",((MUHWI_Descriptor_t*)vaddr)+1); */

                // Finally, advance the injection fifo tail pointer. This action
                // completes the injection operation.
                channel.injFifoAdvanceDesc();

                // The completion callback was not invoked; return false.
                TRACE_FN_EXIT();
                return false;
              }

            // Construct and post a message
            // Create a simple single-descriptor message
            InjectDescriptorMessage<1, false> * msg =
              (InjectDescriptorMessage<1, false> *) get_state->msg1;
            new (msg) InjectDescriptorMessage<1, false> (channel);

            // Clone the remote inject model descriptor into the message
            MUSPI_DescriptorBase * rget = (MUSPI_DescriptorBase *) & msg->desc[0];
            _rget.clone (msg->desc[0]);

            // Determine the remote pinning information
            size_t rfifo =
              _context.pinFifoToSelf (target_task, map);

            // Set the remote injection fifo identifier
            rget->setRemoteGetInjFIFOId (rfifo);

            //MUSPI_DescriptorDumpHex((char *)"Remote Get", desc);

            // Initialize the rget payload descriptor(s)
            void * vaddr = (void *) get_state->e_minus_payload;
            static_cast<T_Model*>(this)->
            initializeRemoteGetPayload (vaddr,
                                        local_dst_pa,
                                        remote_src_pa,
                                        bytes,
                                        map,
                                        MUHWI_PACKET_HINT_E_NONE,
                                        local_fn,
                                        cookie);

            // Determine the physical address of the rget payload buffer from
            // the model state memory.
            Kernel_MemoryRegion_t memRegion;
            uint32_t rc;
            rc = Kernel_CreateMemoryRegion (&memRegion, vaddr, bytes);
            PAMI_assert_debug ( rc == 0 );
            uint64_t paddr = (uint64_t)memRegion.BasePa +
                             ((uint64_t)vaddr - (uint64_t)memRegion.BaseVa);
            TRACE_FORMAT("vaddr = %p, paddr = %ld (%p)", vaddr, paddr, (void *)paddr);

            rget->setPayload (paddr, T_Model::payload_size);
            /*             MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Rget",desc); */
            /*             MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Dput",(MUHWI_Descriptor_t*)vaddr); */
            /*             MUSPI_DescriptorDumpHex((char*)"DmaModelBase-Memfifo-Completion",((MUHWI_Descriptor_t*)vaddr)+1); */

            TRACE_HEXDATA(&msg->desc[0], sizeof(MUHWI_Descriptor_t));

            // Post the message to the injection channel
            channel.post (msg);

            // The completion callback was not invoked; return false.
            TRACE_FN_EXIT();
            return false;
          } // End: Not special E dimension case
      }
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_DmaModelBase_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
