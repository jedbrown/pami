/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemModel.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemModel_h__
#define __components_devices_shmem_ShmemModel_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "Memregion.h"

#include "sys/xmi.h"

#include "components/devices/PacketInterface.h"
#include "components/devices/DmaInterface.h"

#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemMessage.h"

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
    class ShmemModel : public Interface::PacketModel < ShmemModel<T_Device>, T_Device, sizeof(ShmemMessage) > ,
        public Interface::DmaModel < ShmemModel<T_Device>, T_Device, sizeof(ShmemMessage) >
    {
      public:
        ///
        /// \brief Construct a Common Device Interface shared memory packet model.
        ///
        /// \param[in] device  Shared memory device
        ///
        ShmemModel (T_Device & device) :
            Interface::PacketModel < ShmemModel<T_Device>, T_Device, sizeof(ShmemMessage) > (device),
            Interface::DmaModel < ShmemModel<T_Device>, T_Device, sizeof(ShmemMessage) > (device),
            _device (device)
        {};

#ifdef EMULATE_UNRELIABLE_SHMEM_DEVICE
        static const bool   reliable_packet_model             = false;
#else
        static const bool   reliable_packet_model             = true;
#endif
        static const bool   deterministic_packet_model        = true;
        static const size_t packet_model_metadata_bytes       = T_Device::metadata_size;
        static const size_t packet_model_multi_metadata_bytes = T_Device::metadata_size;
        static const size_t packet_model_payload_bytes        = T_Device::payload_size;
        static const size_t packet_model_state_bytes          = sizeof(ShmemMessage);

        xmi_result_t init_impl (size_t                      dispatch,
                                Interface::RecvFunction_t   direct_recv_func,
                                void                      * direct_recv_func_parm,
                                Interface::RecvFunction_t   read_recv_func,
                                void                      * read_recv_func_parm)
        {
          return _device.registerRecvFunction (dispatch, direct_recv_func, direct_recv_func_parm, _dispatch_id);
        };

        inline bool postPacket_impl (uint8_t              (&state)[sizeof(ShmemMessage)],
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     xmi_task_t           target_task,
                                     size_t               target_offset,
                                     void               * metadata,
                                     size_t               metasize,
                                     struct iovec_t     * iov,
                                     size_t               niov)
        {
          size_t peer, sequence;
          XMI::Interface::Mapping::nodeaddr_t addr;
          __global.mapping.task2node (target_task, addr);
          __global.mapping.node2peer (addr, peer);
#ifdef __bgq__
          peer = target_task; //hack
#endif
          TRACE_ERR((stderr, "<< ShmemModel::postPacket_impl(iov) .. {%zd, %zd} -> peer = %zd\n", addr.global, addr.local, peer));

          if (_device.isSendQueueEmpty (peer) &&
              _device.writeSinglePacket (peer, _dispatch_id, metadata, metasize,
                                         iov, niov, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_device.getContext(), cookie, XMI_SUCCESS);

              return true;
            }

          ShmemMessage * obj = (ShmemMessage *) & state[0];
          new (obj) ShmemMessage (_device.getContext(), fn, cookie, _dispatch_id, metadata, metasize, iov, niov, true);

          _device.post (peer, obj);

          return false;
        };

        template <unsigned T_Niov>
        inline bool postPacket_impl (uint8_t              (&state)[sizeof(ShmemMessage)],
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     xmi_task_t           target_task,
                                     size_t               target_offset,
                                     void               * metadata,
                                     size_t               metasize,
                                     struct iovec         (&iov)[T_Niov])
        {
#ifdef ERROR_CHECKS
          {
            unsigned i;
            size_t bytes = 0;
            for (i=0; i<T_Niov; i++) bytes += iov[i].iov_len;
            XMI_assert(bytes<=packet_model_payload_bytes);
          }
#endif
          size_t peer=0, sequence;
          XMI::Interface::Mapping::nodeaddr_t addr;
          __global.mapping.task2node (target_task, addr);
          __global.mapping.node2peer (addr, peer);
#ifdef __bgq__
          peer = target_task; //hack
#endif
          TRACE_ERR((stderr, "<< ShmemModel::postPacket_impl(T_Niov) .. {%zd, %zd} -> peer = %zd\n", addr.global, addr.local, peer));
          if (_device.isSendQueueEmpty (peer) &&
              _device.writeSinglePacket (peer, _dispatch_id, metadata, metasize,
                                         iov, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_device.getContext(), cookie, XMI_SUCCESS);

              return true;
            }

          ShmemMessage * obj = (ShmemMessage *) & state[0];
          new (obj) ShmemMessage (_device.getContext(), fn, cookie, _dispatch_id, metadata, metasize, iov, T_Niov, true);

          _device.post (peer, obj);

          return false;
        };

        inline bool postPacket_impl (uint8_t              (&state)[sizeof(ShmemMessage)],
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     xmi_task_t           target_task,
                                     size_t               target_offset,
                                     void               * metadata,
                                     size_t               metasize,
                                     void               * payload,
                                     size_t               length)
        {
          size_t peer, sequence;
          XMI::Interface::Mapping::nodeaddr_t addr;
          __global.mapping.task2node (target_task, addr);
          __global.mapping.node2peer (addr, peer);
#ifdef __bgq__
          peer = target_task; //hack
#endif
          TRACE_ERR((stderr, "<< ShmemModel::postPacket_impl(contiguous) .. {%zd, %zd} -> peer = %zd\n", addr.global, addr.local, peer));

          if (_device.isSendQueueEmpty (peer) &&
              _device.writeSinglePacket (peer, _dispatch_id, metadata, metasize,
                                         payload, length, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_device.getContext(), cookie, XMI_SUCCESS);

              return true;
            }

          ShmemMessage * obj = (ShmemMessage *) & state[0];
          new (obj) ShmemMessage (_device.getContext(), fn, cookie, _dispatch_id, metadata, metasize, payload, length, true);

          _device.post (peer, obj);

          return false;
        };

        template <unsigned T_Niov>
        inline bool postPacket_impl (xmi_task_t     target_task,
                                     size_t         target_offset,
                                     void         * metadata,
                                     size_t         metasize,
                                     struct iovec   (&iov)[T_Niov])
        {
#ifdef ERROR_CHECKS
          {
            unsigned i;
            size_t bytes = 0;
            for (i=0; i<T_Niov; i++) bytes += iov[i].iov_len;
            XMI_assert(bytes<=packet_model_payload_bytes);
          }
#endif
          size_t peer = 0, sequence;
          XMI::Interface::Mapping::nodeaddr_t addr;
          TRACE_ERR((stderr, ">> ShmemModel::postPacket_impl(immediate) .. target_task = %zd, iov = %p, T_Niov = %zd\n", target_task, iov, T_Niov));
          __global.mapping.task2node (target_task, addr);
          TRACE_ERR((stderr, "   ShmemModel::postPacket_impl(immediate) .. target_task = %zd -> {%zd, %zd}\n", target_task, addr.global, addr.local));
          __global.mapping.node2peer (addr, peer);
#ifdef __bgq__
          peer = target_task; //hack
#endif

          TRACE_ERR((stderr, "<< ShmemModel::postPacket_impl(immediate) .. {%zd, %zd} -> peer = %zd\n", addr.global, addr.local, peer));
          return (_device.isSendQueueEmpty (peer) &&
                  _device.writeSinglePacket (peer, _dispatch_id,
                                             metadata, metasize, iov,
                                             sequence) == XMI_SUCCESS);
          return false;
        };

        inline bool postMultiPacket_impl (uint8_t              (&state)[sizeof(ShmemMessage)],
                                          xmi_event_function   fn,
                                          void               * cookie,
                                          xmi_task_t           target_task,
                                          size_t               target_offset,
                                          void               * metadata,
                                          size_t               metasize,
                                          void               * payload,
                                          size_t               length)
        {
          TRACE_ERR((stderr, ">> ShmemModel::postMessage_impl() Multipacket.. target_task = %zd\n", target_task));
          size_t sequence;

          XMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.task2node (target_task, address);
          TRACE_ERR((stderr, "   ShmemModel::postMessage_impl()Multipacket .. target_task = %zd -> {%zd, %zd}\n", target_task, address.global, address.local));

          size_t peer=0;
          __global.mapping.node2peer (address, peer);
          TRACE_ERR((stderr, "   ShmemModel::postMessage_impl()Multipacket .. {%zd, %zd} -> %zd\n", address.global, address.local, peer));
#ifdef __bgq__
	  peer = target_task; //hack
#endif

          TRACE_ERR((stderr, "   ShmemModel::postMessage_impl()Multipacket .. target_task = %zd, peer = %zd\n", target_task, peer));
          ShmemMessage * msg = (ShmemMessage *) & state[0];
          new (msg) ShmemMessage (_device.getContext(), fn, cookie, _dispatch_id, metadata, metasize, payload, length, false);

          TRACE_ERR((stderr, "   ShmemModel::postMessage_impl()Multipacket .. 0\n"));

          if (_device.isSendQueueEmpty (peer))
            {
              TRACE_ERR((stderr, "   ShmemModel::postMessage_impl()Multipacket .. 1\n"));

              while (_device.writeSinglePacket (peer, msg, sequence) == XMI_SUCCESS)
                {
                  TRACE_ERR((stderr, "   ShmemModel::postMessage_impl()Multipacket .. 2\n"));

                  if (msg->done())
                    {
                      TRACE_ERR((stderr, "   ShmemModel::postMessage_impl()Multipacket .. 3\n"));
                      // Invoke the send completion callback here.. may post
                      // another message!
                      msg->executeCallback ();
                      TRACE_ERR((stderr, "<< ShmemModel::postMessage_impl()Multipacket .. 4\n"));
                      return true;
                    }
                }
            }

          TRACE_ERR((stderr, "   ShmemModel::postMessage_impl()Multipacket .. 5\n"));
          _device.post (peer, msg);
          TRACE_ERR((stderr, "<< ShmemModel::postMessage_impl()Multipacket .. 6\n"));
          return false;
        };

        inline bool postDmaPut_impl (uint8_t              state[sizeof(ShmemMessage)],
                                     xmi_event_function   local_fn,
                                     void               * cookie,
                                     xmi_task_t           target_task,
                                     Memregion          * local_memregion,
                                     size_t               local_offset,
                                     Memregion          * remote_memregion,
                                     size_t               remote_offset,
                                     size_t               bytes)
        {
          if (! Memregion::shared_address_write_supported)
            XMI_abortf("%s<%d>\n",__FILE__,__LINE__);

          size_t peer;
          XMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.task2node (target_task, address);
          __global.mapping.node2peer (address, peer);

          if (_device.isSendQueueEmpty (peer))
            {
              local_memregion->write (local_offset,
                                      remote_memregion,
                                      remote_offset,
                                      bytes);

              if (local_fn) local_fn (_device.getContext(), cookie, XMI_SUCCESS);

              return XMI_SUCCESS;

            }

          ShmemMessage * obj = (ShmemMessage *) & state[0];
          new (obj) ShmemMessage (local_fn, cookie,
                                  local_memregion, local_offset,
                                  remote_memregion, remote_offset,
                                  bytes, true);

          _device.post (peer, obj);
          return XMI_SUCCESS;
        };

        inline bool postDmaGet_impl (uint8_t              state[sizeof(ShmemMessage)],
                                     xmi_event_function   local_fn,
                                     void               * cookie,
                                     xmi_task_t           target_task,
                                     Memregion          * local_memregion,
                                     size_t               local_offset,
                                     Memregion          * remote_memregion,
                                     size_t               remote_offset,
                                     size_t               bytes)
        {
          if (! Memregion::shared_address_read_supported)
            XMI_abortf("%s<%d>\n",__FILE__,__LINE__);

          size_t peer;
          XMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.task2node (target_task, address);
          __global.mapping.node2peer (address, peer);

          if (_device.isSendQueueEmpty (peer))
            {
              local_memregion->read (local_offset,
                                     remote_memregion,
                                     remote_offset,
                                     bytes);

              if (local_fn) local_fn (_device.getContext(), cookie, XMI_SUCCESS);

              return XMI_SUCCESS;

            }

          ShmemMessage * obj = (ShmemMessage *) & state[0];
          new (obj) ShmemMessage (local_fn, cookie,
                                  local_memregion, local_offset,
                                  remote_memregion, remote_offset,
                                  bytes, false);

          _device.post (peer, obj);

          return XMI_SUCCESS;
        };



      protected:
        T_Device      & _device;
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
