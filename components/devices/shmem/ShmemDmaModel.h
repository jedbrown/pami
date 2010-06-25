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

#include <pami.h>

#include "components/devices/DmaInterface.h"
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemDmaMessage.h"
#include "components/devices/shmem/shaddr/ShaddrInterface.h"
#include "components/devices/shmem/ShmemWork.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      ///
      /// \brief Dma model interface implementation for shared memory.
      ///
      template <class T_Device, bool T_Ordered = false>
      class DmaModel : public Interface::DmaModel < DmaModel<T_Device,T_Ordered>,
                                                    T_Device,
                                                    sizeof(DmaMessage<T_Device,T_Ordered>) >
      {
        public:
          ///
          /// \brief Construct a shared memory dma model.
          ///
          /// \param[in] device  Shared memory device
          ///
          DmaModel (T_Device & device, pami_result_t & status) :
              Interface::DmaModel < DmaModel<T_Device,T_Ordered>,
                                    T_Device,
                                    sizeof(DmaMessage<T_Device,T_Ordered>) >
                                  (device, status),
              _device (device),
              _context (device.getContext())
          {
            COMPILE_TIME_ASSERT(sizeof(DmaMessage<T_Device,T_Ordered>) == sizeof(PutDmaMessage<T_Device,T_Ordered>));
            COMPILE_TIME_ASSERT(sizeof(DmaMessage<T_Device,T_Ordered>) == sizeof(GetDmaMessage<T_Device,T_Ordered>));
#if 1
            status = PAMI_ERROR;
            if (T_Device::shaddr_mr_supported &&
                (T_Device::shaddr_read_supported ||
                 T_Device::shaddr_write_supported) &&
                device.shaddr.isEnabled())
              {
                status = PAMI_SUCCESS;
              }
#endif
            return;
          };

          static const size_t dma_model_state_bytes  = sizeof(DmaMessage<T_Device,T_Ordered>);
          static const size_t dma_model_va_supported = T_Device::shaddr_va_supported;
          static const size_t dma_model_mr_supported = T_Device::shaddr_mr_supported;

          inline bool postDmaPut_impl (size_t   task,
                                       size_t   bytes,
                                       void   * local,
                                       void   * remote)
          {
#if 0
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shaddr_write_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            if (T_Ordered == false)
            {
              _device.shaddr.write (remote, local, bytes, task);
              return true;
            }
#else
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
#endif
          };

          inline bool postDmaPut_impl (uint8_t               (&state)[sizeof(PutDmaMessage<T_Device,T_Ordered>)],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote)
          {
#if 0
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shaddr_write_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            if (T_Ordered == false)
            {
              _device.shaddr.write (remote, local, bytes, task);
              if (local_fn) local_fn (_context, cookie, PAMI_SUCCESS);
              return true;
            }

            //
            // Check to see if there are any active or pending messages to
            // the destination, if so add to a "pending dma" queue.
            //
            // Otherwise, simply do the shared address write.
            if (something)
            {
              // Construct a put message and post it to the queue
              PutDmaMessage<T_Device,T_Shaddr,T_Ordered> * msg =
                (PutDmaMessage<T_Device,T_Shaddr,T_Ordered> *) & state[0];
              new (msg) PutDmaMessage<T_Device,T_Shaddr,T_Ordered> (_device.getQS(0), local_fn, cookie, _device, 0,
                                                 local, remote, bytes);

              _device.post(0, msg);
              return false
            }
            else
            {
              T_Shaddr::write (task, local, remote, bytes);
              if (local_fn) local_fn (_context, cookie, PAMI_SUCCESS);
              return true;
            }

            // Should never get here ...
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
#else
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
#endif
          };

          inline bool postDmaPut_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset)
          {
            TRACE_ERR((stderr, ">> postDmaPut_impl():%d\n", __LINE__));
            if (T_Device::shaddr_write_supported)
            {
              size_t fnum = _device.fnum (_device.task2peer(target_task), target_offset);
              if (T_Ordered == false)
              {
                TRACE_ERR((stderr, "   postDmaPut_impl():%d\n", __LINE__));
                _device.shaddr.write (remote_memregion, remote_offset, local_memregion, local_offset, bytes);
                TRACE_ERR((stderr, "<< postDmaPut_impl():%d .. return true\n", __LINE__));
                return true;
              }
              else if ((_device.isSendQueueEmpty (fnum)) &&
                       (_device.activePackets(fnum) == false))
              {
                TRACE_ERR((stderr, "   postDmaPut_impl():%d\n", __LINE__));
                _device.shaddr.write (remote_memregion, remote_offset, local_memregion, local_offset, bytes);
                TRACE_ERR((stderr, "<< postDmaPut_impl():%d .. return true\n", __LINE__));
                return true;
              }
              else
              {
                // Ordered writes are required but either the send queue
                // has pending messages or the fifo has active packets
                // so the write must wait. Since this is a not-non-blocking
                // interface it must return false (put not accomplished).
              }
            }
            else
            {
              // Need to perform a "reverse get", a.k.a. "rendezvous" transfer.
              // The origin task must wait for the target task to complete the
              // transfer, which means it must be non-blocking, which means
              // that this method must return false (put not accomplished).
            }

            TRACE_ERR((stderr, "<< postDmaPut_impl():%d .. return false\n", __LINE__));
            return false;
          };

          inline bool postDmaPut_impl (uint8_t               state[sizeof(DmaMessage<T_Device>)],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset)
          {
            TRACE_ERR((stderr, ">> postDmaPut_impl():%d\n", __LINE__));

            if (! postDmaPut_impl (target_task, target_offset, bytes,
                                   local_memregion, local_offset,
                                   remote_memregion, remote_offset))
            {
              if (! T_Device::shaddr_write_supported)
              {
                // Attempt to inject a "request to reverse get" packet into the
                // fifo. The target task will receive this packet, perform a
                // shared address read, then "consume" the packet from the
                // fifo. At this point the "last reception sequence identifier"
                // will be less than or equal to the sequence identifier of the
                // "request to reverse get" packet, the put operation will be
                // complete, and the origin task will invoke the completion
                // callback.
                size_t fnum = _device.fnum (_device.task2peer(target_task), target_offset);
                if (_device.isSendQueueEmpty (fnum))
                {
                  // It is safe to place this object on the stack because the
                  // object is no longer needed after the "write single packet"
                  // returns true.  If the packet could not be written the
                  // object will be copied into a pending send message.
                  typename T_Device::SystemShaddrInfo info(local_memregion, remote_memregion, local_offset, remote_offset, bytes);

                  COMPILE_TIME_ASSERT(sizeof(typename T_Device::SystemShaddrInfo) <= T_Device::payload_size);

                  size_t sequence = (size_t)-1;
                  if (_device.writeSinglePacket (fnum, T_Device::system_ro_put_dispatch,
                                                 NULL, 0, (void *)&info,
                                                 sizeof(typename T_Device::SystemShaddrInfo),
                                                 sequence) == PAMI_SUCCESS)
                  {
                    if (likely(local_fn != NULL))
                    {
                      // Create a "completion message" on the done queue and wait
                      // until the target task has completed the put operation.
                      RecPacketWork<T_Device> * work = (RecPacketWork<T_Device> *) state;
                      new (work) RecPacketWork<T_Device> (&_device, sequence, fnum, local_fn, cookie);
                      _device.post (work);

                      TRACE_ERR((stderr, "<< postDmaPut_impl():%d .. return false\n", __LINE__));
                      return false;
                    }

                    // The put operation is considered complete because the
                    // "request to reverse get" packet was successfully written
                    // into the fifo and no completion function was specified.
                    TRACE_ERR((stderr, "<< postDmaPut_impl():%d .. return true\n", __LINE__));
                    return false;
                  }
                }
              }

              // The following code handles several cases:
              // 1. read-write, ordered, and non-empty send queue
              // 1. read-write, ordered, and active packets in the fifo
              // 2. read-only, non-empty send queue
              // 3. read-only, full fifo (unable to inject put packet)
              size_t fnum = _device.fnum (_device.task2peer(target_task), target_offset);
              typename T_Device::SystemShaddrInfo info(local_memregion, remote_memregion, local_offset, remote_offset, bytes);
              PutDmaMessage<T_Device,T_Ordered> * msg =
                (PutDmaMessage<T_Device,T_Ordered> *) state;
              new (msg) PutDmaMessage<T_Device,T_Ordered> (local_fn, cookie,
                                                           &_device, fnum, bytes,
                                                           local_memregion,
                                                           remote_memregion,
                                                           local_offset,
                                                           remote_offset,
                                                           (void *)&info, sizeof(typename T_Device::SystemShaddrInfo));
              _device.post(fnum, msg);
              TRACE_ERR((stderr, "<< postDmaPut_impl():%d .. return false\n", __LINE__));
              return false;
            }
            else
            {
              if (likely(local_fn != NULL)) local_fn (_context, cookie, PAMI_SUCCESS);
              TRACE_ERR((stderr, "<< postDmaPut_impl():%d .. return true\n", __LINE__));
              return true;
            }

            // Should never get here.
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
          };

          inline bool postDmaGet_impl (size_t   target_task,
                                       size_t   target_offset,
                                       size_t   bytes,
                                       void   * local,
                                       void   * remote)
          {
#if 0
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shaddr_read_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            if (T_Ordered == false)
            {
              _device.shaddr.read (local, remote, bytes, task);
              return true;
            }

            // Should never get here ...
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
#else
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
#endif
          };

          inline bool postDmaGet_impl (uint8_t               (&state)[sizeof(GetDmaMessage<T_Device,T_Ordered>)],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote)
          {
#if 0
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shaddr_read_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            if (T_Ordered == false)
            {
              _device.shaddr.read (local, remote, bytes, task);
              if (local_fn) local_fn (_context, cookie, PAMI_SUCCESS);
              return true;
            }

            //
            // Check to see if there are any active or pending messages to
            // the destination, if so add to a "pending dma" queue.
            //
            // Otherwise, simply do the shared address write.
            if (something)
            {
              // Construct a put message and post it to the queue
              GetDmaMessage<T_Device,T_Shaddr,T_Ordered> * msg =
                (GetDmaMessage<T_Device,T_Shaddr,T_Ordered> *) & state[0];
              new (msg) GetDmaMessage<T_Device,T_Shaddr,T_Ordered> (_device.getQS(0), local_fn, cookie, _device, 0,
                                                 local, remote, bytes);

              _device.post(0, msg);
              return false
            }
            else
            {
              T_Shaddr::read (local, remote, bytes, task);
              if (local_fn) local_fn (_context, cookie, PAMI_SUCCESS);
              return true;
            }

            // Should never get here ...
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
#else
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
#endif
          };

          inline bool postDmaGet_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset)
          {
            TRACE_ERR((stderr, ">> Shmem::DmaModel<T_Ordered=%d>::postDmaGet_impl('memregion')\n", T_Ordered));

            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shaddr_read_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            if (T_Ordered == false)
            {
              TRACE_ERR((stderr, "   Shmem::DmaModel<T_Ordered=%d>::postDmaGet_impl('memregion'), do an 'unordered' shared address read.\n", T_Ordered));
              _device.shaddr.read (local_memregion, local_offset, remote_memregion, remote_offset, bytes);
              TRACE_ERR((stderr, "<< Shmem::DmaModel::postDmaGet_impl('non-blocking memregion'), return true\n"));
              return true;
            }
            else
            {
              TRACE_ERR((stderr, "   Shmem::DmaModel<T_Ordered=%d>::postDmaGet_impl('memregion'), do an 'ordered' shared address read.\n", T_Ordered));
#if 0
              if (must_be queued)
              {
                return false;
              }
              else
#endif
              {
              _device.shaddr.read (local_memregion, local_offset, remote_memregion, remote_offset, bytes);
              TRACE_ERR((stderr, "<< Shmem::DmaModel::postDmaGet_impl('memregion'), return true\n"));
              return true;
              }
            }
            TRACE_ERR((stderr, "<< Shmem::DmaModel::postDmaGet_impl('memregion'), return false\n"));
            return false;

#if 0
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shared_address_read_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

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

                    if (local_fn) local_fn (_context, cookie, PAMI_SUCCESS);

                    return PAMI_SUCCESS;
                  }
              }

            ShmemGetDmaMessage<T_Device> * obj = (ShmemGetDmaMessage<T_Device> *) & state[0];
            new (obj) ShmemGetDmaMessage<T_Device> (_device.getQS(fnum), local_fn, cookie, &_device, fnum,
                                                    local_memregion, local_offset,
                                                    remote_memregion, remote_offset,
                                                    bytes);

            _device.template post<ShmemGetDmaMessage<T_Device> > (fnum, obj);

            return PAMI_SUCCESS;
