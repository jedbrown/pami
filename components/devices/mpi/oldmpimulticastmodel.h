#ifndef __components_devices_mpi_mpioldmulticastmodel_h__
#define __components_devices_mpi_mpioldmulticastmodel_h__

#include "sys/xmi.h"
#include "components/devices/OldMulticastModel.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class MPIOldmulticastModel : public Interface::OldmulticastModel<MPIOldmulticastModel<T_Device, T_Message>, T_Device, T_Message>
    {
    public:
      MPIOldmulticastModel (T_Device & device) :
        Interface::OldmulticastModel<MPIOldmulticastModel<T_Device, T_Message>, T_Device, T_Message> (device)
        {};
              
      inline void setCallback (xmi_olddispatch_multicast_fn cb_recv, void *arg)
        {
        }

      inline unsigned  send   (XMI_Request_t             * request,
                               const xmi_event_function  * cb_done,
                               xmi_consistency_t           consistency,
                               const xmi_quad_t          * info,
                               unsigned                    info_count,
                               unsigned                    connection_id,
                               const char                * buf,
                               unsigned                    size,
                               unsigned                  * hints,
                               unsigned                  * ranks,
                               unsigned                    nranks,
                               xmi_op                      op    = XMI_UNDEFINED_OP,
                               xmi_dt                      dtype = XMI_UNDEFINED_DT )
        {
        }

              
      inline unsigned send (xmi_oldmulticast_t *mcastinfo)
        {
        }


        inline unsigned postRecv (XMI_Request_t          * request,
                                  const xmi_callback_t   * cb_done,
                                  unsigned                 conn_id,
                                  char                   * buf,
                                  unsigned                 size,
                                  unsigned                 pwidth,
                                  unsigned                 hint   = XMI_UNDEFINED_SUBTASK,
                                  xmi_op                   op     = XMI_UNDEFINED_OP,
                                  xmi_dt                   dtype  = XMI_UNDEFINED_DT)
        {
        }

      inline unsigned postRecv (xmi_oldmulticast_recv_t  *mrecv)
        {
        }
    };
  };
};
#endif // __components_devices_mpi_mpioldmulticastmodel_h__
