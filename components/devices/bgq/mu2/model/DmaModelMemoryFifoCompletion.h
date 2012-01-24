/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/model/DmaModelMemoryFifoCompletion.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_model_DmaModelMemoryFifoCompletion_h__
#define __components_devices_bgq_mu2_model_DmaModelMemoryFifoCompletion_h__

#include "components/devices/bgq/mu2/model/DmaModelBase.h"
#include "components/devices/bgq/mu2/model/MemoryFifoCompletion.h"
#include "components/devices/bgq/mu2/CounterPool.h"
#include "spi/include/kernel/location.h"
#include "hwi/include/bqc/A2_core.h"

#include "util/trace.h"
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
      class DmaModelMemoryFifoCompletion : public MU::DmaModelBase<DmaModelMemoryFifoCompletion,sizeof(MUSPI_DescriptorBase)*2>
      {
        public :

          friend class MU::DmaModelBase<DmaModelMemoryFifoCompletion,sizeof(MUSPI_DescriptorBase)*2>;

          static const bool dma_model_fence_supported = true;
          static const size_t payload_size = sizeof(MUSPI_DescriptorBase)*2;

          /// \see PAMI::Device::Interface::DmaModel::DmaModel
          inline DmaModelMemoryFifoCompletion (MU::Context & context, pami_result_t & status) :
              MU::DmaModelBase<DmaModelMemoryFifoCompletion,sizeof(MUSPI_DescriptorBase)*2> (context, status),
              _completion (context)
          {
            // Determine if this is running on DD2 hardware
            uint32_t pvr; // Processor version register
            int rc;
            rc = Kernel_GetPVR( &pvr );
            assert(rc==0);
            if ( pvr == SPRN_PVR_DD1 )
              _isDD2 = false;
            else
              _isDD2 = true;

            COMPILE_TIME_ASSERT((sizeof(MUSPI_DescriptorBase)*2) <= MU::Context::immediate_payload_size);
          };

          /// \see PAMI::Device::Interface::DmaModel::~DmaModel
          inline ~DmaModelMemoryFifoCompletion () {};

	  /// \brief Initialize Remote Get Payload
	  ///
	  /// Initializes two descriptors in the remote get payload:
	  /// 1. A direct put descriptor that sends the data.  This is deterministically routed
	  ///    so the second descriptor's packet containing the completion indicator follows
	  ///    the data and arrives after all of the data has arrived.
	  /// 2. A memory fifo completion descriptor.
	  ///
          inline size_t initializeRemoteGetPayload (void                * vaddr,
                                                    uint64_t              local_dst_pa,
                                                    uint64_t              remote_src_pa,
                                                    size_t                bytes,
                                                    uint64_t              map,
                                                    uint8_t               hintsE,
                                                    pami_event_function   local_fn,
                                                    void                * cookie)
          {
            TRACE_FN_ENTER();

            // Retreive the route information back to mu context "self"


            // The "immediate" payload contains the remote descriptors
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) vaddr;

            // ----------------------------------------------------------------
            // Initialize the "data mover" descriptor in the rget payload
            // ----------------------------------------------------------------
            // Clone the remote direct put model descriptor into the payload
            _rput.clone (desc[0]);

            // Set the payload of the direct put descriptor to be the physical
            // address of the source buffer on the remote node (from the user's
            // memory region).
            desc[0].setPayload (remote_src_pa, bytes);

            // Set the destination buffer address for the remote direct put.
            //
            // The global BAT id is constant .. should only need to set
            // the "offset" (second parameter) here.
            desc[0].setRecPayloadBaseAddressInfo (_context.getGlobalBatId(), local_dst_pa);

            // Set the pinned fifo/map information
            desc[0].setTorusInjectionFIFOMap (map);

            // Set the ABCD hint bits to zero, and the E hint bits to the caller's
            desc[0].setHints (0, hintsE);

            if (unlikely(local_fn == NULL))
              {
                TRACE_FN_EXIT();
                return sizeof(MUHWI_Descriptor_t);
              }

            // ----------------------------------------------------------------
            // Initialize the "ack to self" descriptor in the rget payload
            // ----------------------------------------------------------------
            _completion.initializeNotifySelfDescriptor (desc[1], local_fn, cookie);

            // Set the pinned fifo/map information
            desc[1].setTorusInjectionFIFOMap (map);

            // Set the ABCD hint bits to zero, and the E hint bits to the caller's
            desc[1].setHints (0, hintsE);

            //MUSPI_DescriptorDumpHex((char *)"Remote Put", &desc[0]);
            //MUSPI_DescriptorDumpHex((char *)"Fifo Completion", &desc[1]);

            TRACE_HEXDATA(desc, 128);
            TRACE_FN_EXIT();

            return sizeof(MUHWI_Descriptor_t) * 2;
          };


          /// \brief Initialize Remote Get Payload (Hybrid)
          ///
          /// Initializes the remote get payload in one of two ways:
          ///
          /// 1. If this is DD2 hardware and there is an MU counter available, 
          ///    the remote get payload contains one descriptor:  a direct put
          ///    that is dynamically routed.
          ///
          /// 2. If this is DD1 hardware (dynamic routing not supported), or no
          ///    MU counter is available, the remote get payload contains two
          ///    descriptors that are deterministically routed (see initializeRemoteGetPayload()).
          ///
          /// This essentially provides a hybrid approach where the data can be dynamically
          /// routed most of the time, but when all MU counters are in use, it reverts back to the
          /// deterministically routed memory fifo completion approach.
          ///
          inline size_t initializeRemoteGetPayloadHybrid (void                * vaddr,
                                                          uint64_t              local_dst_pa,
                                                          uint64_t              remote_src_pa,
                                                          size_t                bytes,
                                                          uint64_t              map,
                                                          uint8_t               hintsE,
                                                          pami_event_function   local_fn,
                                                          void                * cookie)
          {
            TRACE_FN_ENTER();

            uint64_t numCounterPools = _context.getNumCounterPools();
            uint64_t poolID = 0;
            int64_t  counterNum = -1;

            // DD2 hardware and CounterPools exist, allocate a counter.
            if ( ( likely ( ( _isDD2 ) &&  
                            ( numCounterPools != 0 ) ) ) )
            {
              // It is DD2 hardware.  Try to get a MU counter.
              while ( poolID < numCounterPools )
              {
                counterNum = _context.allocateCounter( poolID );
                if ( counterNum >= 0 ) break;
                poolID++;
              }
            }

            // If the counter was successfully allocated, build a dynamically routed
	    // dput descriptor in the rget payload.
	    if ( counterNum >= 0 )
	      {
		// Set the counter info
		_context.setCounter ( poolID,
                                      counterNum,
				      bytes,
				      local_fn,
				      cookie );

		// The "immediate" payload contains the remote descriptor
		MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) vaddr;

		// ----------------------------------------------------------------
		// Initialize the "data mover" descriptor in the rget payload
		// ----------------------------------------------------------------
		// Clone the remote direct put model descriptor into the payload
		_rput.clone (desc[0]);

		// Set the payload of the direct put descriptor to be the physical
		// address of the source buffer on the remote node (from the user's
		// memory region).
		desc[0].setPayload (remote_src_pa, bytes);

		// Set the destination buffer address for the remote direct put.
		//
		// The global BAT id is constant .. should only need to set
		// the "offset" (second parameter) here.
		desc[0].setRecPayloadBaseAddressInfo ( _context.getGlobalBatId(), 
						       local_dst_pa);

		// When not a local transfer, set the pinned fifo/map information to use
		// all 10 directional fifos to maximize performance when using dynamic routing.
		if ( ( map &
		       ( MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0 |
			 MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 ) ) == 0 )
		  {
		    desc[0].setTorusInjectionFIFOMap ( MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM |
						       MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP |
						       MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM |
						       MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP |
						       MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM |
						       MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP |
						       MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM |
						       MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP |
						       MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM |
						       MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP );
		  }
		else // Use local mapping.
		  {
		    desc[0].setTorusInjectionFIFOMap ( map );
		  }
		
		// Set dynamic routing and use zone.
		desc[0].setRouting ( MUHWI_PACKET_USE_DYNAMIC_ROUTING );
		desc[0].setPt2PtVirtualChannel ( MUHWI_PACKET_VIRTUAL_CHANNEL_DYNAMIC );
		desc[0].setZoneRouting ( _context.getDynamicRoutingZone() );
		
		// Set the reception counter atomic address
		desc[0].setRecCounterBaseAddressInfo ( _context.getGlobalBatId(),
						       _context.getCounterAtomicOffset( poolID,
                                                                                        counterNum) );

		// Set the ABCD hint bits to zero, and the E hint bits to the caller's
		desc[0].setHints (0, hintsE);

		TRACE_HEXDATA(desc, 64);
		TRACE_FN_EXIT();

		return sizeof(MUHWI_Descriptor_t);
	      }

	    // If the counter was not successfully allocated, build a deterministically
	    // routed dput descriptor and a memfifo completion descriptor in the
	    // rget payload.
	    else
	      {
		return initializeRemoteGetPayload (vaddr,
						   local_dst_pa,
						   remote_src_pa,
						   bytes,
						   map,
						   hintsE,
						   local_fn,
						   cookie);
	      }
          };

          inline void initializeRemoteGetPayload1ForCommAgent (MUSPI_DescriptorBase * desc,
							       uint64_t              local_dst_pa,
							       uint64_t              remote_src_pa,
							       size_t                bytes,
							       uint64_t              map)
          {
            TRACE_FN_ENTER();

            // ----------------------------------------------------------------
            // Initialize the "data mover" descriptor in the rget payload at "desc"
            // ----------------------------------------------------------------

            // Clone the remote direct put model descriptor into the payload
            _rput.clone (*desc);

            // Set the payload of the direct put descriptor to be the physical
            // address of the source buffer on the remote node (from the user's
            // memory region).
            desc->setPayload (remote_src_pa, bytes);

            // Set the destination buffer address for the remote direct put.
            //
            // The global BAT id is constant .. should only need to set
            // the "offset" (second parameter) here.
            desc->setRecPayloadBaseAddressInfo (_context.getGlobalBatId(), local_dst_pa);

            // Set the pinned fifo/map information
            desc->setTorusInjectionFIFOMap (map);
          };

	  // This builds a memfifo descriptor targeted for our node.
	  // It is set up as a local transfer.
          inline void initializeRemoteGetPayload2ForCommAgent (MUSPI_DescriptorBase * desc,
							       pami_event_function   local_fn,
							       void                * cookie)
          {
            TRACE_FN_ENTER();

            // ----------------------------------------------------------------
            // Initialize the "ack to self memfifo completion" descriptor in the rget payload
            // ----------------------------------------------------------------
            _completion.initializeNotifySelfDescriptor (*desc, local_fn, cookie);

            // Set the pinned fifo/map information
            desc->setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1);

            TRACE_FN_EXIT();
          };

	  // This builds a memfifo descriptor targeted for our node.
	  // It is set up as a remote transfer.
          inline void initializeRemoteGetPayload3ForCommAgent (MUSPI_DescriptorBase * desc,
							       pami_event_function   local_fn,
							       void                * cookie,
							       uint64_t              map)
          {
            TRACE_FN_ENTER();

            // ----------------------------------------------------------------
            // Initialize the "ack to self memfifo completion" descriptor in the rget payload
            // ----------------------------------------------------------------
            _completion.initializeNotifySelfDescriptor (*desc, local_fn, cookie);

            // Set the pinned fifo/map information
            desc->setTorusInjectionFIFOMap (map);

	    // Clear the hints
	    desc->setHints (0, MUHWI_PACKET_HINT_E_NONE);

            TRACE_FN_EXIT();
          };

        protected:

          MemoryFifoCompletion _completion;
          bool                 _isDD2;

      }; // PAMI::Device::MU::DmaModelMemoryFifoCompletion class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
