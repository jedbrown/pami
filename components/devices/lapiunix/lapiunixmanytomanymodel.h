/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapi/lapimanytomanymodel.h
 * \brief ???
 */

#ifndef __components_devices_lapi_lapimanytomanymodel_h__
#define __components_devices_lapi_lapimanytomanymodel_h__

#include "sys/xmi.h"
#include "components/devices/ManytomanyModel.h"
#include "components/devices/MessageModel.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class LAPIManytomanyModel : public Interface::MessageModel<LAPIManytomanyModel<T_Device, T_Message>,T_Device>
    {
    public:
      LAPIManytomanyModel (T_Device & device) :
        Interface::MessageModel < LAPIManytomanyModel<T_Device, T_Message>, T_Device > (device)
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
