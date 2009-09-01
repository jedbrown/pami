#ifndef __components_devices_mpi_mpimulticastmodel_h__
#define __components_devices_mpi_mpimulticastmodel_h__

#include "sys/xmi.h"
#include "components/devices/MulticastModel.h"
#include "components/devices/MessageModel.h"

namespace XMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
        class MPIMulticastModel : public Interface::MessageModel<MPIMulticastModel<T_Device, T_Message>,T_Device, T_Message>
        {
        public:
            MPIMulticastModel (T_Device & device) :
                Interface::MessageModel < MPIMulticastModel<T_Device, T_Message>, T_Device, T_Message > (device)
                {};

            inline void setRequestBuffer (XMI_Request_t *request, size_t req_size)
                {

                }
            inline void setConnectionId (unsigned conn)
                {
                }
            inline void setRoles (unsigned roles)
                {
                }
            inline void setSendData (XMI_PipeWorkQueue_t *src, size_t bytes)
                {
                }
            inline void setSendRanks (XMI_Topology_t *src_participants)
                {
                }
            inline void setRecvData (XMI_PipeWorkQueue_t *dst, size_t bytes)
                {
                }
            inline void setRecvRanks (XMI_Topology_t *dst_participants)
                {
                }
            inline void setCallback (void (*fn) (void *, xmi_error_t *),  void *cd)
                {
                }
            inline void setInfo (XMIQuad *info, int count)
                {
                }
            inline bool postMultiCast (T_Message * obj)
                {
                }
        };
    };
};
#endif // __components_devices_mpi_mpimulticastmodel_h__
