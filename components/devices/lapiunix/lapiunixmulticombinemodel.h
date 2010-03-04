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

namespace XMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
    class LAPIMulticombineModel : public Interface::MulticombineModel<LAPIMulticombineModel<T_Device, T_Message>,sizeof(T_Message)>
        {
        public:
      static const size_t multicombine_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg                     = sizeof(T_Message);

      LAPIMulticombineModel (T_Device & device, xmi_result_t &status) :
        Interface::MulticombineModel < LAPIMulticombineModel<T_Device, T_Message>, sizeof(T_Message) > (status)
            {};
      inline xmi_result_t postMulticombine_impl (uint8_t (&state)[multicombine_model_state_bytes],
                                                 xmi_multicombine_t *mcombine)
                {
              XMI_abort();
              return XMI_SUCCESS;
                }
        };
    };
};
#endif // __components_devices_lapi_lapimulticombinemodel_h__
