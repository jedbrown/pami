/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixmanytomanymodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixmanytomanymodel_h__
#define __components_devices_lapiunix_lapiunixmanytomanymodel_h__

#include "sys/xmi.h"
#include "components/devices/ManytomanyModel.h"

namespace XMI
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
      LAPIManytomanyModel (T_Device & device, xmi_result_t &status) :
        Interface::ManytomanyModel < LAPIManytomanyModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device, status)
        {};

      inline void setCallback (xmi_dispatch_manytomany_fn cb_recv, void *arg)
        {
        }

      inline void send  (xmi_manytomany_t parameters)
        {
        }

      inline void postRecv (T_Message              * request,
                            const XMI_Callback_t   * cb_done,
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
