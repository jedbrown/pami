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

#include "components/devices/bgq/mu2/trace.h"
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
                                                    size_t                from_task,
                                                    size_t                from_offset,
                                                    pami_event_function   local_fn,
                                                    void                * cookie)
          {
            TRACE_FN_ENTER();

            // Retreive the route information back to mu context "self"
            uint64_t map;
            uint8_t  hintsABCD;
            uint8_t  hintsE;

            _context.pinInformation (from_task,
                                     from_offset,
                                     map,
                                     hintsABCD,
                                     hintsE);

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
            desc[0].setRecPayloadBaseAddressInfo (0, local_dst_pa);

            desc[0].setTorusInjectionFIFOMap (map);  // is this needed ?
            desc[0].setHints (hintsABCD, hintsE);    // is this needed ?

            // ----------------------------------------------------------------
            // Initialize the "ack to self" descriptor in the rget payload
            // ----------------------------------------------------------------
            _completion.initializeNotifySelfDescriptor (desc[1], local_fn, cookie);

            desc[1].setTorusInjectionFIFOMap (desc[0].Torus_FIFO_Map); // is this needed ?
            desc[1].setHints (desc[0].PacketHeader.NetworkHeader.pt2pt.Hints, // is this needed ?
                              desc[0].PacketHeader.NetworkHeader.pt2pt.Byte2.Byte2); // is this right for 'E' hints?

            TRACE_HEXDATA(desc,128);
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
