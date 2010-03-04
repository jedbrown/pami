/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/ManytomanyModel.h
 * \brief ???
 */

#ifndef __components_devices_ManytomanyModel_h__
#define __components_devices_ManytomanyModel_h__

#include "sys/xmi.h"
#include "algorithms/ccmi.h"  //for XMI_Callback_t...probably move this at some point
namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      template <class T_Model, unsigned T_StateBytes>
      class ManytomanyModel
      {
      public:
        /// \param[in] device                Manytomany device reference
        ManytomanyModel (xmi_result_t &status)
          {
            COMPILE_TIME_ASSERT(T_Model::sizeof_msg == T_StateBytes);
            status = XMI_SUCCESS;
          };
        ~ManytomanyModel () {};

        inline void setCallback (xmi_dispatch_manytomany_fn cb_recv, void *arg);
        inline void send  (xmi_manytomany_t parameters);
        inline void postRecv (uint8_t (&state)[T_StateBytes],
                              const XMI_Callback_t  * cb_done,
                              unsigned                 connid,
                              char                   * buf,
                              unsigned               * sizes,
                              unsigned               * offsets,
                              unsigned               * counters,
                              unsigned                 nranks,
                              unsigned                 myindex);
      };
      template <class T_Model, unsigned T_StateBytes>
      void ManytomanyModel<T_Model, T_StateBytes>::setCallback (xmi_dispatch_manytomany_fn cb_recv,
                                                                      void *arg)
      {
        static_cast<T_Model*>(this)->setCallback_impl(cb_recv, arg);
      }

      template <class T_Model, unsigned T_StateBytes>
      void ManytomanyModel<T_Model, T_StateBytes>::send (xmi_manytomany_t parameters)
      {
        static_cast<T_Model*>(this)->send_impl(parameters);
      }

      template <class T_Model, unsigned T_StateBytes>
      void ManytomanyModel<T_Model, T_StateBytes>::postRecv (uint8_t (&state)[T_StateBytes],
                                                                   const XMI_Callback_t  * cb_done,
                                                                   unsigned                connid,
                                                                   char                  * buf,
                                                                   unsigned              * sizes,
                                                                   unsigned              * offsets,
                                                                   unsigned              * counters,
                                                                   unsigned                nranks,
                                                                   unsigned                myindex)
      {
        static_cast<T_Model*>(this)->postRecv_impl(state, cb_done, connid,
                                                   buf, sizes, offsets, counters,
                                                   nranks, myindex);
      }

    };
  };
};
#endif // __components_device_manytomanymodel_h__
