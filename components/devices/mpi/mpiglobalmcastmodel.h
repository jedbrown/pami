/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpiglobalmulticastmodel.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpiglobalmulticastmodel_h__
#define __components_devices_mpi_mpiglobalmulticastmodel_h__

#include "sys/xmi.h"
#include "Global.h"
#include "components/devices/MulticastModel.h"

#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif
namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class MPIGlobalMcastModel : public XMI::Device::Interface::MulticastModel<MPIGlobalMcastModel<T_Device, T_Message> >
    {
    public:
      static const size_t sizeof_msg = sizeof(T_Message);
      MPIGlobalMcastModel(size_t dispatch_id,       
                          xmi_dispatch_multicast_fn     dispatch,
                          T_Device &device) :
      Interface::MulticastModel<MPIGlobalMcastModel<T_Device, T_Message> >(_status),
      _device(device),
      _peer(__global.topology_local.rank2Index(__global.mapping.task())),
      _npeers(__global.topology_local.size()),
      _dispatch(dispatch)
      {

        _dispatch_id = device.initMcast();
        TRACE_DEVICE((stderr, "<%#.8X>MPIGlobalMcastModel() dispatch_id %zd\n", (int)this, _dispatch_id));        
/* 
         XMI::Device::Interface::RecvFunction_t   recv_func;
         void                      * recv_func_parm;
        _dispatch_id = device.registerRecvFunction (dispatch_id,
                                recv_func,
                                recv_func_parm);
        TRACE_DEVICE((stderr, "<%#.8X>MPIGlobalMcastModel() dispatch_id %zd, _device_dispatch_id %zd\n", (int)this, dispatch_id, _dispatch_id));
*/
      }

      inline void reset_impl()
      {
      }

      inline void setConnectionId_impl (unsigned conn)
      {
      }
      inline void setRoles_impl (unsigned roles)
      {
      }
      inline void setSendData_impl (xmi_pipeworkqueue_t *src, size_t bytes)
      {
      }
      inline void setSendRanks_impl (xmi_topology_t *src_participants)
      {
      }
      inline void setRecvData_impl (xmi_pipeworkqueue_t *dst, size_t bytes)
      {
      }
      inline void setRecvRanks_impl (xmi_topology_t *dst_participants)
      {
      }
      inline void setCallback_impl (xmi_event_function fn,  void *clientdata)
      {
      }
      inline void setInfo_impl (xmi_quad_t *info, int count)
      {
      }
      inline bool postMulticast_impl(xmi_multicast_t *mcast)
      {
        T_Message *msg = new // (mcast->request) \todo memory allocator?  templatize size?
                         T_Message(mcast,_dispatch_id);
        _device.enqueue(msg);
        TRACE_DEVICE((stderr, "<%#.8X>MPIGlobalMcastModel() postMulticast_impl dispatch_id %zd, msg %p\n", (int)this, _dispatch_id, msg));
        return true;
      };
    private:
      xmi_result_t    _status;
      T_Device      & _device;
      size_t          _dispatch_id;
      unsigned        _peer;
      unsigned        _npeers;
      xmi_dispatch_multicast_fn     _dispatch;
    }; // class MPIGlobalMcastModel

  };
};

#endif // __components_devices_mpi_mpiglobalmulticastmodel_h__


