/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemDmaMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemDmaMessage_h__
#define __components_devices_shmem_ShmemDmaMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"

#include <pami.h>

#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemMessage.h"
#include "Memregion.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      template <class T_Device>
      class DmaMessage : public SendQueue::Message
      {
        protected:
          /// invoked by the thread object
          /// \see SendQueue::Message::_work
          static pami_result_t __advance_mr (pami_context_t context, void * cookie)
          {
            DmaMessage * msg = (DmaMessage *) cookie;
            return msg->advance_mr();
          };

          static pami_result_t __advance_va (pami_context_t context, void * cookie)
          {
            DmaMessage * msg = (DmaMessage *) cookie;
            return msg->advance_va();
          };

          inline pami_result_t advance_mr ()
          {
            TRACE_ERR((stderr, ">> DmaMessage<..,%d>::advance_mr()\n", T_Device::shaddr_write_supported));

            // Do not perform the shared address operation until all previous
            // packets from this origin context have been received.
            if (! _device->activePackets(_fnum))
              {
                if (T_Device::shaddr_write_supported)
                  {
                    _device->shaddr.write(_remote_mr, _remote_offset,
                                          _local_mr, _local_offset,
                                          _bytes);
                  }
                else
                  {
                    _device->shaddr.read(_local_mr, _local_offset,
                                         _remote_mr, _remote_offset,
                                         _bytes);
                  }

                TRACE_ERR((stderr, "<< DmaMessage<..,%d>::advance_mr(), return PAMI_SUCCESS\n", T_Device::shaddr_write_supported));
                return PAMI_SUCCESS;
              }

            TRACE_ERR((stderr, "<< DmaMessage<..,%d>::advance_mr(), return PAMI_EAGAIN\n", T_Device::shaddr_write_supported));
            return PAMI_EAGAIN;
          };

          inline pami_result_t advance_va ()
          {
            TRACE_ERR((stderr, ">> DmaMessage<..,%d>::advance_va()\n", T_Device::shaddr_write_supported));

            // Do not perform the shared address operation until all previous
            // packets from this origin context have been received.
            if (! _device->activePackets(_fnum))
              {
                PAMI_abortf("virtual address shaddr operations not currently supported.\n");

                if (T_Device::shaddr_write_supported)
                  {
                    //_device->shaddr.write(...);
                  }
                else
                  {
                    //_device->shaddr.read(...);
                  }

                TRACE_ERR((stderr, "<< DmaMessage<..,%d>::advance_va(), return PAMI_SUCCESS\n", T_Device::shaddr_write_supported));
                return PAMI_SUCCESS;
              }

            TRACE_ERR((stderr, "<< DmaMessage<..,%d>::advance_va(), return PAMI_EAGAIN\n", T_Device::shaddr_write_supported));
            return PAMI_EAGAIN;
          };

        public:
          inline DmaMessage (pami_event_function   fn,
                             void                * cookie,
                             T_Device            * device,
                             size_t                fnum,
                             Memregion           * local_mr,
                             size_t                local_offset,
                             Memregion           * remote_mr,
                             size_t                remote_offset,
                             size_t                bytes) :
              SendQueue::Message (DmaMessage::__advance_mr, this, fn, cookie, device->getContextOffset()),
              _device (device),
              _fnum (fnum),
              _local_mr (local_mr),
              _local_offset (local_offset),
              _remote_mr (remote_mr),
              _remote_offset (remote_offset),
              _bytes (bytes)
          {
            TRACE_ERR((stderr, "<> DmaMessage<..,%d>::DmaMessage()\n", T_Device::shaddr_write_supported));
          };

        protected:

          T_Device  * _device;
          size_t      _fnum;

          Memregion * _local_mr;
          size_t      _local_offset;
          Memregion * _remote_mr;
          size_t      _remote_offset;

          size_t      _bytes;
      };  // PAMI::Device::Shmem::DmaMessage class
    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace
#undef TRACE_ERR
#endif // __components_devices_shmem_ShmemDmaMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