#endif
          };

          inline bool postDmaGet_impl (uint8_t               state[sizeof(DmaMessage<T_Device>)],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset)
          {
            TRACE_ERR((stderr, ">> Shmem::DmaModel<T_Ordered=%d>::postDmaGet_impl('non-blocking memregion')\n", T_Ordered));

            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shaddr_read_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            if (T_Ordered == false)
            {
              TRACE_ERR((stderr, "   Shmem::DmaModel<T_Ordered=%d>::postDmaGet_impl('non-blocking memregion'), do an 'unordered' shared address read.\n", T_Ordered));
              _device.shaddr.read (local_memregion, local_offset, remote_memregion, remote_offset, bytes);
              TRACE_ERR((stderr, "<< Shmem::DmaModel::postDmaGet_impl('non-blocking memregion'), return true\n"));
              return true;
            }
            else
            {
              TRACE_ERR((stderr, "   Shmem::DmaModel<T_Ordered=%d>::postDmaGet_impl('non-blocking memregion'), do an 'ordered' shared address read.\n", T_Ordered));

              size_t fnum = _device.fnum (_device.task2peer(target_task), target_offset);
              if (_device.activePackets(fnum))
              {
                // Construct a get message and post it to the queue
                GetDmaMessage<T_Device,T_Ordered> * msg =
                  (GetDmaMessage<T_Device,T_Ordered> *) & state[0];
                new (msg) GetDmaMessage<T_Device,T_Ordered> (local_fn, cookie, &_device, fnum, bytes,
                                                 local_memregion, remote_memregion, local_offset, remote_offset);

                _device.post(fnum, msg);
                return false;
              }
              else
              {
                _device.shaddr.read (local_memregion, local_offset, remote_memregion, remote_offset, bytes);
                TRACE_ERR((stderr, "<< Shmem::DmaModel::postDmaGet_impl('non-blocking memregion'), return true\n"));
                return true;
              }
            }
            TRACE_ERR((stderr, "<< Shmem::DmaModel::postDmaGet_impl('non-blocking memregion'), return false\n"));
            return false;

#if 0
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shared_address_read_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

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

                    if (local_fn) local_fn (_context, cookie, PAMI_SUCCESS);

                    return PAMI_SUCCESS;
                  }
              }

            ShmemGetDmaMessage<T_Device> * obj = (ShmemGetDmaMessage<T_Device> *) & state[0];
            new (obj) ShmemGetDmaMessage<T_Device> (_device.getQS(fnum), local_fn, cookie, &_device, fnum,
                                                    local_memregion, local_offset,
                                                    remote_memregion, remote_offset,
                                                    bytes);

            _device.template post<ShmemGetDmaMessage<T_Device> > (fnum, obj);

            return PAMI_SUCCESS;
#endif
          };

        protected:

          T_Device       & _device;
          pami_context_t   _context;

      };  // PAMI::Device::Shmem::DmaModel class
    };    // PAMI::Device::Shmem namespace
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif // __components_devices_shmem_ShmemDmaModel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
