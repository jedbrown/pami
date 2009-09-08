/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemPacketModel.h
 * \brief ???
 */

#ifndef __components_devices_shmem_shmempacketmodel_h__
#define __components_devices_shmem_shmempacketmodel_h__

#include "sys/xmi.h"

#include "../MessageModel.h"
#include "ShmemPacketDevice.h"
#include "ShmemSysDep.h"

#include "errno.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
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
    template <class T_Device, class T_Message>
    class ShmemPacketModel : public Interface::MessageModel<ShmemPacketModel<T_Device, T_Message>, T_Device, T_Message>
    {
      public:
        ///
        /// \brief Construct a Common Device Interface shared memory packet model.
        ///
        /// \param[in] device  Shared memory device
        ///
        ShmemPacketModel (T_Device & device, xmi_context_t context) :
            Interface::MessageModel < ShmemPacketModel<T_Device, T_Message>, T_Device, T_Message > (device, context),
            _device (device),
            _context (context)
        {};

        xmi_result_t init_impl (Interface::RecvFunction_t   direct_recv_func,
                                void                      * direct_recv_func_parm,
                                Interface::RecvFunction_t   read_recv_func,
                                void                      * read_recv_func_parm)
        {
          _dispatch_id = _device.registerRecvFunction (direct_recv_func, direct_recv_func_parm);
          return XMI_SUCCESS;  // <--- fix this
        };

        inline bool postPacket_impl (T_Message          * obj,
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     size_t               target_rank,
                                     void               * metadata,
                                     size_t               metasize,
                                     void               * payload,
                                     size_t               bytes)
        {
          size_t global, peer, sequence;
          _device._sysdep->mapping.task2node (target_rank, global, peer);

          TRACE_ERR((stderr,"ShmemPacketModel::postPacket_impl(1) .. target_rank = %zd, global = %zd, peer = %zd\n", target_rank, global, peer));
          if (_device.isSendQueueEmpty (peer) &&
              _device.writeSinglePacket (peer, _dispatch_id, metadata, metasize,
                                         payload, bytes, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_context, cookie, XMI_SUCCESS);

              return true;
            }

          new (obj) T_Message (_context, fn, cookie, _dispatch_id, metadata, metasize, payload, bytes, true);

          _device.post (peer, obj);

          return false;
        };

        inline bool postPacket_impl (T_Message          * obj,
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     size_t               target_rank,
                                     void               * metadata,
                                     size_t               metasize,
                                     void               * payload0,
                                     size_t               bytes0,
                                     void               * payload1,
                                     size_t               bytes1)
        {
          size_t global, peer, sequence;
          _device.getMapping()->rank2node (target_rank, global, peer);

          if (_device.isSendQueueEmpty (peer) &&
              _device.writeSinglePacket (peer, _dispatch_id,
                                         metadata, metasize,
                                         payload0, bytes0,
                                         payload1, bytes1,
                                         sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_context, cookie, XMI_SUCCESS);

              return true;
            }

          new (obj) T_Message (_context, fn, cookie, _dispatch_id, metadata, metasize,
                               payload0, bytes0, payload1, bytes1, true);

          _device.post (peer, obj);

          return false;
        };

        inline bool postPacket_impl (T_Message        * obj,
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     size_t             target_rank,
                                     void             * metadata,
                                     size_t             metasize,
                                     struct iovec_t   * iov,
                                     size_t             niov)
        {
          size_t global, peer, sequence;
          _device.getMapping()->rank2node (target_rank, global, peer);

          if (_device.isSendQueueEmpty (peer) &&
              _device.writeSinglePacket (peer, _dispatch_id, metadata, metasize,
                                         iov, niov, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_context, cookie, XMI_SUCCESS);

              return true;
            }

          new (obj) T_Message (_context, fn, cookie, _dispatch_id, metadata, metasize, iov, niov, true);

          _device.post (peer, obj);

          return false;
        };

        inline bool postMessage_impl (T_Message        * obj,
                                     xmi_event_function   fn,
                                     void               * cookie,
                                      size_t             target_rank,
                                      void             * metadata,
                                      size_t             metasize,
                                      void             * src,
                                      size_t             bytes)
        {
          size_t global, peer, sequence;
          _device._sysdep->mapping.task2node (target_rank, global, peer);

          new (obj) T_Message (_context, fn, cookie, _dispatch_id, metadata, metasize, src, bytes, false);

          if (_device.isSendQueueEmpty (peer))
            {
              while (_device.writeSinglePacket (peer, obj, sequence) == XMI_SUCCESS)
                {
                  if (obj->done())
                    {
                      // Invoke the send completion callback here.. may post
                      // another message!
                      obj->executeCallback ();
                      return true;
                    }
                }
            }

          _device.post (peer, obj);
          return false;
        };


      protected:
        T_Device      & _device;
        xmi_context_t   _context;
        size_t          _dispatch_id;
    };
  };
};
#undef TRACE_ERR
#endif // __components_devices_shmem_shmempacketmodel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
