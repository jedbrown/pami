/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixpacketmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixpacketmodel_h__
#define __components_devices_lapiunix_lapiunixpacketmodel_h__

#define USE_GCC_ICE_WORKAROUND

#include <pami.h>
#include "components/devices/PacketInterface.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "errno.h"
#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
#ifdef USE_GCC_ICE_WORKAROUND
    class LAPIPacketModel : public Interface::PacketModel<LAPIPacketModel<T_Device, T_Message>, T_Device, 512>
#else // USE_GCC_ICE_WORKAROUND
    class LAPIPacketModel : public Interface::PacketModel<LAPIPacketModel<T_Device, T_Message>, T_Device, sizeof(T_Message)>
#endif // USE_GCC_ICE_WORKAROUND
    {
    public:
      LAPIPacketModel (T_Device & device) :
#ifdef USE_GCC_ICE_WORKAROUND
        Interface::PacketModel < LAPIPacketModel<T_Device, T_Message>, T_Device, 512 > (device),
#else // USE_GCC_ICE_WORKAROUND
        Interface::PacketModel < LAPIPacketModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device),
#endif // USE_GCC_ICE_WORKAROUND
        _device (device),
//        _client(client),
        _context (device.getContext()),
        _contextid (device.getContextOffset())
        {};

      static const bool   deterministic_packet_model   = true;
      static const bool   deterministic_message_model  = true;
      static const bool   reliable_packet_model        = true;
      static const bool   reliable_message_model       = true;
      static const size_t packet_model_metadata_bytes       = T_Device::metadata_size;
      static const size_t packet_model_multi_metadata_bytes = T_Device::metadata_size;
      static const size_t packet_model_payload_bytes        = T_Device::payload_size;
      static const size_t packet_model_immediate_bytes      = 0;

#ifdef USE_GCC_ICE_WORKAROUND
      static const size_t packet_model_state_bytes          = 512;
#else // USE_GCC_ICE_WORKAROUND
      static const size_t packet_model_state_bytes          = sizeof(T_Message);
#endif // USE_GCC_ICE_WORKAROUND

      pami_result_t init_impl (size_t                      dispatch,
                              Interface::RecvFunction_t   direct_recv_func,
                              void                      * direct_recv_func_parm,
                              Interface::RecvFunction_t   read_recv_func,
                              void                      * read_recv_func_parm)
        {
#ifdef USE_GCC_ICE_WORKAROUND
          COMPILE_TIME_ASSERT(sizeof(T_Message) <= 512);
#endif // USE_GCC_ICE_WORKAROUND
          return PAMI_SUCCESS;
        };

      inline bool postPacket_impl (uint8_t              (&state)[LAPIPacketModel::packet_model_state_bytes],
                                   pami_event_function   fn,
                                   void               * cookie,
                                   pami_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec_t     * iov,
                                   size_t               niov)
        {
          PAMI_abort();
          return false;
        };

      template <unsigned T_Niov>
      inline bool postPacket_impl (uint8_t              (&state)[LAPIPacketModel::packet_model_state_bytes],
                                   pami_event_function   fn,
                                   void               * cookie,
                                   pami_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec         (&iov)[T_Niov])
        {
          PAMI_abort();
          return false;
        };


      inline bool postPacket_impl (uint8_t              (&state)[LAPIPacketModel::packet_model_state_bytes],
                                   pami_event_function   fn,
                                   void               * cookie,
                                   pami_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec         (&iov)[1])
        {
          PAMI_abort();
          return true;
        };

      inline bool postPacket_impl (uint8_t              (&state)[LAPIPacketModel::packet_model_state_bytes],
                                   pami_event_function   fn,
                                   void               * cookie,
                                   pami_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec         (&iov)[2])
        {
          PAMI_abort();
          return true;
        };

      inline bool postPacket_impl (uint8_t              (&state)[LAPIPacketModel::packet_model_state_bytes],
                                   pami_event_function   fn,
                                   void               * cookie,
                                   pami_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   void               * payload,
                                   size_t               length)
        {
          PAMI_abort();
          return true;
        };



      template <unsigned T_Niov>
      inline bool postPacket_impl (pami_task_t     target_task,
                                   size_t         target_offset,
                                   void         * metadata,
                                   size_t         metasize,
                                   struct iovec   (&iov)[T_Niov])
        {
          PAMI_abort();
          return true;
        }


      inline bool postMultiPacket_impl (uint8_t              (&state)[LAPIPacketModel::packet_model_state_bytes],
                                        pami_event_function   fn,
                                        void               * cookie,
                                        pami_task_t           target_task,
                                        size_t               target_offset,
                                        void               * metadata,
                                        size_t               metasize,
                                        void               * payload,
                                        size_t               length)
        {
          PAMI_abort();
          return true;
        };

    protected:
      T_Device                    & _device;
      pami_client_t                  _client;
      pami_context_t                 _context;
      size_t                        _contextid;
      size_t                        _dispatch_id;
      Interface::RecvFunction_t     _direct_recv_func;
      void                        * _direct_recv_func_parm;
      Interface::RecvFunction_t     _read_recv_func;
      void                        * _read_recv_func_parm;
    };
  };
};
#endif // __components_devices_lapi_lapipacketmodel_h__
