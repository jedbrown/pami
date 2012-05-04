/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixmulticastmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixmulticastmodel_h__
#define __components_devices_lapiunix_lapiunixmulticastmodel_h__

#include <pami.h>
#include "components/devices/MulticastModel.h"

namespace PAMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
    class LAPIMulticastModel :
      public Interface::AMMulticastModel<LAPIMulticastModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
        {
        public:
      static const size_t mcast_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg              = sizeof(T_Message);
      static const bool   is_active_message       = true;


      LAPIMulticastModel (T_Device & device, pami_result_t &status) :
        Interface::AMMulticastModel < LAPIMulticastModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device, status)
                {};

      inline pami_result_t registerMcastRecvFunction_impl (int                        dispatch_id,
                                                          pami_dispatch_multicast_function recv_func,
                                                          void                      *async_arg)
                {
              return PAMI_SUCCESS;
                }


      inline pami_result_t postMulticast_impl (uint8_t (&state)[mcast_model_state_bytes],
					       size_t           client,
					       size_t           context, 
                                              pami_multicast_t *mcast)
                {
              return PAMI_SUCCESS;
                }
        };
    };
};
#endif // __components_devices_lapi_lapimulticastmodel_h__
