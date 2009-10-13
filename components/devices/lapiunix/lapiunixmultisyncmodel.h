/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapi/lapimultisyncmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapi_lapimultisyncmodel_h__
#define __components_devices_lapi_lapimultisyncmodel_h__

#include "sys/xmi.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MessageModel.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class LAPIMultisyncModel : public Interface::MessageModel<LAPIMultisyncModel<T_Device, T_Message>,T_Device>
    {
    public:
      LAPIMultisyncModel (T_Device & device) :
        Interface::MessageModel < LAPIMultisyncModel<T_Device, T_Message>, T_Device > (device)
        {};
      inline void setConnectionId (unsigned conn)
        {
        }
      inline void setRoles(unsigned roles)
        {
        }
      inline void setRanks(xmi_topology_t *participants)
        {
        }
      inline void setCallback(xmi_callback_t &cb_done)
        {
        }
      inline bool postMultisync_impl (T_Message * obj)
        {
	  return false;
        }
    };
  };
};
#endif // __components_devices_lapi_lapimultisyncmodel_h__
