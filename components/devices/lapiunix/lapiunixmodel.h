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

#ifndef __components_devices_lapi_lapipacketmodel_h__
#define __components_devices_lapi_lapipacketmodel_h__

#include "sys/xmi.h"
#include "components/devices/MessageModel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "errno.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class LAPIModel : public Interface::MessageModel<LAPIModel<T_Device, T_Message>, T_Device, sizeof(T_Message)>
    {
    public:
      LAPIModel (T_Device & device, xmi_context_t context) :
        Interface::MessageModel < LAPIModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device,context),
        _device (device),
        _context(context)
        {};
      static const bool   deterministic_packet_model   = true;
      static const bool   reliable_packet_model        = true;
      static const size_t packet_model_metadata_bytes  = T_Device::metadata_size;
      static const size_t packet_model_payload_bytes   = T_Device::payload_size;
      static const size_t packet_model_status_bytes    = sizeof(T_Message);
      static const size_t packet_model_state_bytes     = sizeof(T_Message);

      static const bool   deterministic_message_model  = true;
      static const bool   reliable_message_model       = true;
      static const size_t message_model_metadata_bytes = T_Device::metadata_size;
      static const size_t message_model_payload_bytes  = T_Device::payload_size;
      static const size_t message_model_status_bytes   = sizeof(T_Message);
      static const size_t message_model_state_bytes     = sizeof(T_Message);

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

      inline bool postPacket_impl (uint8_t              (&state)[LAPIModel::packet_model_status_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t               target_rank,
                                   void               * metadata,
                                   size_t               metasize,
                                   void               * payload,
                                   size_t               bytes)
        {
          int rc;
          LAPIMessage * msg = (LAPIMessage *)state;
          new(msg)LAPIMessage(this->_context,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=0;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=bytes;
          msg->_p2p_msg._payloadsize1=0;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], payload, bytes);



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
                                   void               * payload0,
                                   size_t               bytes0,
                                   void               * payload1,
                                   size_t               bytes1)
        {
          assert(0);
          return false;
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
          assert(0);
          return false;
        };

      inline bool postPacketImmediate (size_t   target_rank,
                                       void   * metadata,
                                       size_t   metasize,
                                       void   * payload0,
                                       size_t   bytes0,
                                       void   * payload1,
                                       size_t   bytes1)
        {
          int rc;
          void       * obj = malloc(sizeof(LAPIMessage));
          LAPIMessage * msg = (LAPIMessage *)obj;
          new(msg)LAPIMessage(this->_context,
                             this->_dispatch_id,
                             NULL,
                             0);
          msg->_freeme=1;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=bytes0;
          msg->_p2p_msg._payloadsize1=bytes1;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], payload0, bytes0);
          memcpy(&msg->_p2p_msg._payload[bytes0], payload1, bytes1);
          _device.enqueue(msg);
          assert(rc == LAPI_SUCCESS);
          return true;
        }


      inline bool postMessage_impl (uint8_t              (&state)[LAPIModel::packet_model_status_bytes],
                                    xmi_event_function   fn,
                                    void               * cookie,
                                    size_t               target_rank,
                                    void               * metadata,
                                    size_t               metasize,
                                    void               * src,
                                    size_t               bytes)
        {
          int rc;
          LAPIMessage * msg = (LAPIMessage *)malloc(sizeof(LAPIMessage)+metasize+bytes-DEV_HEADER_SIZE-DEV_PAYLOAD_SIZE);
          new(msg)LAPIMessage(this->_context,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=1;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=bytes;
          msg->_p2p_msg._payloadsize1=0;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy((char*)(&msg->_p2p_msg._metadata[0])+metasize, src, bytes);
          _device.enqueue(msg);
          assert(rc == LAPI_SUCCESS);

          return true;
        };

    protected:
      T_Device                    & _device;
      xmi_context_t                 _context;
      size_t                        _dispatch_id;
      Interface::RecvFunction_t     _direct_recv_func;
      void                        * _direct_recv_func_parm;
      Interface::RecvFunction_t     _read_recv_func;
      void                        * _read_recv_func_parm;
    };
  };
};
#endif // __components_devices_lapi_lapipacketmodel_h__
