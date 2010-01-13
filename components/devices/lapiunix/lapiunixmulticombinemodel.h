/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixmulticombinemodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixmulticombinemodel_h__
#define __components_devices_lapiunix_lapiunixmulticombinemodel_h__

#include "sys/xmi.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/MessageModel.h"

namespace XMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
        class LAPIMulticombineModel : public Interface::MessageModel<LAPIMulticombineModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
        {
        public:
            LAPIMulticombineModel (T_Device & device) :
                Interface::MessageModel < LAPIMulticombineModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device)
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
#endif // __components_devices_lapi_lapimulticombinemodel_h__
