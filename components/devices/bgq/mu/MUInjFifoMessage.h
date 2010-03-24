/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUInjFifoMessage.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_MUInjFifoMessage_h__
#define __components_devices_bgq_mu_MUInjFifoMessage_h__

#include <sys/uio.h>

#include "sys/pami.h"
#include "util/queue/Queue.h"

//#ifdef TRACE
//#error TRACE already defined!
//#else
//#define TRACE(x) fprintf x
//#endif

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class MUInjFifoMessage : public PAMI::Queue::Element
      {
        public:
          inline MUInjFifoMessage (uint64_t sequenceNum = 0) :
              PAMI::Queue::Element (),
              _desc (),
              _wrapper (&_desc)
          {
            _wrapper.setSequenceNumber (sequenceNum);
          }

          inline MUInjFifoMessage (pami_event_function function,
                                   void *             cookie,
                                   pami_context_t      context,
                                   uint64_t           sequenceNum = 0) :
              PAMI::Queue::Element (),
              _desc (),
              _wrapper (&_desc)
          {
            _wrapper.setCallback (function, cookie, context);
            _wrapper.setSequenceNumber (sequenceNum);
          }

          inline MUSPI_DescriptorBase * getDescriptor ()
          {
            return &_desc;
          }

          inline MUDescriptorWrapper * getWrapper ()
          {
            return &_wrapper;
          }

          inline bool isCallbackDesired ()
          {
            return _wrapper.requiresCallback ();
          }

          inline void setSourceBuffer (struct iovec (&iov)[1])
          {
            __iov[0].iov_base = iov[0].iov_base;
            __iov[0].iov_len  = iov[0].iov_len;

            _iov  = &__iov[0];
            _niov = 1;
          }

          inline void setSourceBuffer (void * addr, size_t length)
          {
            __iov[0].iov_base = addr;
            __iov[0].iov_len  = length;

            _iov  = &__iov[0];
            _niov = 1;
          }

          inline void setSourceBuffer (struct iovec (&iov)[2])
          {
            __iov[0].iov_base = iov[0].iov_base;
            __iov[0].iov_len  = iov[0].iov_len;
            __iov[1].iov_base = iov[1].iov_base;
            __iov[1].iov_len  = iov[1].iov_len;

            _iov  = &__iov[0];
            _niov = 2;
          }

          inline void setSourceBuffer (struct iovec * iov,
                                       size_t         niov)
          {
            _iov  = iov;
            _niov = niov;
          }

          inline bool advance (MUSPI_InjFifo_t    * injfifo,
                               MUHWI_Descriptor_t * hwi_desc,
                               void               * payloadVa,
                               void               * payloadPa)
          {
            fprintf (stderr, "MUInjFifoMessage::advance() >>\n");
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            if (_desc.PacketHeader.messageUnitHeader.Packet_Types.Memory_FIFO.Unused1 == 1)
              {
                // Single-packet message
                //
                // Pack the source buffer(s) into the payload buffer, copy the
                // descriptor, set the payload physical address, and advance
                // the injection fifo tail.
                unsigned i;
                uint64_t bytes = 0;
                uint8_t * ptr = (uint8_t *) payloadVa;

                for (i = 0; i < _niov; i++)
                  {
                    memcpy (ptr, _iov[i].iov_base, _iov[i].iov_len);
                    bytes += _iov[i].iov_len;
                    ptr += _iov[i].iov_len;
                  }

                _desc.clone (*desc);
                desc->setPayload ((uint64_t) payloadPa, bytes);

                _wrapper.setSequenceNumber (MUSPI_InjFifoAdvanceDesc (injfifo));
              }
            else
              {
                // Multi-packet message
                //
                // The descriptor should be fully constructed at this point and all
                // that remains is to copy it into the injection fifo and advance
                // the injection fifo tail.
                fprintf (stderr, "MUInjFifoMessage::advance() .. before clone\n");
                _desc.clone (*desc);

                // Advance the injection fifo tail.
                fprintf (stderr, "MUInjFifoMessage::advance() .. before MUSPI_InjFifoAdvanceDesc()\n");
                _wrapper.setSequenceNumber (MUSPI_InjFifoAdvanceDesc (injfifo));
                fprintf (stderr, "MUInjFifoMessage::advance() ..  after MUSPI_InjFifoAdvanceDesc()\n");
              }

            fprintf (stderr, "MUInjFifoMessage::advance() <<\n");
            return true;
          }


        private:
          MUSPI_DescriptorBase    _desc __attribute__((__aligned__(16)));
          MUDescriptorWrapper     _wrapper __attribute__((__aligned__(16)));
//          MUSPI_DescriptorWrapper _wrapper __attribute__((__aligned__(16)));
          struct iovec   __iov[2];
          struct iovec * _iov;
          size_t         _niov;

      }; // PAMI::Device::MU::MUInjFifoMessage class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

//#undef TRACE

#endif /* __devices_bgq_mu_inj_fifo_message_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
