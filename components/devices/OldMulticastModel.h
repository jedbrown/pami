#ifndef __components_device_Oldmulticastmodel_h__
#define __components_device_Oldmulticastmodel_h__

#include "sys/xmi.h"
#include "algorithms/ccmi.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      template <class T_Model, class T_Device, class T_Object>
      class OldmulticastModel
      {
      public:
        OldmulticastModel (T_Device & device) {};
        ~OldmulticastModel () {};
              
        inline void setCallback (xmi_olddispatch_multicast_fn cb_recv, void *arg);
        inline unsigned  send   (XMI_Request_t            * request,
                                 const xmi_callback_t     * cb_done,
                                 xmi_consistency_t            consistency,
                                 const xmi_quad_t          * info,
                                 unsigned                    info_count,
                                 unsigned                    connection_id,
                                 const char                * buf,
                                 unsigned                    size,
                                 unsigned                  * hints,
                                 unsigned                  * ranks,
                                 unsigned                    nranks,
                                 xmi_op                      op    = XMI_UNDEFINED_OP,
                                 xmi_dt                      dtype = XMI_UNDEFINED_DT );
              
        inline unsigned send (xmi_oldmulticast_t  *mcastinfo);

        inline unsigned postRecv (XMI_Request_t          * request,
                                  const xmi_callback_t   * cb_done,
                                  unsigned                 conn_id,
                                  char                   * buf,
                                  unsigned                 size,
                                  unsigned                 pwidth,
                                  unsigned                 hint   = XMI_UNDEFINED_SUBTASK,
                                  xmi_op                   op     = XMI_UNDEFINED_OP,
                                  xmi_dt                   dtype  = XMI_UNDEFINED_DT);
        inline unsigned postRecv (xmi_oldmulticast_recv_t  *mrecv);
      };


      
      template <class T_Model, class T_Device, class T_Object>
      void OldmulticastModel<T_Model, T_Device, T_Object>::setCallback (xmi_olddispatch_multicast_fn cb_recv, void *arg)
      {
        static_cast<T_Model*>(this)->setCallback_impl(cb_recv, arg);
      }
      
      template <class T_Model, class T_Device, class T_Object>
      unsigned OldmulticastModel<T_Model, T_Device, T_Object>::send (XMI_Request_t             * request,
                                                                     const xmi_callback_t      * cb_done,
                                                                     xmi_consistency_t           consistency,
                                                                     const xmi_quad_t          * info,
                                                                     unsigned                    info_count,
                                                                     unsigned                    connection_id,
                                                                     const char                * buf,
                                                                     unsigned                    size,
                                                                     unsigned                  * hints,
                                                                     unsigned                  * ranks,
                                                                     unsigned                    nranks,
                                                                     xmi_op                      op,
                                                                     xmi_dt                      dtype)
      {
        return static_cast<T_Model*>(this)->send_impl(request, cb_done, consistency, info, info_count,
                                               connection_id, buf, size, hints, ranks, nranks,
                                               op, dtype);
      }

      template <class T_Model, class T_Device, class T_Object>
      unsigned OldmulticastModel<T_Model, T_Device, T_Object>::postRecv(XMI_Request_t          * request,
                                                                        const xmi_callback_t   * cb_done,
                                                                        unsigned                 conn_id,
                                                                        char                   * buf,
                                                                        unsigned                 size,
                                                                        unsigned                 pwidth,
                                                                        unsigned                 hint,
                                                                        xmi_op                   op,
                                                                        xmi_dt                   dtype)
      {
        return static_cast<T_Model*>(this)->postRecv_impl(request, cb_done, conn_id, buf,
                                                          size, pwidth,hint, op, dtype);
      }

      template <class T_Model, class T_Device, class T_Object>
      unsigned OldmulticastModel<T_Model, T_Device, T_Object>::postRecv(xmi_oldmulticast_recv_t *mrecv)
      {
       return static_cast<T_Model*>(this)->postRecv_impl(mrecv);
      }
    };
  };
};
#endif // __components_device_packetmodel_h__
