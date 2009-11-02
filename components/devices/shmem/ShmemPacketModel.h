/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemPacketModel.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemPacketModel_h__
#define __components_devices_shmem_ShmemPacketModel_h__

#include <errno.h>

#include "Arch.h"

#include "sys/xmi.h"

#include "components/devices/myDmaModel.h" //for now using a separate model than M
#include "components/devices/MessageModel.h"
#include "components/devices/shmem/ShmemPacketDevice.h"

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
    template < class T_Device, class T_Message, class T_Memregion = XMI::MemRegion::Noop >
    class ShmemPacketModel : public Interface::MessageModel < ShmemPacketModel<T_Device, T_Message, T_Memregion>, T_Device, sizeof(T_Message) > ,
        public myInterface::DmaModel < ShmemPacketModel<T_Device, T_Message, T_Memregion>, T_Device, T_Memregion, sizeof(T_Message) >
    {
      public:
        ///
        /// \brief Construct a Common Device Interface shared memory packet model.
        ///
        /// \param[in] device  Shared memory device
        ///
        ShmemPacketModel (T_Device & device, xmi_context_t context) :
            Interface::MessageModel < ShmemPacketModel<T_Device, T_Message, T_Memregion>, T_Device, sizeof(T_Message) > (device, context),
            myInterface::DmaModel < ShmemPacketModel<T_Device, T_Message, T_Memregion>, T_Device, T_Memregion, sizeof(T_Message) > (device, context),
            _device (device),
            _context (context)
        {};

        static const bool   deterministic_packet_model   = true;
        static const bool   reliable_packet_model        = true;
        static const size_t packet_model_metadata_bytes  = T_Device::metadata_size;
        static const size_t packet_model_payload_bytes   = T_Device::payload_size;
        static const size_t packet_model_state_bytes     = sizeof(T_Message);

        static const bool   deterministic_message_model  = true;
        static const bool   reliable_message_model       = true;
        static const size_t message_model_metadata_bytes = T_Device::metadata_size;
        static const size_t message_model_payload_bytes  = T_Device::payload_size;
        static const size_t message_model_state_bytes    = sizeof(T_Message);

        xmi_result_t init_impl (size_t                      dispatch,
                                Interface::RecvFunction_t   direct_recv_func,
                                void                      * direct_recv_func_parm,
                                Interface::RecvFunction_t   read_recv_func,
                                void                      * read_recv_func_parm)
        {
          _dispatch_id = _device.registerRecvFunction (dispatch, direct_recv_func, direct_recv_func_parm);
          return XMI_SUCCESS;  // <--- fix this
        };

        inline bool postPacket_impl (uint8_t              (&state)[sizeof(T_Message)],
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     size_t               target_rank,
                                     void               * metadata,
                                     size_t               metasize,
                                     void               * payload,
                                     size_t               bytes)
        {
          size_t peer, sequence;
          XMI::Interface::Mapping::nodeaddr_t addr;
          TRACE_ERR((stderr, ">> ShmemPacketModel::postPacket_impl(1) .. target_rank = %zd\n", target_rank));
          __global.mapping.task2node (target_rank, addr);
          TRACE_ERR((stderr, "  ShmemPacketModel::postPacket_impl(1) .. target_rank = %zd -> {%zd, %zd}\n", target_rank, addr.global, addr.local));
          __global.mapping.node2peer (addr, peer);
          TRACE_ERR((stderr, "  ShmemPacketModel::postPacket_impl(1) .. {%zd, %zd} -> %zd\n", addr.global, addr.local, peer));

          TRACE_ERR((stderr, "ShmemPacketModel::postPacket_impl(1) .. target_rank = %zd, peer = %zd\n", target_rank, peer));

          if (_device.isSendQueueEmpty (peer) &&
              _device.writeSinglePacket (peer, _dispatch_id, metadata, metasize,
                                         payload, bytes, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_context, cookie, XMI_SUCCESS);

              return true;
            }

          T_Message * obj = (T_Message *) & state[0];
          new (obj) T_Message (_context, fn, cookie, _dispatch_id, metadata, metasize, payload, bytes, true);

          _device.post (peer, obj);

          return false;
        };

        inline bool postPacket_impl (uint8_t              (&state)[sizeof(T_Message)],
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
          size_t peer, sequence;
          XMI::Interface::Mapping::nodeaddr_t addr;
          __global.mapping.task2node (target_rank, addr);
          __global.mapping.node2peer (addr, peer);

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

          T_Message * obj = (T_Message *) & state[0];
          new (obj) T_Message (_context, fn, cookie, _dispatch_id, metadata, metasize,
                               payload0, bytes0, payload1, bytes1, true);

          _device.post (peer, obj);

          return false;
        };

        inline bool postPacket_impl (uint8_t              (&state)[sizeof(T_Message)],
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     size_t               target_rank,
                                     void               * metadata,
                                     size_t               metasize,
                                     struct iovec_t     * iov,
                                     size_t               niov)
        {
          size_t peer, sequence;
          XMI::Interface::Mapping::nodeaddr_t addr;
          __global.mapping.task2node (target_rank, addr);
          __global.mapping.node2peer (addr, peer);

          if (_device.isSendQueueEmpty (peer) &&
              _device.writeSinglePacket (peer, _dispatch_id, metadata, metasize,
                                         iov, niov, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_context, cookie, XMI_SUCCESS);

              return true;
            }

          T_Message * obj = (T_Message *) & state[0];
          new (obj) T_Message (_context, fn, cookie, _dispatch_id, metadata, metasize, iov, niov, true);

          _device.post (peer, obj);

          return false;
        };

        inline bool postPacketImmediate_impl (size_t   target_rank,
                                              void   * metadata,
                                              size_t   metasize,
                                              void   * payload0,
                                              size_t   bytes0,
                                              void   * payload1,
                                              size_t   bytes1)
        {
          size_t peer = 0, sequence;
          XMI::Interface::Mapping::nodeaddr_t addr;
          TRACE_ERR((stderr, ">> ShmemPacketModel::postPacketImmediate_impl(1) .. target_rank = %zd\n", target_rank));
          __global.mapping.task2node (target_rank, addr);
          TRACE_ERR((stderr, "   ShmemPacketModel::postPacketImmediate_impl(1) .. target_rank = %zd -> {%zd, %zd}\n", target_rank, addr.global, addr.local));
          __global.mapping.node2peer (addr, peer);

          TRACE_ERR((stderr, "<< ShmemPacketModel::postPacketImmediate_impl(1) .. {%zd, %zd} -> peer = %zd\n", addr.global, addr.local, peer));
          return (_device.isSendQueueEmpty (peer) &&
                  _device.writeSinglePacket (peer, _dispatch_id,
                                             metadata, metasize,
                                             payload0, bytes0,
                                             payload1, bytes1,
                                             sequence) == XMI_SUCCESS);
        };

        inline bool postMessage_impl (uint8_t              (&state)[sizeof(T_Message)],
                                      xmi_event_function   fn,
                                      void               * cookie,
                                      size_t               target_rank,
                                      void               * metadata,
                                      size_t               metasize,
                                      void               * src,
                                      size_t               bytes)
        {
          TRACE_ERR((stderr, ">> ShmemPacketModel::postMessage_impl() .. target_rank = %zd\n", target_rank));
          size_t sequence;

          XMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.task2node (target_rank, address);
          TRACE_ERR((stderr, "   ShmemPacketModel::postMessage_impl() .. target_rank = %zd -> {%zd, %zd}\n", target_rank, address.global, address.local));

          size_t peer;
          __global.mapping.node2peer (address, peer);
          TRACE_ERR((stderr, "   ShmemPacketModel::postMessage_impl() .. {%zd, %zd} -> %zd\n", address.global, address.local, peer));

          TRACE_ERR((stderr, "   ShmemPacketModel::postMessage_impl() .. target_rank = %zd, peer = %zd\n", target_rank, peer));
          T_Message * obj = (T_Message *) & state[0];
          new (obj) T_Message (_context, fn, cookie, _dispatch_id, metadata, metasize, src, bytes, false);

          TRACE_ERR((stderr, "   ShmemPacketModel::postMessage_impl() .. 0\n"));

          if (_device.isSendQueueEmpty (peer))
            {
              TRACE_ERR((stderr, "   ShmemPacketModel::postMessage_impl() .. 1\n"));

              while (_device.writeSinglePacket (peer, obj, sequence) == XMI_SUCCESS)
                {
                  TRACE_ERR((stderr, "   ShmemPacketModel::postMessage_impl() .. 2\n"));

                  if (obj->done())
                    {
                      TRACE_ERR((stderr, "   ShmemPacketModel::postMessage_impl() .. 3\n"));
                      // Invoke the send completion callback here.. may post
                      // another message!
                      obj->executeCallback ();
                      TRACE_ERR((stderr, "<< ShmemPacketModel::postMessage_impl() .. 4\n"));
                      return true;
                    }
                }
            }

          TRACE_ERR((stderr, "   ShmemPacketModel::postMessage_impl() .. 5\n"));
          _device.post (peer, obj);
          TRACE_ERR((stderr, "<< ShmemPacketModel::postMessage_impl() .. 6\n"));
          return false;
        };

        inline bool postDmaPut_impl (uint8_t              state[sizeof(T_Message)],
                                     xmi_event_function   local_fn,
                                     void               * cookie,
                                     size_t               target_rank,
                                     T_Memregion        * local_memregion,
                                     size_t               local_offset,
                                     T_Memregion        * remote_memregion,
                                     size_t               remote_offset,
                                     size_t               bytes)
        {
          if (! T_Memregion::shared_address_write_supported)
            XMI_abort();

          size_t peer;
          XMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.task2node (target_rank, address);
          __global.mapping.node2peer (address, peer);

          if (_device.isSendQueueEmpty (peer))
            {
              local_memregion->write (local_offset,
                                      remote_memregion,
                                      remote_offset,
                                      bytes);

              if (local_fn) local_fn (_context, cookie, XMI_SUCCESS);

              return XMI_SUCCESS;

            }

          T_Message * obj = (T_Message *) & state[0];
          new (obj) T_Message (local_fn, cookie,
                               local_memregion, local_offset,
                               remote_memregion, remote_offset,
                               bytes, true);

          _device.post (peer, obj);
          return XMI_SUCCESS;
        };

        inline bool postDmaGet_impl (uint8_t              state[sizeof(T_Message)],
                                     xmi_event_function   local_fn,
                                     void               * cookie,
                                     size_t               target_rank,
                                     T_Memregion        * local_memregion,
                                     size_t               local_offset,
                                     T_Memregion        * remote_memregion,
                                     size_t               remote_offset,
                                     size_t               bytes)
        {
          if (! T_Memregion::shared_address_read_supported)
            XMI_abort();

          size_t peer;
          XMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.task2node (target_rank, address);
          __global.mapping.node2peer (address, peer);

          if (_device.isSendQueueEmpty (peer))
            {
              local_memregion->read (local_offset,
                                     remote_memregion,
                                     remote_offset,
                                     bytes);

              if (local_fn) local_fn (_context, cookie, XMI_SUCCESS);

              return XMI_SUCCESS;

            }

          T_Message * obj = (T_Message *) & state[0];
          new (obj) T_Message (local_fn, cookie,
                               local_memregion, local_offset,
                               remote_memregion, remote_offset,
                               bytes, false);

          _device.post (peer, obj);

          return XMI_SUCCESS;
        };



      protected:
        T_Device      & _device;
        xmi_context_t   _context;
        uint16_t        _dispatch_id;
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
