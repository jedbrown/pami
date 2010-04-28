/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/msg/InjFifoMessage.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_msg_InjFifoMessage_h__
#define __components_devices_bgq_mu_msg_InjFifoMessage_h__

#include <pami.h>
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
      class InjFifoMessage : public PAMI::Queue::Element
      {
        public:
          inline InjFifoMessage (uint64_t sequenceNum = 0) :
              PAMI::Queue::Element (),
              _desc (),
              _wrapper (&_desc)
          {
            _wrapper.setSequenceNumber (sequenceNum);
          }

          inline InjFifoMessage (pami_event_function   function,
                                 void                * cookie,
                                 pami_context_t        context,
                                 uint64_t              sequenceNum = 0) :
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

          inline bool advance (MUSPI_InjFifo_t    * injfifo,
                               MUHWI_Descriptor_t * hwi_desc,
                               void               * payloadVa,
                               void               * payloadPa)
          {
            fprintf (stderr, "InjFifoMessage::advance() >>\n");
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            // The descriptor should be fully constructed at this point and all
            // that remains is to copy it into the injection fifo and advance
            // the injection fifo tail.
            fprintf (stderr, "InjFifoMessage::advance() .. before clone\n");
            _desc.clone (*desc);

            // Advance the injection fifo tail.
            fprintf (stderr, "InjFifoMessage::advance() .. before MUSPI_InjFifoAdvanceDesc()\n");
            _wrapper.setSequenceNumber (MUSPI_InjFifoAdvanceDesc (injfifo));
            fprintf (stderr, "InjFifoMessage::advance() ..  after MUSPI_InjFifoAdvanceDesc()\n");

            fprintf (stderr, "InjFifoMessage::advance() <<\n");
            return true;
          }

        protected:

          MUSPI_DescriptorBase    _desc __attribute__((__aligned__(16)));
          MUDescriptorWrapper     _wrapper __attribute__((__aligned__(16)));

      }; // PAMI::Device::MU::InjFifoMessage class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

//#undef TRACE

#endif /* __components_devices_bgq_mu_msg_InjFifoMessage_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
