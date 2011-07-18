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
      class DmaModelMemoryFifoCompletion : public MU::DmaModelBase<DmaModelMemoryFifoCompletion>
      {

        public :

          friend class MU::DmaModelBase<DmaModelMemoryFifoCompletion>;

          /// \see PAMI::Device::Interface::DmaModel::DmaModel
          inline DmaModelMemoryFifoCompletion (MU::Context & context, pami_result_t & status) :
              MU::DmaModelBase<DmaModelMemoryFifoCompletion> (context, status),
              _completion (context)
          {
            COMPILE_TIME_ASSERT((sizeof(MUSPI_DescriptorBase)*2) <= MU::Context::immediate_payload_size);
          };

          /// \see PAMI::Device::Interface::DmaModel::~DmaModel
          inline ~DmaModelMemoryFifoCompletion () {};

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

        protected:

          MemoryFifoCompletion _completion;

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
