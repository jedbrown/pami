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
 * \file components/devices/lapiunix/lapiunixmanytomanymodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixmanytomanymodel_h__
#define __components_devices_lapiunix_lapiunixmanytomanymodel_h__

#include <pami.h>
#include "components/devices/ManytomanyModel.h"

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class LAPIManytomanyModel :
      public Interface::ManytomanyModel<LAPIManytomanyModel<T_Device, T_Message>,T_Device, sizeof(T_Message)>
    {
      static const size_t manytomany_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg                   = sizeof(T_Message);
    public:
      LAPIManytomanyModel (T_Device & device, pami_result_t &status) :
        Interface::ManytomanyModel < LAPIManytomanyModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device, status)
        {};

      inline void setCallback (pami_dispatch_manytomany_function cb_recv, void *arg)
        {
        }

      inline void send  (pami_manytomany_t parameters)
        {
        }

      inline void postRecv (T_Message              * request,
                            const PAMI_Callback_t   * cb_done,
                            unsigned                 connid,
                            char                   * buf,
                            unsigned               * sizes,
                            unsigned               * offsets,
                            unsigned               * counters,
                            unsigned                 nranks,
                            unsigned                 myindex)
        {
        }
    };
  };
};
#endif // __components_devices_lapi_lapimanytomanymodel_h__
