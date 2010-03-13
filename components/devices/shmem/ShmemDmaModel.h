/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemDmaModel.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemDmaModel_h__
#define __components_devices_shmem_ShmemDmaModel_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "Memregion.h"

#include "sys/xmi.h"

#include "components/devices/DmaInterface.h"
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemDmaMessage.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    ///
    /// \brief Packet model interface implementation for shared memory.
    ///
    /// The shared memory packet model contains the shared memory
    /// packet header which is block-copied into a shared memory message
    /// when it is generated.
    ///
    /// The model also registers the dispatch function with the shared
    /// memory device and stores the dispatch id in the packet header.
    ///
    /// \see ShmemPacketDevice
    ///
    template <class T_Device>
    class ShmemDmaModel : public Interface::DmaModel < ShmemDmaModel<T_Device>, T_Device, sizeof(ShmemDmaMessage<T_Device>) >
    {
      public:
        ///
        /// \brief Construct a Common Device Interface shared memory packet model.
        ///
        /// \param[in] device  Shared memory device
        ///
        ShmemDmaModel (T_Device & device) :
            Interface::DmaModel < ShmemDmaModel<T_Device>, T_Device, sizeof(ShmemDmaMessage<T_Device>) > (device),
            _device (device),
            _context (device.getContext())
        {
          COMPILE_TIME_ASSERT(sizeof(ShmemDmaMessage<T_Device>) == sizeof(ShmemDmaPutMessage<T_Device>));
          COMPILE_TIME_ASSERT(sizeof(ShmemDmaMessage<T_Device>) == sizeof(ShmemDmaGetMessage<T_Device>));
        };

        static const size_t dma_model_state_bytes          = sizeof(ShmemDmaMessage<T_Device>);

        inline bool postDmaPut_impl (uint8_t              state[sizeof(ShmemDmaMessage<T_Device>)],
                                     xmi_event_function   local_fn,
                                     void               * cookie,
                                     xmi_task_t           target_task,
                                     Memregion          * local_memregion,
                                     size_t               local_offset,
                                     Memregion          * remote_memregion,
                                     size_t               remote_offset,
                                     size_t               bytes)
        {
          // This constant-expression branch will be optimized out by the compiler
          if (! Memregion::shared_address_write_supported)
            XMI_abortf("%s<%d>\n", __FILE__, __LINE__);

          // Always use context 0 to determine the fifo for dma operations
          size_t fnum = _device.fnum (_device.task2peer(target_task), 0);

          if (_device.isSendQueueEmpty (fnum))
            {
              size_t sequence = _device.nextInjSequenceId (fnum);
              size_t last_rec_seq_id = _device.lastRecSequenceId (fnum);

              if (sequence - 1 <= last_rec_seq_id) //sequence id is carried by a pt-to-pt message before me
                {
                  local_memregion->write (local_offset,
                                          remote_memregion,
                                          remote_offset,
                                          bytes);

                  if (local_fn) local_fn (_context, cookie, XMI_SUCCESS);

                  return XMI_SUCCESS;
                }
            }

          ShmemDmaPutMessage<T_Device> * obj = (ShmemDmaPutMessage<T_Device> *) & state[0];
          new (obj) ShmemDmaPutMessage<T_Device> (_device.getQS(fnum), local_fn, cookie, _device, fnum,
                                        local_memregion, local_offset,
                                        remote_memregion, remote_offset,
                                        bytes);

          _device.template post<ShmemDmaPutMessage<T_Device> > (fnum, obj);
          return XMI_SUCCESS;
        };

        inline bool postDmaGet_impl (uint8_t              state[sizeof(ShmemDmaMessage<T_Device>)],
                                     xmi_event_function   local_fn,
                                     void               * cookie,
                                     xmi_task_t           target_task,
                                     Memregion          * local_memregion,
                                     size_t               local_offset,
                                     Memregion          * remote_memregion,
                                     size_t               remote_offset,
                                     size_t               bytes)
        {
          // This constant-expression branch will be optimized out by the compiler
          if (! Memregion::shared_address_read_supported)
            XMI_abortf("%s<%d>\n", __FILE__, __LINE__);

          // Always use context 0 to determine the fifo for dma operations
          size_t fnum = _device.fnum (_device.task2peer(target_task), 0);

          if (_device.isSendQueueEmpty (fnum))
            {
#warning FIX THESE NEXT TWO LINES
#if 0
              size_t sequence = _device.nextInjSequenceId (fnum);
              size_t last_rec_seq_id = _device.lastRecSequenceId (fnum);

              if (sequence - 1 <= last_rec_seq_id) //sequence id is carried by a pt-to-pt message before me
#endif
                {
                  local_memregion->read (local_offset,
                                         remote_memregion,
                                         remote_offset,
                                         bytes);

                  if (local_fn) local_fn (_context, cookie, XMI_SUCCESS);

                  return XMI_SUCCESS;
                }
            }

          ShmemDmaGetMessage<T_Device> * obj = (ShmemDmaGetMessage<T_Device> *) & state[0];
          new (obj) ShmemDmaGetMessage<T_Device> (_device.getQS(fnum), local_fn, cookie, &_device, fnum,
                                                  local_memregion, local_offset,
                                                  remote_memregion, remote_offset,
                                                  bytes);

          _device.template post<ShmemDmaGetMessage<T_Device> > (fnum, obj);

          return XMI_SUCCESS;
        };

      protected:

        T_Device      & _device;
        xmi_context_t   _context;

    };  // XMI::Device::ShmemDmaModel class
  };    // XMI::Device namespace
};      // XMI namespace
#undef TRACE_ERR
#endif // __components_devices_shmem_ShmemDmaModel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
