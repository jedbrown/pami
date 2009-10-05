/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file devices/prod/bgq/mu/MUInjFifoMessage.h
 * \brief ???
 */
#ifndef __devices_bgq_mu_inj_fifo_message_h__
#define __devices_bgq_mu_inj_fifo_message_h__

#include <sys/uio.h>

#include "sys/xmi.h"
#include "util/queue/Queue.h"

//#ifdef TRACE
//#error TRACE already defined!
//#else
//#define TRACE(x) fprintf x
//#endif

namespace XMI
{
  namespace Device
  {
    namespace MU
    {
      class MUInjFifoMessage : public QueueElem
      {
        public:
          inline MUInjFifoMessage (xmi_callback_t & cb, uint64_t sequenceNum = 0) :
              QueueElem (),
              _desc (),
              _wrapper (&_desc)
          {
            _wrapper.setDoneCallback ((void (*)(void*))cb.function, cb.clientdata);
            _wrapper.setSequenceNumber (sequenceNum);
          }

          inline MUSPI_DescriptorBase * getDescriptor ()
          {
            return &_desc;
          }

          inline MUSPI_DescriptorWrapper * getWrapper ()
          {
            return &_wrapper;
          }

          inline bool isCallbackDesired ()
          {
            return _wrapper.isCallbackDesired ();
          }

          inline void setSourceBuffer (void            * payload,
                                       size_t            bytes)
          {
            __iov[0].iov_base = payload;
            __iov[0].iov_len  = bytes;

            _iov  = &__iov[0];
            _niov = 1;
          }

          inline void setSourceBuffer (void            * payload0,
                                       size_t            bytes0,
                                       void            * payload1,
                                       size_t            bytes1)
          {
            __iov[0].iov_base = payload0;
            __iov[0].iov_len  = bytes0;
            __iov[1].iov_base = payload1;
            __iov[1].iov_len  = bytes1;

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
          MUSPI_DescriptorWrapper _wrapper __attribute__((__aligned__(16)));
          struct iovec   __iov[2];
          struct iovec * _iov;
          size_t         _niov;

      }; // XMI::Device::MU::MUInjFifoMessage class
    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace

//#undef TRACE

#endif /* __devices_bgq_mu_inj_fifo_message_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
