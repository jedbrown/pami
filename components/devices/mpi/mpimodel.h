/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpimodel.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpipacketmodel_h__
#define __components_devices_mpi_mpipacketmodel_h__

#include "sys/xmi.h"
#include "components/devices/MessageModel.h"
#include "components/devices/mpi/mpimessage.h"
#include "errno.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class MPIModel : public Interface::MessageModel<MPIModel<T_Device, T_Message>, T_Device, T_Message>
    {
    public:
      MPIModel (T_Device & device, xmi_context_t context) :
        Interface::MessageModel < MPIModel<T_Device, T_Message>, T_Device, T_Message > (device,context),
        _device (device),
        _context(context)
        {};

      xmi_result_t init_impl (Interface::RecvFunction_t   direct_recv_func,
                              void                      * direct_recv_func_parm,
                              Interface::RecvFunction_t   read_recv_func,
                              void                      * read_recv_func_parm)
        {
          _dispatch_id = _device.registerRecvFunction (direct_recv_func, direct_recv_func_parm);
          return XMI_SUCCESS;
        };

      inline bool isDeterministic ()
        {
          return true;
        }

      inline bool postPacket_impl (T_Message          * obj,
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t               target_rank,
                                   void               * metadata,
                                   size_t               metasize,
                                   void               * payload,
                                   size_t               bytes)
        {
          int rc;
          MPIMessage * msg = (MPIMessage *)obj;
          new(msg)MPIMessage(this->_context,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize =bytes;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], payload, bytes);
          rc = MPI_Isend (&msg->_p2p_msg,
                          sizeof(msg->_p2p_msg),
                          MPI_CHAR,
                          target_rank,
                          0,
                          MPI_COMM_WORLD,
                          &msg->_request);
          _device.enqueue(msg);
          assert(rc == MPI_SUCCESS);

          return true;
        };

      inline bool postPacket_impl (T_Message          * obj,
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t               target_rank,
                                   void               * metadata,
                                   size_t               metasize,
                                   void               * payload0,
                                   size_t               bytes0,
                                   void               * payload1,
                                   size_t               bytes1)
        {
          assert(0);
          return false;
        };

      inline bool postPacket_impl (T_Message        * obj,
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t             target_rank,
                                   void             * metadata,
                                   size_t             metasize,
                                   struct iovec_t   * iov,
                                   size_t             niov)
        {
          assert(0);
          return false;
        };

      inline bool postPacketImmediate (size_t   target_rank,
                                       void   * metadata,
                                       size_t   metasize,
                                       void   * payload0,
                                       size_t   bytes0,
                                       void   * payload1,
                                       size_t   bytes1)
        {
          assert(0);
          return false;
        }
      

      inline bool postMessage_impl (T_Message        * obj,
                                    xmi_event_function   fn,
                                    void               * cookie,
                                    size_t             target_rank,
                                    void             * metadata,
                                    size_t             metasize,
                                    void             * src,
                                    size_t             bytes)
        {
          assert(0);
          return false;
        };

    protected:
      T_Device                    & _device;
      xmi_context_t                 _context;
      size_t                        _dispatch_id;
      Interface::RecvFunction_t     _direct_recv_func;
      void                        * _direct_recv_func_parm;
      Interface::RecvFunction_t     _read_recv_func;
      void                        * _read_recv_func_parm;
    };
  };
};
#endif // __components_devices_mpi_mpipacketmodel_h__
