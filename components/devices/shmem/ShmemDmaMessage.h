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
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      template <class T_Device, bool T_Ordered = false>
      class DmaMessage : public SendQueue::Message
      {
        protected:

          inline DmaMessage (pami_work_function    work_func,
                             void                * work_cookie,
                             pami_event_function   fn,
                             void                * cookie,
                             T_Device            * device,
                             size_t                task,
                             size_t                bytes,
                             void                * local,
                             void                * remote) :
              SendQueue::Message (work_func, work_cookie, fn, cookie, device->getContextOffset()),
              _device (device),
              _task (task),
              _bytes (bytes),
              _local_va (local),
              _remote_va (remote)
          {};

          inline DmaMessage (pami_work_function    work_func,
                             void                * work_cookie,
                             pami_event_function   fn,
                             void                * cookie,
                             T_Device            * device,
                             size_t                fnum,
                             size_t                bytes,
                             Memregion           * local,
                             Memregion           * remote,
                             size_t                local_offset,
                             size_t                remote_offset) :
              SendQueue::Message (work_func, work_cookie, fn, cookie, device->getContextOffset()),
              _device (device),
              _fnum (fnum),
              _bytes (bytes),
              _local_mr (local),
              _remote_mr (remote),
              _local_offset (local_offset),
              _remote_offset (remote_offset)
          {};

          inline DmaMessage (pami_work_function    work_func,
                             void                * work_cookie,
                             pami_event_function   fn,
                             void                * cookie,
                             T_Device            * device,
                             size_t                fnum,
                             size_t                bytes,
                             Memregion           * local,
                             Memregion           * remote,
                             size_t                local_offset,
                             size_t                remote_offset,
                             void                * payload_data,
                             size_t                payload_length) :
              SendQueue::Message (work_func, work_cookie, fn, cookie, device->getContextOffset()),
              _device (device),
              _fnum (fnum),
              _bytes (bytes),
              _local_mr (local),
              _remote_mr (remote),
              _local_offset (local_offset),
              _remote_offset (remote_offset),
              _packet_injected (false),
              _length (payload_length)
          {
            memcpy ((void *)_data, payload_data, payload_length);
          };

          T_Device  * _device;
          size_t      _task;
          size_t      _fnum;
          size_t      _bytes;
          void      * _local_va;
          void      * _remote_va;

          Memregion * _local_mr;
          Memregion * _remote_mr;
          size_t      _local_offset;
          size_t      _remote_offset;

          // Used only by the "memory region read only put message"
          pami_event_function   _done_fn;
          void                * _done_cookie;
          bool                  _packet_injected;
          size_t                _sequence;
          size_t                _length;
          uint8_t               _data[T_Device::payload_size];

          //PAMI::Device::Generic::GenericThread _done; // used for "done completion" queue
      };  // PAMI::Device::Shmem::DmaMessage class


      template <class T_Device, bool T_Ordered = false>
      class PutDmaMessage : public DmaMessage<T_Device, T_Ordered>
      {
        protected:

          ///
          /// \note This static function is invoked by the thread object
          /// \see SendQueue::Message::_work
          ///
          static pami_result_t __advance_va (pami_context_t context, void * cookie)
          {
            PutDmaMessage * msg = (PutDmaMessage *) cookie;
            return msg->advance_va();
          };

          inline pami_result_t advance_va ()
          {
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shared_address_write_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            TRACE_ERR((stderr, ">> PutDmaMessage::advance_va()\n"));

            if (T_Ordered == false)
              {
                this->_device->shaddr.write (this->_remote_va, this->_local_va, this->_bytes, this->_task);
                this->setStatus (PAMI::Device::Done);
                return PAMI_SUCCESS;
              }
            else
              {
                PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
              }

            TRACE_ERR((stderr, "<< PutDmaMessage::advance_va(), return PAMI_EAGAIN\n"));
            return PAMI_EAGAIN;
          }

          ///
          /// \note This static function is invoked by the thread object
          /// \see SendQueue::Message::_work
          ///
          static pami_result_t __advance_mr (pami_context_t context, void * cookie)
          {
            pami_result_t result;
            PutDmaMessage * msg = (PutDmaMessage *) cookie;

            // This constant-expression branch will be optimized out by the compiler
            if (T_Device::shaddr_write_supported)
              result = msg->advance_mr_readwrite();
            else
              result = msg->advance_mr_readonly(context);

            return result;
          };

          inline pami_result_t advance_mr_readwrite ()
          {
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shaddr_write_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            TRACE_ERR((stderr, ">> PutDmaMessage::advance_mr_readwrite()\n"));

            if (T_Ordered == false || this->_device->activePackets(this->_fnum) == false)
              {
                this->_device->shaddr.write (this->_remote_mr, this->_remote_offset,
                                             this->_local_mr, this->_local_offset,
                                             this->_bytes);
                this->setStatus (PAMI::Device::Done);

                TRACE_ERR((stderr, "<< PutDmaMessage::advance_mr_readwrite(), return PAMI_SUCCESS\n"));
                return PAMI_SUCCESS;
              }

            TRACE_ERR((stderr, "<< PutDmaMessage::advance_mr_readwrite(), return PAMI_EAGAIN\n"));
            return PAMI_EAGAIN;
          }

          inline pami_result_t advance_mr_readonly (pami_context_t context)
          {
            TRACE_ERR((stderr, ">> PutDmaMessage::advance_mr_readonly()\n"));
            
            if (this->_packet_injected == false)
            {
              if (this->_device->writeSinglePacket (this->_fnum, T_Device::system_ro_put_dispatch,
                                                    NULL, 0, (void *)this->_data,
                                                    this->_length, this->_sequence) == PAMI_SUCCESS)
              {
                this->_packet_injected = true;

                // Remove the message from the head of the "pending send" queue
                // This will invoke the completion callback!
                this->setStatus (PAMI::Device::Done);
              }
            }
            else if (this->_sequence <= this->_device->lastRecSequenceId(this->_fnum))
            {
              // system ro put packet has been received and completed by the
              // target task. This put message in done.
              
              // Invoke the completion callback for the put operation
              if (this->_done_fn)
                this->_done_fn (context, this->_done_cookie, PAMI_SUCCESS);
              
              // return 'success' which will remove the work object from the work queue.
              TRACE_ERR((stderr, "<< PutDmaMessage::advance_mr_readonly(), return PAMI_SUCCESS\n"));
              return PAMI_SUCCESS;
            }

            // Remain on the work queue
            TRACE_ERR((stderr, "<< PutDmaMessage::advance_mr_readonly(), return PAMI_EAGAIN\n"));
            return PAMI_EAGAIN;
          }
#if 0
          static pami_result_t __advance_pending_done (pami_context_t context, void * cookie)
          {
            PutDmaMessage * msg = (PutDmaMessage *) cookie;
            return msg->advance_pending_done();
          };

          inline pami_result_t advance_pending_done ()
          {
            TRACE_ERR((stderr, ">> PutDmaMessage::advance_pending_done()\n"));
#if 0
            size_t last_sequence_id = this->_device->lastRecSequenceId (this->_fnum);
            if (last_sequence_id < this->_sequence)
            {
              // The remote task has not processed the packet with the sequence
              // number that this message is watching. Return PAMI_EAGAIN to
              // remain on the work queue.
              TRACE_ERR((stderr, "<< PutDmaMessage::advance_pending_done(), return PAMI_EAGAIN\n"));
              return PAMI_EAGAIN;
            }
#endif
            // This (entire) message is done. Set the status to "done" to
            // remove the "completion message" from the generic message queue
            // and invoke the completion callback function and return
            // PAMI_SUCESS to remove this "done work" from the generic work
            // queue.
            this->setStatus (PAMI::Device::Done);

            TRACE_ERR((stderr, "<< PutDmaMessage::advance_pending_done(), return PAMI_SUCCESS\n"));
            return PAMI_SUCCESS;
          };
#endif
        public:

          inline PutDmaMessage (pami_event_function   fn,
                                void                * cookie,
                                T_Device            * device,
                                size_t                task,
                                size_t                bytes,
                                void                * local,
                                void                * remote) :
              DmaMessage<T_Device, T_Ordered> (PutDmaMessage::__advance_va,
                                              (void *)this, fn, cookie,
                                              device, task, bytes, local, remote)
          {};

          inline PutDmaMessage (pami_event_function   fn,
                                void                * cookie,
                                T_Device            * device,
                                size_t                task,
                                size_t                bytes,
                                Memregion           * local,
                                Memregion           * remote,
                                size_t                local_offset,
                                size_t                remote_offset) :
              DmaMessage<T_Device, T_Ordered> (PutDmaMessage::__advance_mr, (void *)this,
                                               fn, cookie,
                                               device, task, bytes, local, remote,
                                               local_offset, remote_offset)
          {};

          /// read-only put message .. sends a system message which causes the
          /// remote task to complete the put operation.
          inline PutDmaMessage (pami_event_function   fn,
                                void                * cookie,
                                T_Device            * device,
                                size_t                task,
                                size_t                bytes,
                                Memregion           * local,
                                Memregion           * remote,
                                size_t                local_offset,
                                size_t                remote_offset,
                                void                * payload_data,
                                size_t                payload_length) :
              DmaMessage<T_Device, T_Ordered> (PutDmaMessage::__advance_mr, (void *)this,
                                               //fn, cookie,
                                               NULL, NULL,
                                               device, task, bytes, local, remote,
                                               local_offset, remote_offset,
                                               payload_data, payload_length)
          {
            this->_done_fn      = fn;
            this->_done_cookie  = cookie;
          };

      };  // PAMI::Device::Shmem::PutDmaMessage class


      template <class T_Device, bool T_Ordered = false>
      class GetDmaMessage : public DmaMessage<T_Device, T_Ordered>
      {
        protected:

          ///
          /// \note This static function is invoked by the thread object
          /// \see SendQueue::Message::_work
          ///
          static pami_result_t __advance_va (pami_context_t context, void * cookie)
          {
            GetDmaMessage * msg = (GetDmaMessage *) cookie;
            return msg->advance_va();
          };

          inline pami_result_t advance_va ()
          {
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shared_address_read_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            TRACE_ERR((stderr, ">> GetDmaMessage::advance_va()\n"));

            if (T_Ordered == false)
              {
                this->_device->shaddr.read (this->_local_va, this->_remote_va,
                                this->_bytes, this->_task);
                this->setStatus (PAMI::Device::Done);
                return PAMI_SUCCESS;
              }
            else
              {
                PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
              }

            TRACE_ERR((stderr, "<< GetDmaMessage::advance_va(), return PAMI_EAGAIN\n"));
            return PAMI_EAGAIN;
          }

          ///
          /// \note This static function is invoked by the thread object
          /// \see SendQueue::Message::_work
          ///
          static pami_result_t __advance_mr (pami_context_t context, void * cookie)
          {
            GetDmaMessage * msg = (GetDmaMessage *) cookie;
            return msg->advance_mr();
          };

          inline pami_result_t advance_mr ()
          {
            // This constant-expression branch will be optimized out by the compiler
            if (! T_Device::shaddr_read_supported)
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

            TRACE_ERR((stderr, ">> GetDmaMessage::advance_mr()\n"));

            if (T_Ordered == false || this->_device->activePackets(this->_fnum) == false)
              {
                this->_device->shaddr.read (this->_local_mr, this->_local_offset,
                                this->_remote_mr, this->_remote_offset,
                                this->_bytes);
                this->setStatus (PAMI::Device::Done);
                return PAMI_SUCCESS;
              }

            TRACE_ERR((stderr, "<< GetDmaMessage::advance_mr(), return PAMI_EAGAIN\n"));
            return PAMI_EAGAIN;
          }


        public:

          inline GetDmaMessage (pami_event_function   fn,
                                void                * cookie,
                                T_Device            * device,
                                size_t                task,
                                size_t                bytes,
                                void                * local,
                                void                * remote) :
              DmaMessage<T_Device, T_Ordered> (GetDmaMessage::__advance_va,
                                              (void *)this, fn, cookie,
                                              device, task, bytes, local, remote)
          {};

          inline GetDmaMessage (pami_event_function   fn,
                                void                * cookie,
                                T_Device            * device,
                                size_t                fnum,
                                size_t                bytes,
                                Memregion           * local,
                                Memregion           * remote,
                                size_t                local_offset,
                                size_t                remote_offset) :
              DmaMessage<T_Device, T_Ordered> (GetDmaMessage::__advance_mr,
                                              (void *)this, fn, cookie,
                                              device, fnum, bytes, local, remote,
                                              local_offset, remote_offset)
          {};

      };  // PAMI::Device::Shmem::DmaMessage class
    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace
#undef TRACE_ERR
#endif // __components_devices_shmem_shmempacketmodel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
