#ifndef __components_devices_mpi_mpipacketmodel_h__
#define __components_devices_mpi_mpipacketmodel_h__

#include "sys/xmi.h"
#include "components/devices/MessageModel.h"
#include "errno.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class MPIModel : public Interface::MessageModel<MPIModel<T_Device, T_Message>, T_Device, T_Message>
    {
      public:
        MPIModel (T_Device & device) :
            Interface::MessageModel < MPIModel<T_Device, T_Message>, T_Device, T_Message > (device),
            _device (device)
        {};

        bool init_impl (Interface::RecvFunction_t   direct_recv_func,
                        void                      * direct_recv_func_parm,
                        Interface::RecvFunction_t   read_recv_func,
                        void                      * read_recv_func_parm)
        {
          return true;  // <--- fix this
        };

        inline bool postPacket_impl (T_Message          * obj,
                                     xmi_event_function   fn,
                                     void               * cookie,
                                     size_t               target_rank,
                                     void               * metadata,
                                     size_t               metasize,
                                     void               * payload,
                                     size_t               bytes)
        {
          return false;
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
          return false;
        };

        inline bool postMessage_impl (T_Message        * obj,
                                     xmi_event_function   fn,
                                     void               * cookie,
                                      size_t             target_rank,
                                      void             * metadata,
                                      size_t             metasize,
                                      void             * src,
                                      size_t             bytes)
        {
          return false;
        };

      protected:
        T_Device   & _device;
        size_t       _dispatch_id;
    };
  };
};
#endif // __components_devices_mpi_mpipacketmodel_h__
