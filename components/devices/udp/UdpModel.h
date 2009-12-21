/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/udp/UdpModel.h
 * \brief Provides the implementation of the PacketModel for the UDP device
 */

#ifndef __components_devices_udp_UdpModel_h__
#define __components_devices_udp_UdpModel_h__

#include <sys/uio.h>
#include "sys/xmi.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/udp/UdpMessage.h"
#include "errno.h"

namespace XMI
{
  namespace Device
  {
  namespace UDP
  {
    template <class T_Device, class T_Message>
    class UdpModel : public Interface::PacketModel<UdpModel<T_Device, T_Message>, T_Device, sizeof(T_Message)>
    {
    public:
       UdpModel (T_Device & device, xmi_client_t client, size_t context) :
            Interface::PacketModel < UdpModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device, client, context),
            _device (device),
            _client (client),
            _context (context)
        {};

       // PacketInterface implements isPacket* to return these variables
      static const bool   deterministic_packet_model   = false;
      static const bool   reliable_packet_model        = false;

      static const size_t packet_model_metadata_bytes       = T_Device::metadata_size;
      static const size_t packet_model_multi_metadata_bytes = T_Device::metadata_size;
      static const size_t packet_model_payload_bytes        = T_Device::payload_size;
      static const size_t    packet_model_state_bytes       = sizeof(T_Message);

      xmi_result_t init_impl (size_t                      dispatch,
                              Interface::RecvFunction_t   direct_recv_func,
                              void                      * direct_recv_func_parm,
                              Interface::RecvFunction_t   read_recv_func,
                              void                      * read_recv_func_parm)
        {
         _usr_dispatch_id = dispatch;
         return _device.setDispatchFunc (dispatch, direct_recv_func, direct_recv_func_parm);
        };

      inline bool postPacket_impl (uint8_t              (&state)[UdpModel::packet_model_state_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t               target,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec       * iov,
                                   size_t               niov)
        {
          T_Message * msg = (T_Message *) & state[0];
          new (msg) T_Message (_client, _context, fn, cookie, _device_dispatch_id,
                               metadata, metasize, iov, niov, false);
          _device.post(target, msg );
          return false;
        };

      template <unsigned T_Niov>
      inline bool postPacket_impl (uint8_t              (&state)[UdpModel::packet_model_state_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t               target,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec         (&iov)[T_Niov])
        {
            return postPacket(state, fn, cookie, target, metadata, metasize, iov, T_Niov );
        };

      template <unsigned T_Niov>
      inline bool postPacket_impl (size_t         target,
                                   void         * metadata,
                                   size_t         metasize,
                                   struct iovec   (&iov)[T_Niov])
        {
          // Cannot ensure that this will send right away, so just indicate this doesn't work
          return false;

        }

      template <unsigned T_Niov>
      inline bool postMultiPacket_impl (uint8_t              (&state)[UdpModel::packet_model_state_bytes],
                                        xmi_event_function   fn,
                                        void               * cookie,
                                        size_t               target,
                                        void               * metadata,
                                        size_t               metasize,
                                        struct iovec         (&iov)[T_Niov])
        {
         T_Message * msg = (T_Message *) & state[0];
          new (msg) T_Message (_client, _context, fn, cookie, _device_dispatch_id, metadata,
                               metasize, iov, T_Niov, true);
          _device.post(target, msg );
          return false;
        }

       inline int read_impl(void * dst, size_t bytes, void * cookie )
       {
         return _device.read(dst, bytes, cookie);
       }

    protected:
      T_Device                   & _device;
      xmi_client_t                 _client;
      size_t                       _context;
      uint32_t                     _usr_dispatch_id;
      uint32_t                     _device_dispatch_id;

    };
  };
  };
};
#endif // __components_devices_udp_udppacketmodel_h__
