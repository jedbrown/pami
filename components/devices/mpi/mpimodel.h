/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
         TRACE_ADAPTOR((stderr,"<%#.8X>MPIModel::init_impl %d \n",(int)this, _dispatch_id));
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
          TRACE_ADAPTOR((stderr,"<%#.8X>MPIModel::postPacket_impl %d \n",(int)this, this->_dispatch_id));
          new(msg)MPIMessage(this->_context,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=0;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=bytes;
          msg->_p2p_msg._payloadsize1=0;
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
          int rc;
          void       * obj = malloc(sizeof(MPIMessage));
          TRACE_ADAPTOR((stderr,"<%#.8X>MPIModel::postPacketImmediate %d \n",(int)this, this->_dispatch_id));
          MPIMessage * msg = (MPIMessage *)obj;
          new(msg)MPIMessage(this->_context,
                             this->_dispatch_id,
                             NULL,
                             0);
          msg->_freeme=1;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=bytes0;
          msg->_p2p_msg._payloadsize1=bytes1;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], payload0, bytes0);
          memcpy(&msg->_p2p_msg._payload[bytes0], payload1, bytes1);
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
          int rc;
          TRACE_ADAPTOR((stderr,"<%#.8X>MPIModel::postMessage_impl %d \n",(int)this, this->_dispatch_id));
          MPIMessage * msg = (MPIMessage *)malloc(sizeof(MPIMessage)+metasize+bytes-128-224);
          new(msg)MPIMessage(this->_context,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=1;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=bytes;
          msg->_p2p_msg._payloadsize1=0;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy((char*)(&msg->_p2p_msg._metadata[0])+metasize, src, bytes);
          rc = MPI_Isend (&msg->_p2p_msg,
                          sizeof(msg->_p2p_msg)+metasize+bytes-128-224,
                          MPI_CHAR,
                          target_rank,
                          1,
                          MPI_COMM_WORLD,
                          &msg->_request);
          _device.enqueue(msg);
          assert(rc == MPI_SUCCESS);

          return true;
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
