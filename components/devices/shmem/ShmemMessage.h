/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemMessage_h__
#define __components_devices_shmem_ShmemMessage_h__

#include <sys/uio.h>

#include "sys/xmi.h"

#include "util/common.h"
#include "util/queue/Queue.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/Message.h"

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    namespace Shmem
    {
      // GenericDeviceMessageQueue defined in GenericDevicePlatform.h
      class SendQueue : public GenericDeviceMessageQueue
      {
        public:

          class Message : public XMI::Device::Generic::GenericMessage
          {
            protected:
              inline Message (xmi_work_function    work_func,
                              void               * work_cookie,
                              xmi_event_function   done_fn,
                              void               * done_cookie,
                              size_t               contextid) :
                  XMI::Device::Generic::GenericMessage(NULL, (xmi_callback_t) {done_fn, done_cookie}, 0, contextid),
              _work (work_func, work_cookie),
              _genericdevice (NULL)
              {
                TRACE_ERR((stderr, "<> SendQueue::Message::Message()\n"));
                _work.setStatus (Ready);
              };

            public:

              virtual ~Message () {};

              /// \note This is required to make "C" programs link successfully with virtual destructors
              inline void operator delete (void * p)
              {
                XMI_abortf("%s<%d>\n", __FILE__, __LINE__);
              }

              void setup (XMI::Device::Generic::Device * device, SendQueue * sendQ)
              {
                TRACE_ERR((stderr, ">> SendQueue::Message::setup(%p, %p)\n", device, sendQ));
                _genericdevice = device;
                this->_QS = sendQ;
                TRACE_ERR((stderr, "<< SendQueue::Message::setup(%p, %p)\n", device, sendQ));
              }

              ///
              /// \brief virtual function implementation
              /// \see XMI::Device::Generic::GenericMessage::postNext()
              ///
              /// Post this message to the appropriate generic device, this is the
              /// completion message and the thread (work) message(s).
              ///
              /// \todo Figure out the input parameters and the return value
              ///
              inline xmi_context_t postNext (bool something)
              {
                TRACE_ERR((stderr, ">> SendQueue::Message::postNext(%d)\n", something));
                XMI_assert_debug (_genericdevice != NULL);

                _genericdevice->postMsg ((XMI::Device::Generic::GenericMessage *) this);
                _genericdevice->postThread ((XMI::Device::Generic::GenericThread *) &_work);

                TRACE_ERR((stderr, "<< SendQueue::Message::postNext(%d), return NULL\n", something));
                return NULL; // what should this be?
              };

            protected:

              XMI::Device::Generic::GenericThread _work;
              XMI::Device::Generic::Device * _genericdevice;
          };

          inline SendQueue (XMI::Device::Generic::Device * progress, size_t local) :
              GenericDeviceMessageQueue (),
              _progress (progress),
              _local_progress_device (XMI::Device::Generic::Device::Factory::getDevice(progress, 0, local))
          {
          };

          /// \brief post the message to be advanced later
          ///
          /// First, post to the "secondary" queue (owned by the shmem device),
          /// then, if the secondary queue was empty, post to the "primary"
          /// queue (owned by the generic device)
          inline void post (SendQueue::Message * msg)
          {
            TRACE_ERR((stderr, ">> SendQueue::post(%p)\n", msg));
            this->enqueue (msg);
            TRACE_ERR((stderr, "<< SendQueue::post(%p)\n", msg));
          };

          /// \brief virtual function implementation
          /// \see XMI::Device::Generic::GenericMessage::postNext()
          ///
          /// Post this message to the appropriate generic device, this is the
          /// completion message and the thread (work) message(s).
          inline bool postNext (SendQueue::Message * msg)
          {
            TRACE_ERR((stderr, ">> SendQueue::postNext(%p)\n", msg));
            _local_progress_device.postMsg((XMI::Device::Generic::GenericMessage *) msg);
            TRACE_ERR((stderr, "<< SendQueue::postNext(%p), return true\n", msg));
            return true; // huh?
          };

        private:
          XMI::Device::Generic::Device * _progress;
          XMI::Device::Generic::Device & _local_progress_device;
      };
    };
  };
};
#undef TRACE_ERR
#endif // __components_devices_shmem_ShmemMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
