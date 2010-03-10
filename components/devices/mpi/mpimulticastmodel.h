/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpimulticastmodel.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpimulticastmodel_h__
#define __components_devices_mpi_mpimulticastmodel_h__

#include "sys/xmi.h"
#include "components/devices/MulticastModel.h"
#include "components/devices/mpi/mpimessage.h"

namespace XMI
{
    namespace Device
    {
        template <class T_Device, class T_Message>
    class MPIMulticastModel :
      public Interface::AMMulticastModel<MPIMulticastModel<T_Device, T_Message>,T_Device, sizeof(T_Message)>
        {
        public:
      static const size_t mcast_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg              = sizeof(T_Message);
      static const bool   is_active_message       = true;


      MPIMulticastModel (T_Device & device, xmi_result_t &status) :
        Interface::AMMulticastModel < MPIMulticastModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device, status),
        _device(device)
                {
          status = XMI_SUCCESS;
        };

      inline xmi_result_t registerMcastRecvFunction_impl (int                        dispatch_id,
                                                          xmi_dispatch_multicast_fn  recv_func,
                                                          void                      *async_arg)
                {
          _device.registerMcastRecvFunction (dispatch_id,recv_func, async_arg);
          return XMI_SUCCESS;
                }

      inline xmi_result_t postMulticast_impl (uint8_t (&state)[mcast_model_state_bytes],
                                              xmi_multicast_t *mcast)
                {
          xmi_result_t      rc         = XMI_SUCCESS;
          MPIMcastMessage  *msg        = (MPIMcastMessage *) state;
          unsigned          myrank     = __global.mapping.task();
          msg->_cb_done                = mcast->cb_done;
          msg->_p2p_msg._connection_id = mcast->connection_id;
          msg->_srcranks               = (XMI::Topology*)mcast->src_participants;
          msg->_dstranks               = (XMI::Topology*)mcast->src_participants;
          msg->_srcpwq                 = (XMI::PipeWorkQueue*)mcast->src;
          msg->_dstpwq                 = (XMI::PipeWorkQueue*)mcast->dst;
          msg->_root                   = msg->_srcranks->index2Rank(0);
          msg->_bytes                  = mcast->bytes;


          XMI_abort();

          if(msg->_dstpwq)
                {
                msg->_dstranks->rankList(&msg->_ranks);
                msg->_numRanks = msg->_dstranks->size();
                }

          if(msg->_srcpwq)
                {
                msg->_currBytes = msg->_srcpwq->bytesAvailableToConsume();
                msg->_currBuf   = msg->_srcpwq->bufferToConsume();
                }

          if(msg->_root == myrank)
                {

                }
	  return rc;
                }
      T_Device                  &_device;
      xmi_dispatch_multicast_fn  _cb_async_head;
      void                      *_async_arg;
        };
    };
};
#endif // __components_devices_mpi_mpimulticastmodel_h__
