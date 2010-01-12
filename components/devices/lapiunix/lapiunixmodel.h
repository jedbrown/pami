/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixmodel_h__
#define __components_devices_lapiunix_lapiunixmodel_h__

#include "sys/xmi.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "errno.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class LAPIModel : public Interface::PacketModel<LAPIModel<T_Device, T_Message>, T_Device, sizeof(T_Message)>
    {
    public:
      LAPIModel (T_Device & device, xmi_client_t client, size_t context) :
        Interface::PacketModel < LAPIModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device,client,context),
        _device (device),
        _client (client),
        _context(context)
        {};
      static const bool   deterministic_packet_model         = true;
      static const bool   reliable_packet_model              = true;
      static const size_t packet_model_multi_metadata_bytes  = T_Device::metadata_size;
      static const size_t packet_model_metadata_bytes        = T_Device::metadata_size;
      static const size_t packet_model_payload_bytes         = T_Device::payload_size;
      static const size_t packet_model_status_bytes          = sizeof(T_Message);
      static const size_t packet_model_state_bytes           = sizeof(T_Message);

      xmi_result_t init_impl (size_t                      dispatch,
			      Interface::RecvFunction_t   direct_recv_func,
			      void                      * direct_recv_func_parm,
			      Interface::RecvFunction_t   read_recv_func,
			      void                      * read_recv_func_parm)
        {
          _dispatch_id = _device.registerRecvFunction (direct_recv_func, direct_recv_func_parm);
          return XMI_SUCCESS;
        };

      inline bool isDeterministic ()
        {
          return true;
        }

      template <unsigned T_Niov>
      inline bool postPacket_impl (uint8_t              (&state)[LAPIModel::packet_model_status_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t               target_rank,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec   (&iov)[T_Niov])
        {
          XMI_assert(T_Niov==1);
          int rc = LAPI_SUCCESS;
          LAPIMessage * msg = (LAPIMessage *)state;
          new(msg)LAPIMessage(this->_context,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=0;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=iov[0].iov_len;
          msg->_p2p_msg._payloadsize1=0;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], iov[0].iov_base, iov[0].iov_len);

          _device.enqueue(msg);
          assert(rc == LAPI_SUCCESS);

          return true;
        };

      inline bool postPacket_impl (uint8_t              (&state)[LAPIModel::packet_model_status_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t               target_rank,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec_t     * iov,
                                   size_t               niov)
        {
          XMI_abort();
          return false;
        };

      inline bool postPacket_impl (uint8_t              (&state)[LAPIModel::packet_model_status_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t               target_rank,
                                   void               * metadata,
                                   size_t               metasize,
                                   void               * payload,
                                   size_t               length)
        {
          int rc = LAPI_SUCCESS;
          LAPIMessage * msg = (LAPIMessage *)state;
          new(msg)LAPIMessage(this->_context,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=0;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=length;
          msg->_p2p_msg._payloadsize1=0;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], payload, length);

          _device.enqueue(msg);
          assert(rc == LAPI_SUCCESS);

          return true;
        };


      template <unsigned T_Niov>
      inline bool postPacket_impl (size_t         target_rank,
                                   void         * metadata,
                                   size_t         metasize,
                                   struct iovec   (&iov)[T_Niov])
        {
          XMI_assert(T_Niov<=2);

          int rc=LAPI_SUCCESS;
          void       * obj = malloc(sizeof(LAPIMessage));
          LAPIMessage * msg = (LAPIMessage *)obj;
          new(msg)LAPIMessage(this->_context,
                             this->_dispatch_id,
                             NULL,
                             0);
          msg->_freeme=1;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=iov[0].iov_len;
          if (T_Niov == 2)
            msg->_p2p_msg._payloadsize1=iov[1].iov_len;
          else
            msg->_p2p_msg._payloadsize1=0;

          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], iov[0].iov_base, iov[0].iov_len);
          if (T_Niov == 2)
            memcpy(&msg->_p2p_msg._payload[iov[0].iov_len], iov[1].iov_base, iov[1].iov_len);
          _device.enqueue(msg);
          assert(rc == LAPI_SUCCESS);
          return true;
        }


      inline bool postMultiPacket_impl (uint8_t              (&state)[LAPIModel::packet_model_status_bytes],
                                        xmi_event_function   fn,
                                        void               * cookie,
                                        size_t               target_rank,
                                        void               * metadata,
                                        size_t               metasize,
                                        void               * payload,
                                        size_t               length)
        {
          int rc = LAPI_SUCCESS;
          LAPIMessage * msg = (LAPIMessage *)malloc(sizeof(LAPIMessage)+metasize+length-DEV_HEADER_SIZE-DEV_PAYLOAD_SIZE);
          new(msg)LAPIMessage(this->_context,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=1;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=length;
          msg->_p2p_msg._payloadsize1=0;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy((char*)(&msg->_p2p_msg._metadata[0])+metasize, payload, length);
          _device.enqueue(msg);
          assert(rc == LAPI_SUCCESS);

          return true;
        };

    protected:
      T_Device                    & _device;
      xmi_client_t                  _client;
      size_t                        _context;
      size_t                        _dispatch_id;
      Interface::RecvFunction_t     _direct_recv_func;
      void                        * _direct_recv_func_parm;
      Interface::RecvFunction_t     _read_recv_func;
      void                        * _read_recv_func_parm;
    };
  };
};
#endif // __components_devices_lapi_lapipacketmodel_h__
