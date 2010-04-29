/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/msg/ShortInjFifoMessage.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_msg_ShortInjFifoMessage_h__
#define __components_devices_bgq_mu_msg_ShortInjFifoMessage_h__

#include <pami.h>
#include "components/devices/bgq/mu/msg/InjFifoMessage.h"

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
      class ShortInjFifoMessage : public InjFifoMessage
      {
        public:
          inline ShortInjFifoMessage (uint64_t sequenceNum = 0) :
              InjFifoMessage (sequenceNum)
          {
            _wrapper.setSequenceNumber (sequenceNum);
          }

          inline ShortInjFifoMessage (pami_event_function   function,
                                      void                * cookie,
                                      pami_context_t        context,
                                      uint64_t              sequenceNum = 0) :
              InjFifoMessage (function, cookie, context, sequenceNum)
          {
          }

          inline ~ShortInjFifoMessage () {};

          inline void * getPayload ()
          {
            return (void *) _payload;
          }

          inline size_t copyPayload (void * src, size_t bytes)
          {
            memcpy ((void *)_payload, src, bytes);
            return bytes;
          }

          inline size_t copyPayload (struct iovec * iov, size_t niov)
          {
                // Pack the source buffer(s) into the payload buffer, copy the
                // descriptor, set the payload physical address, and advance
                // the injection fifo tail.
                unsigned i;
                uint64_t bytes = 0;
                uint8_t * ptr = (uint8_t *) _payload;

                for (i = 0; i < niov; i++)
                  {
                    memcpy (ptr, iov[i].iov_base, iov[i].iov_len);
                    bytes += iov[i].iov_len;
                    ptr += iov[i].iov_len;
                  }

                return bytes;
          }


        protected:

          uint64_t _payload[64] __attribute__((__aligned__(16)));

      }; // PAMI::Device::MU::ShortInjFifoMessage class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

//#undef TRACE

#endif /* __components_devices_bgq_mu_msg_ShortInjFifoMessage_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
