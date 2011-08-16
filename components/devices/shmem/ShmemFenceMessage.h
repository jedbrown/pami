/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemFenceMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemFenceMessage_h__
#define __components_devices_shmem_ShmemFenceMessage_h__

#include <errno.h>
#include <pami.h>

#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemMessage.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      template < class T_Device>
      class FenceMessage : public SendQueue::Message
      {
        protected:

          /// invoked by the thread object
          /// \see SendQueue::Message::_work
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            FenceMessage * msg = (FenceMessage *) cookie;
            return msg->advance();
          };

          inline pami_result_t advance ()
          {
            // Block until all previous packets from this origin context have
            // been received.
            if (_device->activePackets(_fnum))
              return PAMI_EAGAIN;


            // This removes the work from the generic device.
            this->setStatus (PAMI::Device::Done);

            // This causes the message completion callback to be invoked.
            return PAMI_SUCCESS;
          };

        public:

          inline FenceMessage (pami_event_function   fn,
                               void                * cookie,
                               T_Device            * device,
                               size_t                fnum) :
              SendQueue::Message (FenceMessage::__advance, this, fn, cookie, device->getContextOffset()),
              _device (device),
              _fnum (fnum)
          {};

        protected:

          T_Device            * _device;
          size_t                _fnum;
          pami_event_function   _user_fn;
          void                * _user_cookie;

      };  // PAMI::Device::Shmem::FenceMessage class
    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemFenceMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
