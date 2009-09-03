#ifndef __components_devices_mpi_mpioldmulticastmodel_h__
#define __components_devices_mpi_mpioldmulticastmodel_h__

#include "sys/xmi.h"
#include "components/devices/OldmulticastModel.h"
#include "components/devices/MessageModel.h"

namespace XMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
        class MPIOldmulticastModel : public Interface::MessageModel<MPIOldmulticastModel<T_Device, T_Message>,T_Device, T_Message>
        {
        public:
            MPIOldmulticastModel (T_Device & device) :
                Interface::MessageModel < MPIOldmulticastModel<T_Device, T_Message>, T_Device, T_Message > (device)
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
            inline void setCallback_impl (void (*fn) (void *, xmi_result_t *),  void *cd)
                {
                }
            inline void setInfo_impl (xmi_quad_t *info, int count)
                {
                }
            inline bool postmulticast_impl (T_Message * obj)
                {
                }
        };
    };
};
#endif // __components_devices_mpi_mpioldmulticastmodel_h__
