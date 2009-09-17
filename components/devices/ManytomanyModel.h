/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/ManytomanyModel.h
 * \brief ???
 */

#ifndef __components_device_manytomanymodel_h__
#define __components_device_manytomanymodel_h__

#include "sys/xmi.h"
#include "algorithms/ccmi.h"  //for XMI_Callback_t...probably move this at some point
namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      template <class T_Model, class T_Device, class T_Object>
      class ManytomanyModel
      {
      public:
        /// \param[in] device                Manytomany device reference
        ManytomanyModel (T_Device & device) {};
        ~ManytomanyModel () {};

        inline void setCallback (xmi_dispatch_manytomany_fn cb_recv, void *arg);
        inline void send  (xmi_manytomany_t parameters);
        inline void postRecv (T_Object              * request,
                              const XMI_Callback_t  * cb_done,
                              unsigned                 connid,
                              char                   * buf,
                              unsigned               * sizes,
                              unsigned               * offsets,
                              unsigned               * counters,
                              unsigned                 nranks,
                              unsigned                 myindex);
      };
      template <class T_Model, class T_Device, class T_Object>
      void ManytomanyModel<T_Model, T_Device, T_Object>::setCallback (xmi_dispatch_manytomany_fn cb_recv,
                                                                      void *arg)
      {
        static_cast<T_Model*>(this)->setCallback_impl(cb_recv, arg);
      }

      template <class T_Model, class T_Device, class T_Object>
      void ManytomanyModel<T_Model, T_Device, T_Object>::send (xmi_manytomany_t parameters)
      {
        static_cast<T_Model*>(this)->send_impl(parameters);
      }

      template <class T_Model, class T_Device, class T_Object>
      void ManytomanyModel<T_Model, T_Device, T_Object>::postRecv (T_Object              * request,
                                                                   const XMI_Callback_t  * cb_done,
                                                                   unsigned                connid,
                                                                   char                  * buf,
                                                                   unsigned              * sizes,
                                                                   unsigned              * offsets,
                                                                   unsigned              * counters,
                                                                   unsigned                nranks,
                                                                   unsigned                myindex)
      {
        static_cast<T_Model*>(this)->postRecv_impl(request, cb_done, connid,
                                                   buf, sizes, offsets, counters,
                                                   nranks, myindex);
      }

    };
  };
};
#endif // __components_device_manytomanymodel_h__
