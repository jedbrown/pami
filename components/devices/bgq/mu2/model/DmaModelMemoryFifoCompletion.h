/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/DmaModelMemoryFifoCompletion.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__
#define __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__

#include "components/devices/bgq/mu2/model/DmaModelBase.h"
#include "components/devices/bgq/mu2/model/MemoryFifoCompletion.h"

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

          template <unsigned T_State>
          inline void processCompletion_impl (uint8_t                (&state)[T_State],
                                              InjChannel           * channel,
                                              pami_event_function    fn,
                                              void                 * cookie,
                                              MUSPI_DescriptorBase   (&desc)[1])
          {
            _completion.inject (state, channel, fn, cookie, desc);
          };


          inline size_t initializeRemoteGetPayload (void                * vaddr,
                                                    uint64_t              local_dst_pa,
                                                    uint64_t              remote_src_pa,
                                                    size_t                bytes,
                                                    size_t                from_task,
                                                    size_t                from_offset,
                                                    pami_event_function   local_fn,
                                                    void                * cookie)
          {
            // ----------------------------------------------------------------
            // Initialize the "data mover" descriptor in the rget payload
            // ----------------------------------------------------------------
            MU::DmaModelBase<DmaModelMemoryFifoCompletion> * parent =
              (MU::DmaModelBase<DmaModelMemoryFifoCompletion> *) this;

            size_t pbytes =
              parent->initializeRemoteGetPayload (vaddr, local_dst_pa, remote_src_pa,
                                                  bytes, from_task, from_offset,
                                                  local_fn, cookie);


            // ----------------------------------------------------------------
            // Initialize the "ack to self" descriptor in the rget payload
            // ----------------------------------------------------------------
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) vaddr;
            _completion.initializeNotifySelfDescriptor (desc[1], local_fn, cookie);

            desc[1].setTorusInjectionFIFOMap (desc[0].Torus_FIFO_Map);
            desc[1].setHints (desc[0].PacketHeader.NetworkHeader.pt2pt.Hints,
                              desc[0].PacketHeader.NetworkHeader.pt2pt.Byte2.Byte2); // is this right for 'E' hints?

            return pbytes + sizeof(MUHWI_Descriptor_t);
          };

        protected:

          MemoryFifoCompletion _completion;

      }; // PAMI::Device::MU::DmaModelMemoryFifoCompletion class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//




