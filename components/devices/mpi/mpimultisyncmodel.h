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

            inline bool postMultisync_impl (T_Object * obj)
                {

                }
            
        };
    };
};
#endif // __components_devices_mpi_mpimulticombinemodel_h__
