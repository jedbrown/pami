/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/oldmpim2mmodel.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpioldm2mmodel_h__
#define __components_devices_mpi_mpioldm2mmodel_h__

#include "sys/xmi.h"
#include "components/devices/OldM2MModel.h"
#include "components/devices/mpi/mpimessage.h"
#include <mpi.h>
#include "util/common.h"
namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message, class T_Counter>
    class MPIOldm2mModel : public Interface::Oldm2mModel<MPIOldm2mModel<T_Device, T_Message, T_Counter>, T_Device, T_Message, T_Counter>
    {
    public:
      MPIOldm2mModel (T_Device & device) :
        Interface::Oldm2mModel<MPIOldm2mModel<T_Device, T_Message, T_Counter>, T_Device, T_Message, T_Counter> (device),
        _device(device)
        {
//          assert(0);
        };
        
      inline void setCallback (xmi_olddispatch_manytomany_fn cb_recv, void *arg)
        {
          assert(0);
        }

      inline void  send_impl  (XMI_Request_t         * request,
                               const xmi_callback_t  * cb_done,
                               unsigned                connid,
                               unsigned                rcvindex,
                               const char            * buf,
                               T_Counter              * sizes,
                               T_Counter              * offsets,
                               T_Counter              * counters,
                               T_Counter              * ranks,
                               T_Counter              * permutation,
                               unsigned                nranks)
        {
          assert(0);
          return;
        }
      
      inline void postRecv_impl (XMI_Request_t          * request,
                                 const xmi_callback_t   * cb_done,
                                 unsigned                 connid,
                                 char                   * buf,
                                 T_Counter               * sizes,
                                 T_Counter               * offsets,
                                 T_Counter               * counters,
                                 unsigned                 nranks,
                                 unsigned                 myindex)
        {
          assert(0);
          return;
        }
      T_Device                     &_device;
      size_t                        _dispatch_id;
      xmi_olddispatch_manytomany_fn _cb_async_head;
      void                         *_async_arg;

    };
  };
};
#endif // __components_devices_mpi_mpioldm2mmodel_h__
