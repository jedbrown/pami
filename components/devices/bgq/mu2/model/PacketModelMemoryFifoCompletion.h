/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/PacketModelMemoryFifoCompletion.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_PacketModelMemoryFifoCompletion_h__
#define __components_devices_bgq_mu2_PacketModelMemoryFifoCompletion_h__

#include "components/devices/bgq/mu2/model/PacketModelBase.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class PacketModelMemoryFifoCompletion : public MU::PacketModelBase<PacketModelMemoryFifoCompletion>
      {
        public :

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          inline PacketModelMemoryFifoCompletion (MU::Context & device) :
             MU::PacketModelBase<PacketModelMemoryFifoCompletion> (device),
             _done (_device)
          {
            MemoryFifoPacketHeader * hdr =
              (MemoryFifoPacketHeader *) & _done.desc[0].PacketHeader;

            // Set the "notify" system dispatch identifier
            hdr->setSinglePacket (true);
            hdr->setDispatchId (MU::Context::dispatch_system_notify);

            // Set the payload information.
            _done.desc[0].setPayload (0, 0);
          };

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          inline ~PacketModelMemoryFifoCompletion () {};

          /// \see PAMI::Device::MU::PacketModelBase::processCompletion
          inline void processCompletion_impl (size_t                fnum,
                                              MUSPI_InjFifo_t     * ififo,
                                              size_t                ndesc,
                                              MUHWI_Descriptor_t  * desc,
                                              pami_event_function   fn,
                                              void                * cookie)
          {
            if (likely(ndesc > 1))
            {
              // There is enough space in the injection fifo to add the
              // "ack to self" memory fifo descriptor.

              // Clone the completion model descriptor into the injection fifo
              MemoryFifoDescriptor * done = (MemoryFifoDescriptor *) & desc[1];
              _done.clone (done);

              // Copy the completion function+cookie into the packet header.
              MU::Context::notify_t * hdr =
                (MU::Context::notify_t *) & done->desc[0].PacketHeader;
              hdr->fn = fn;
              hdr->cookie = cookie;

              // Advance the injection fifo tail pointer. This action
              // completes the injection operation.
// !!!!
// Need a single MU SPI to advance the tail by *2* (or more) descriptors
// !!!!
              MUSPI_InjFifoAdvanceDesc (ififo);
              MUSPI_InjFifoAdvanceDesc (ififo);
            }
            else
            {
              // Perhaps there is only one descriptor slot available due to an
              // injection fifo wrap event.
              MUSPI_InjFifoAdvanceDesc (ififo);

              MUHWI_Descriptor_t * desc;
              void               * vaddr;
              uint64_t             paddr;

              size_t ndesc =
                _device.nextInjectionDescriptor (fnum, &desc, &vaddr, &paddr);

              if (likely(ndesc > 0))
              {
                // Clone the completion model descriptor into the injection fifo
                MemoryFifoDescriptor * done = (MemoryFifoDescriptor *) desc;
                _done.clone (done);

                // Copy the completion function+cookie into the packet header.
                MU::Context::notify_t * hdr =
                  (MU::Context::notify_t *) & done->desc[0].PacketHeader;
                hdr->fn = fn;
                hdr->cookie = cookie;
              }
              else
              {
                // The injection fifo must be full. Create a "notification"
                // message to post to the device.
                PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
              }

            }
          }

          MemoryFifoSelfDescriptor _done;

      }; // PAMI::Device::MU::PacketModelMemoryFifoCompletion class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_PacketModelMemoryFifoCompletion_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//




