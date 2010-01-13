/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/OldM2MModel.h
 * \brief ???
 */

#ifndef __components_devices_OldM2MModel_h__
#define __components_devices_OldM2MModel_h__

#include "sys/xmi.h"
#include "algorithms/ccmi.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      template <class T_Model, class T_Device, class T_Object, class T_Counter>
      class Oldm2mModel
      {
      public:
        Oldm2mModel (T_Device & device) {};
        ~Oldm2mModel () {};
        inline void setCallback (xmi_olddispatch_manytomany_fn cb_recv, void *arg);

        inline void send  (XMI_Request_t          * request,
                           const xmi_callback_t   * cb_done,
                           unsigned                 connid,
                           unsigned                 rcvindex,
                           const char             * buf,
                           T_Counter               * sizes,
                           T_Counter               * offsets,
                           T_Counter               * counters,
                           T_Counter               * ranks,
                           T_Counter               * permutation,
                           unsigned                 nranks);

        inline void postRecv (XMI_Request_t          * request,
                              const xmi_callback_t   * cb_done,
                              unsigned                 connid,
                              char                   * buf,
                              T_Counter               * sizes,
                              T_Counter               * offsets,
                              T_Counter               * counters,
                              unsigned                 nranks,
                              unsigned                 myindex);
      };



      template <class T_Model, class T_Device, class T_Object, class T_Counter>
      void Oldm2mModel<T_Model, T_Device, T_Object, T_Counter>::setCallback (xmi_olddispatch_manytomany_fn cb_recv, void *arg)
      {
        static_cast<T_Model*>(this)->setCallback_impl(cb_recv, arg);
      }

      template <class T_Model, class T_Device, class T_Object, class T_Counter>
      void Oldm2mModel<T_Model, T_Device, T_Object, T_Counter>::send (XMI_Request_t         * request,
                                                           const xmi_callback_t  * cb_done,
                                                           unsigned                 connid,
                                                           unsigned                 rcvindex,
                                                           const char             * buf,
                                                           T_Counter               * sizes,
                                                           T_Counter               * offsets,
                                                           T_Counter               * counters,
                                                           T_Counter               * ranks,
                                                           T_Counter               * permutation,
                                                           unsigned                 nranks)
      {
        static_cast<T_Model*>(this)->send_impl(request,
                                               cb_done,
                                               connid,
                                               rcvindex,
                                               buf,
                                               sizes,
                                               offsets,
                                               counters,
                                               ranks,
                                               permutation,
                                               nranks);

      }

      template <class T_Model, class T_Device, class T_Object, class T_Counter>
      void Oldm2mModel<T_Model, T_Device, T_Object, T_Counter>::postRecv(XMI_Request_t         * request,
                                                              const xmi_callback_t  * cb_done,
                                                              unsigned                 connid,
                                                              char                   * buf,
                                                              T_Counter               * sizes,
                                                              T_Counter               * offsets,
                                                              T_Counter               * counters,
                                                              unsigned                 nranks,
                                                              unsigned                 myindex)
      {
        static_cast<T_Model*>(this)->postRecv_impl(request,
                                                   cb_done,
                                                   connid,
                                                   buf,
                                                   sizes,
                                                   offsets,
                                                   counters,
                                                   nranks,
                                                   myindex);
      }
    };
  };
};
#endif // __components_device_packetmodel_h__
