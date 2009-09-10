#ifndef __components_devices_mpi_mpimultisyncmodel_h__
#define __components_devices_mpi_mpimultisyncmodel_h__

#include "sys/xmi.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MessageModel.h"

namespace XMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
        class MPIMultisyncModel : public Interface::MessageModel<MPIMultisyncModel<T_Device, T_Message>,T_Device, T_Message>
        {
        public:
            MPIMultisyncModel (T_Device & device) :
                Interface::MessageModel < MPIMultisyncModel<T_Device, T_Message>, T_Device, T_Message > (device)
                {};

            inline bool postMultisync_impl (T_Message * obj)
                {

                }
            
        };
    };
};
#endif // __components_devices_mpi_mpimultisyncmodel_h__
