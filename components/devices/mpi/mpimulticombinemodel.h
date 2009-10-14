/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpimulticombinemodel.h
 * \brief ???
 */

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
        class MPIMulticombineModel : public Interface::MessageModel<MPIMulticombineModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
        {
        public:
            MPIMulticombineModel (T_Device & device) :
              Interface::MessageModel < MPIMulticombineModel<T_Device, T_Message>, T_Device,sizeof(T_Message) > (device)
                {};

            inline void setConnectionId_impl (unsigned conn)
                {
                }
            inline void setRoles_impl (unsigned roles)
                {
                }
            inline void setData_impl (xmi_pipeworkqueue_t *data, size_t count)
                {
                }
            inline void setDataRanks_impl (xmi_topology_t *data_participants)
                {
                }
            inline void setResults_impl (xmi_pipeworkqueue_t *results, size_t count)
                {
                }
            inline void setResultsRanks_impl (xmi_topology_t *results_participants)
                {
                }
            inline void setReduceInfo_impl (xmi_op op,  xmi_dt dt)
                {
                }
            inline void setCallback_impl (void (*fn) (void *, xmi_result_t *),  void *cd)
                {
                }
        };
    };
};
#endif // __components_devices_mpi_mpimulticombinemodel_h__
