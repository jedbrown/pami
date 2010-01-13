/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixmultisyncmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixmultisyncmodel_h__
#define __components_devices_lapiunix_lapiunixmultisyncmodel_h__

#include "sys/xmi.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MessageModel.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class LAPIMultisyncModel : public Interface::MessageModel<LAPIMultisyncModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
    {
    public:
      LAPIMultisyncModel (T_Device & device) :
        Interface::MessageModel < LAPIMultisyncModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device)
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
