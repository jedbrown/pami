/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#include <sys/uio.h>

#include "Arch.h"

#include "sys/xmi.h"

#include "components/devices/PacketInterface.h"
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemPacketMessage.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //  fprintf x
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
    class ShmemPacketModel : public Interface::PacketModel < ShmemPacketModel<T_Device>, T_Device, sizeof(ShmemPacketMessage<T_Device>) >
    {
      public:
        ///
        /// \brief Construct a Common Device Interface shared memory packet model.
        ///
        /// \param[in] device  Shared memory device
        ///
        ShmemPacketModel (T_Device & device) :
            Interface::PacketModel < ShmemPacketModel<T_Device>, T_Device, sizeof(ShmemPacketMessage<T_Device>) > (device),
            _device (device)
        {
          COMPILE_TIME_ASSERT(sizeof(ShmemPacketMessage<T_Device>) == sizeof(ShmemMultiPacketMessage<T_Device>));
        };

#ifdef EMULATE_UNRELIABLE_SHMEM_DEVICE
        static const bool   reliable_packet_model             = false;
#else
        static const bool   reliable_packet_model             = true;
#endif
        static const bool   deterministic_packet_model        = true;
        static const size_t packet_model_metadata_bytes       = T_Device::metadata_size;
        static const size_t packet_model_multi_metadata_bytes = T_Device::metadata_size;
        static const size_t packet_model_payload_bytes        = T_Device::payload_size;
        static const size_t packet_model_state_bytes          = sizeof(ShmemPacketMessage<T_Device>);

        xmi_result_t init_impl (size_t                      dispatch,
                                Interface::RecvFunction_t   direct_recv_func,
                                void                      * direct_recv_func_parm,
                                Interface::RecvFunction_t   read_recv_func,
                                void                      * read_recv_func_parm)
        {
          return _device.registerRecvFunction (dispatch, direct_recv_func, direct_recv_func_parm, _dispatch_id);
        };

        inline bool postPacket_impl (uint8_t              (&state)[sizeof(ShmemPacketMessage<T_Device>)],
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     xmi_task_t           target_task,
                                     size_t               target_offset,
                                     void               * metadata,
                                     size_t               metasize,
                                     struct iovec_t     * iov,
                                     size_t               niov)
        {
          size_t sequence;
          size_t fnum = _device.fnum (_device.task2peer(target_task), target_offset);

          if (_device.isSendQueueEmpty (fnum) &&
              _device.writeSinglePacket (fnum, _dispatch_id, metadata, metasize,
                                         iov, niov, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_context, cookie, XMI_SUCCESS);

              return true;
            }

          ShmemPacketMessage<T_Device> * obj = (ShmemPacketMessage<T_Device> *) & state[0];
          new (obj) ShmemPacketMessage<T_Device> (fn, cookie, &_device, fnum);
          obj->setHeader (_dispatch_id, metadata, metasize);
          obj->setPayload (iov, niov);
          _device.post (fnum, obj);

          return false;
        };

        template <unsigned T_Niov>
        inline bool postPacket_impl (uint8_t              (&state)[sizeof(ShmemPacketMessage<T_Device>)],
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

            for (i = 0; i < T_Niov; i++) bytes += iov[i].iov_len;

            XMI_assert(bytes <= packet_model_payload_bytes);
          }
#endif
          size_t sequence;
          size_t fnum = _device.fnum (_device.task2peer(target_task), target_offset);

          if (_device.isSendQueueEmpty (fnum) &&
              _device.writeSinglePacket (fnum, _dispatch_id, metadata, metasize,
                                         iov, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_context, cookie, XMI_SUCCESS);

              return true;
            }

          ShmemPacketMessage<T_Device> * obj = (ShmemPacketMessage<T_Device> *) & state[0];
          new (obj) ShmemPacketMessage<T_Device> (fn, cookie, &_device, fnum);
          obj->setHeader (_dispatch_id, metadata, metasize);
          obj->setPayload (iov);
          _device.post (fnum, obj);

          return false;
        };

        inline bool postPacket_impl (uint8_t              (&state)[sizeof(ShmemPacketMessage<T_Device>)],
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     xmi_task_t           target_task,
                                     size_t               target_offset,
                                     void               * metadata,
                                     size_t               metasize,
                                     void               * payload,
                                     size_t               length)
        {
          size_t sequence;
          size_t fnum = _device.fnum (_device.task2peer(target_task), target_offset);

          if (_device.isSendQueueEmpty (fnum) &&
              _device.writeSinglePacket (fnum, _dispatch_id, metadata, metasize,
                                         payload, length, sequence) == XMI_SUCCESS)
            {
              if (fn) fn (_device.getContext(), cookie, XMI_SUCCESS);

              return true;
            }

          ShmemPacketMessage<T_Device> * obj = (ShmemPacketMessage<T_Device> *) & state[0];
          new (obj) ShmemPacketMessage<T_Device> (fn, cookie, &_device, fnum);
          obj->setHeader (_dispatch_id, metadata, metasize);
          obj->setPayload (payload, length);
          _device.post (fnum, obj);

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

            for (i = 0; i < T_Niov; i++) bytes += iov[i].iov_len;

            XMI_assert(bytes <= packet_model_payload_bytes);
          }
#endif
          TRACE_ERR((stderr,">> ShmemPacketModel::postPacket_impl(\"immediate\")\n"));
          size_t sequence;
          size_t fnum = _device.fnum (_device.task2peer(target_task), target_offset);
          TRACE_ERR((stderr,"   ShmemPacketModel::postPacket_impl(\"immediate\") .. after _device.fnum() and _device.task2peer()\n"));
          return (_device.isSendQueueEmpty (fnum) &&
                  _device.writeSinglePacket (fnum, _dispatch_id,
                                             metadata, metasize, iov,
                                             sequence) == XMI_SUCCESS);
        };

        inline bool postMultiPacket_impl (uint8_t              (&state)[sizeof(ShmemMultiPacketMessage<T_Device>)],
                                          xmi_event_function   fn,
                                          void               * cookie,
                                          xmi_task_t           target_task,
                                          size_t               target_offset,
                                          void               * metadata,
                                          size_t               metasize,
                                          void               * payload,
                                          size_t               length)
        {
          size_t fnum = _device.fnum (_device.task2peer(target_task), target_offset);

          ShmemMultiPacketMessage<T_Device> * obj = (ShmemMultiPacketMessage<T_Device> *) & state[0];
          new (obj) ShmemMultiPacketMessage<T_Device> (fn, cookie, &_device, fnum);
          obj->setHeader (_dispatch_id, metadata, metasize);
          obj->setPayload (payload, length);

          if (_device.isSendQueueEmpty (fnum) && obj->advance(_context))
            return true;

          _device.post (fnum, obj);
          return false;
        };

      protected:

        T_Device      & _device;
        uint16_t        _dispatch_id;
        xmi_context_t   _context;

    };  // XMI::Device::ShmemPacketModel class
  };    // XMI::Device namespace
};      // XMI namespace
#undef TRACE_ERR
#endif // __components_devices_shmem_ShmemPacketModel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
