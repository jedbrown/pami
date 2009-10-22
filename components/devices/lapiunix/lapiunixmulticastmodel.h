/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
#include "components/devices/MessageModel.h"

namespace XMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
        class LAPIMulticastModel : public Interface::MessageModel<LAPIMulticastModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
        {
        public:
            LAPIMulticastModel (T_Device & device) :
                Interface::MessageModel < LAPIMulticastModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device)
                {};
            inline void setConnectionId_impl (unsigned conn)
                {
                }
            inline void setRoles_impl (unsigned roles)
                {
                }
            inline void setSendData_impl (xmi_pipeworkqueue_t *src, size_t bytes)
                {
                }
            inline void setSendRanks_impl (xmi_topology_t *src_participants)
                {
                }
            inline void setRecvData_impl (xmi_pipeworkqueue_t *dst, size_t bytes)
                {
                }
            inline void setRecvRanks_impl (xmi_topology_t *dst_participants)
                {
                }
            inline void setCallback_impl (xmi_event_function fn,  void *clientdata)
                {
                }
            inline void setInfo_impl (xmi_quad_t *info, int count)
                {
                }
            inline bool postMultiCast_impl (T_Message * obj)
                {
		  return false;
                }
        };
    };
};
#endif // __components_devices_lapi_lapimulticastmodel_h__
