/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/OldMulticastModel.h
 * \brief ???
 */

#ifndef __components_devices_OldMulticastModel_h__
#define __components_devices_OldMulticastModel_h__

#include "sys/pami.h"
#include "algorithms/ccmi.h"

namespace PAMI
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

        inline void setCallback (pami_olddispatch_multicast_fn cb_recv, void *arg);
        inline unsigned  send   (PAMI_Request_t            * request,
                                 const pami_callback_t     * cb_done,
                                 pami_consistency_t            consistency,
                                 const pami_quad_t          * info,
                                 unsigned                    info_count,
                                 unsigned                    connection_id,
                                 const char                * buf,
                                 unsigned                    size,
                                 unsigned                  * hints,
                                 unsigned                  * ranks,
                                 unsigned                    nranks,
                                 pami_op                      op    = PAMI_UNDEFINED_OP,
                                 pami_dt                      dtype = PAMI_UNDEFINED_DT );

        inline unsigned send (pami_oldmulticast_t  *mcastinfo);

        inline unsigned postRecv (PAMI_Request_t          * request,
                                  const pami_callback_t   * cb_done,
                                  unsigned                 conn_id,
                                  char                   * buf,
                                  unsigned                 size,
                                  unsigned                 pwidth,
                                  unsigned                 hint   = PAMI_UNDEFINED_SUBTASK,
                                  pami_op                   op     = PAMI_UNDEFINED_OP,
                                  pami_dt                   dtype  = PAMI_UNDEFINED_DT);
        inline unsigned postRecv (pami_oldmulticast_recv_t  *mrecv);
      };



      template <class T_Model, class T_Device, class T_Object>
      void OldmulticastModel<T_Model, T_Device, T_Object>::setCallback (pami_olddispatch_multicast_fn cb_recv, void *arg)
      {
        static_cast<T_Model*>(this)->setCallback_impl(cb_recv, arg);
      }

      template <class T_Model, class T_Device, class T_Object>
      unsigned OldmulticastModel<T_Model, T_Device, T_Object>::send (PAMI_Request_t             * request,
                                                                     const pami_callback_t      * cb_done,
                                                                     pami_consistency_t           consistency,
                                                                     const pami_quad_t          * info,
                                                                     unsigned                    info_count,
                                                                     unsigned                    connection_id,
                                                                     const char                * buf,
                                                                     unsigned                    size,
                                                                     unsigned                  * hints,
                                                                     unsigned                  * ranks,
                                                                     unsigned                    nranks,
                                                                     pami_op                      op,
                                                                     pami_dt                      dtype)
      {
        return static_cast<T_Model*>(this)->send_impl(request, cb_done, consistency, info, info_count,
                                               connection_id, buf, size, hints, ranks, nranks,
                                               op, dtype);
      }

      template <class T_Model, class T_Device, class T_Object>
      unsigned OldmulticastModel<T_Model, T_Device, T_Object>::postRecv(PAMI_Request_t          * request,
                                                                        const pami_callback_t   * cb_done,
                                                                        unsigned                 conn_id,
                                                                        char                   * buf,
                                                                        unsigned                 size,
                                                                        unsigned                 pwidth,
                                                                        unsigned                 hint,
                                                                        pami_op                   op,
                                                                        pami_dt                   dtype)
      {
        return static_cast<T_Model*>(this)->postRecv_impl(request, cb_done, conn_id, buf,
                                                          size, pwidth,hint, op, dtype);
      }

      template <class T_Model, class T_Device, class T_Object>
      unsigned OldmulticastModel<T_Model, T_Device, T_Object>::postRecv(pami_oldmulticast_recv_t *mrecv)
      {
       return static_cast<T_Model*>(this)->postRecv_impl(mrecv);
      }
    };
  };
};
#endif // __components_device_packetmodel_h__
