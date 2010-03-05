/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixmulticastmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixmulticastmodel_h__
#define __components_devices_lapiunix_lapiunixmulticastmodel_h__

#include "sys/xmi.h"
#include "components/devices/MulticastModel.h"

namespace XMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
    class LAPIMulticastModel :
      public Interface::AMMulticastModel<LAPIMulticastModel<T_Device, T_Message>,sizeof(T_Message)>
        {
        public:
      static const size_t mcast_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg              = sizeof(T_Message);
      static const bool   is_active_message       = true;


      LAPIMulticastModel (T_Device & device, xmi_result_t &status) :
        Interface::AMMulticastModel < LAPIMulticastModel<T_Device, T_Message>, sizeof(T_Message) > (status)
                {};

      inline xmi_result_t registerMcastRecvFunction_impl (int                        dispatch_id,
                                                          xmi_dispatch_multicast_fn  recv_func,
                                                          void                      *async_arg)
                {
              return XMI_SUCCESS;
                }


      inline xmi_result_t postMulticast_impl (uint8_t (&state)[mcast_model_state_bytes],
                                              xmi_multicast_t *mcast)
                {
              return XMI_SUCCESS;
                }
        };
    };
};
#endif // __components_devices_lapi_lapimulticastmodel_h__
