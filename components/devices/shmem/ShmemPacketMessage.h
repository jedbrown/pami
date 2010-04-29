/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemPacketMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemPacketMessage_h__
#define __components_devices_shmem_ShmemPacketMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"

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

      template <class T_Device>
      class PacketMessage : public SendQueue::Message
      {
        protected:
          // invoked by the thread object
          /// \see SendQueue::Message::_work
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            PacketMessage * msg = (PacketMessage *) cookie;
            return msg->advance();
          };

          inline pami_result_t advance ()
          {
            size_t sequence = 0;
            TRACE_ERR((stderr, ">> PacketMessage::advance()\n"));

            if (_device->writeSinglePacket (_fifo, _dispatch_id, _metadata, _metasize,
                                            _iov, _niov, sequence) == PAMI_SUCCESS)
              {
                TRACE_ERR((stderr, "   PacketMessage::advance(), write single packet successful\n"));
                this->setStatus (PAMI::Device::Done);
                TRACE_ERR((stderr, "<< PacketMessage::advance(), return PAMI_SUCCESS\n"));
                return PAMI_SUCCESS;
              }

            TRACE_ERR((stderr, "<< PacketMessage::advance(), return PAMI_EAGAIN\n"));
            return PAMI_EAGAIN;
          }

          inline PacketMessage (pami_work_function    work_func,
                                void               * work_cookie,
                                pami_event_function   fn,
                                void               * cookie,
                                T_Device           * device,
                                size_t               fifo) :
              SendQueue::Message (work_func, work_cookie, fn, cookie, device->getContextOffset()),
              _device (device),
              _fifo (fifo)
          {};


        public:
          inline PacketMessage (pami_event_function   fn,
                                void               * cookie,
                                T_Device           * device,
                                size_t               fifo) :
              SendQueue::Message (PacketMessage::__advance, this, fn, cookie, device->getContextOffset()),
              _device (device),
              _fifo (fifo)
          {
            TRACE_ERR((stderr, "<> PacketMessage::PacketMessage()\n"));
          };

          inline void setHeader (uint16_t   dispatch_id,
                                 void     * metadata,
                                 size_t     metasize)
          {
            _dispatch_id = dispatch_id;
            memcpy ((void *)&_metadata, metadata, metasize);
            _metasize = metasize;
          };

          inline void setPayload (void * src, size_t bytes)
          {
            __iov.iov_base = src;
            __iov.iov_len  = bytes;
            _iov  = & __iov;
            _niov = 1;
          };

          inline void setPayload (struct iovec * iov, size_t niov)
          {
            _iov = iov;
            _niov = niov;
          };

          template <unsigned T_Niov>
          inline void setPayload (struct iovec (&iov)[T_Niov])
          {
            _iov  = & iov[0];
            _niov = T_Niov;
          };

        protected:

          T_Device      * _device;
          size_t          _fifo;

          uint16_t        _dispatch_id;
          size_t          _metasize;
          uint8_t         _metadata[T_Device::metadata_size];

          struct iovec  * _iov;
          size_t          _niov;
          struct iovec    __iov;
      };  // PAMI::Device::PacketMessage class

      template <class T_Device>
      class MultiPacketMessage : public PacketMessage<T_Device>
      {
        protected:
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            MultiPacketMessage * msg = (MultiPacketMessage *) cookie;
            return msg->advance();
          };

        public:
          inline MultiPacketMessage (pami_event_function   fn,
                                     void               * cookie,
                                     T_Device           * device,
                                     size_t               fifo) :
              PacketMessage<T_Device> (MultiPacketMessage<T_Device>::__advance, (void *)this, fn, cookie, device, fifo)
          {};

          inline pami_result_t advance ()
          {
            size_t sequence = 0;
            size_t bytes = MIN(this->__iov.iov_len, T_Device::payload_size);
            TRACE_ERR((stderr, ">> MultiPacketMessage::advance() .. __iov.iov_len = %zu, T_Device::payload_size = %zu, bytes = %zu\n", this->__iov.iov_len, T_Device::payload_size, bytes));

            while (this->_device->writeSinglePacket (this->_fifo, this->_dispatch_id,
                                                     this->_metadata, this->_metasize,
                                                     this->__iov.iov_base, bytes,
                                                     sequence) == PAMI_SUCCESS)
              {
                TRACE_ERR((stderr, "   MultiPacketMessage::advance() .. this->__iov.iov_base = %p, this->__iov.iov_len = %zu, bytes = %zu\n", this->__iov.iov_base, this->__iov.iov_len, bytes));
                if (this->__iov.iov_len <= bytes)
                  {
                    this->setStatus (PAMI::Device::Done);
                    TRACE_ERR((stderr, "<< MultiPacketMessage::advance() .. done (== PAMI_SUCCESS)\n"));
                    return PAMI_SUCCESS;
                  }

                uint8_t * tmp = (uint8_t *) this->__iov.iov_base;
                this->__iov.iov_base = (void *)(tmp + bytes);
                this->__iov.iov_len -= bytes;
                bytes = MIN(this->__iov.iov_len, T_Device::payload_size);
                TRACE_ERR((stderr, "   MultiPacketMessage::advance() .. update state, __iov.iov_base = %p, __iov.iov_len = %zu, bytes = %zu\n", this->__iov.iov_base, this->__iov.iov_len, bytes));
              }

            TRACE_ERR((stderr, "<< MultiPacketMessage::advance() .. return PAMI_EAGAIN (== \"not done\")\n"));
            return PAMI_EAGAIN;
          };

          inline void setPayload (void * src, size_t bytes)
          {
            this->__iov.iov_base = src;
            this->__iov.iov_len  = bytes;
          };
      };  // PAMI::Device::Shmem::MultiPacketMessage class
    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemPacketMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
