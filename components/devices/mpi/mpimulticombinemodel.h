#ifndef __components_devices_mpi_mpimulticombinemodel_h__
#define __components_devices_mpi_mpimulticombinemodel_h__

#include "sys/xmi.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/MessageModel.h"

namespace XMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
        class MPIMulticombineModel : public Interface::MessageModel<MPIMulticombineModel<T_Device, T_Message>,T_Device, T_Message>
        {
        public:
            MPIMulticombineModel (T_Device & device) :
                Interface::MessageModel < MPIMulticombineModel<T_Device, T_Message>, T_Device, T_Message > (device)
                {};

            inline void setRequestBuffer(XMI_Request_t *request, size_t req_size)
                {
                }
            inline void setConnectionId (unsigned conn)
                {
                }
            inline void setRoles (unsigned roles)
                {
                }
            inline void setData (XMI_PipeWorkQueue_t *data, size_t count)
                {
                }
            inline void setDataRanks (XMI_Topology_t *data_participants)
                {
                }
            inline void setResults (XMI_PipeWorkQueue_t *results, size_t count)
                {
                }
            inline void setResultsRanks (XMI_Topology_t *results_participants)
                {
                }
            inline void setReduceInfo (xmi_op op,  xmi_dt dt)
                {
                }
            inline void setCallback (void (*fn) (void *, xmi_error_t *),  void *cd)
                {
                }
        };
    };
};
#endif // __components_devices_mpi_mpimulticombinemodel_h__
