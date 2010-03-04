/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpimanytomanymodel.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpimanytomanymodel_h__
#define __components_devices_mpi_mpimanytomanymodel_h__

#include "sys/xmi.h"
#include "components/devices/ManytomanyModel.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class MPIManytomanyModel : public Interface::ManytomanyModel<MPIManytomanyModel<T_Device, T_Message>,sizeof(T_Message)>
    {
    public:

      static const size_t manytomany_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg                   = sizeof(T_Message);

      MPIManytomanyModel (T_Device & device, xmi_result_t &status) :
        Interface::ManytomanyModel < MPIManytomanyModel<T_Device, T_Message>, sizeof(T_Message)> (status)
        {};

      inline void setCallback (xmi_dispatch_manytomany_fn cb_recv, void *arg)
        {
        }

      inline void send  (xmi_manytomany_t parameters)
        {
        }

      inline void postRecv (T_Message              * request,
                            const XMI_Callback_t   * cb_done,
                            unsigned                 connid,
                            char                   * buf,
                            unsigned               * sizes,
                            unsigned               * offsets,
                            unsigned               * counters,
                            unsigned                 nranks,
                            unsigned                 myindex)
        {
        }
    };
  };
};
#endif // __components_devices_mpi_mpimanytomanymodel_h__
