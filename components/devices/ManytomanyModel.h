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

#include <pami.h>
#include "algorithms/ccmi.h"  //for PAMI_Callback_t...probably move this at some point
namespace PAMI
{
  namespace Device
  {
    namespace Interface
    {
      template <class T_Model, class T_Device, unsigned T_StateBytes>
      class ManytomanyModel
      {
      public:
        /// \param[in] device                Manytomany device reference
        ManytomanyModel (T_Device & device, pami_result_t &status)
        {
          COMPILE_TIME_ASSERT(T_Model::sizeof_msg == T_StateBytes);
          status = PAMI_SUCCESS;
        };
        ~ManytomanyModel () {};
        inline pami_result_t postManytomany(uint8_t (&state)[T_StateBytes],
                                            pami_manytomany_t *m2minfo);

      };

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      pami_result_t ManytomanyModel<T_Model, T_Device, T_StateBytes>::postManytomany(uint8_t (&state)[T_StateBytes], pami_manytomany_t *m2minfo)

      {
        return static_cast<T_Model*>(this)->postManytomany_impl(state, m2minfo);
      }


      ///
      /// \brief Add an active message dispatch to the multicast model.
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      ///
      /// \param T_Model   Manytomany model template class
      ///
      /// \see Manytomany::Model
      ///
      /** \todo We probably need client and context passed to the model */
      template <class T_Model,class T_Device,unsigned T_StateBytes>
      class AMManytomanyModel : public ManytomanyModel<T_Model,T_Device,T_StateBytes>
      {
      public:
        AMManytomanyModel (T_Device &device, pami_result_t &status) :
          ManytomanyModel<T_Model,T_Device,T_StateBytes> (device, status)
          {
          };
        ~AMManytomanyModel ()
          {
          };
        inline pami_result_t
        registerManytomanyRecvFunction(int dispatch_id,
                                       pami_dispatch_manytomany_fn recv_func,
                                       void  *async_arg);
      }; // class AMManytomanyModel
      template <class T_Model,class T_Device,unsigned T_StateBytes>
      pami_result_t AMManytomanyModel<T_Model,T_Device,T_StateBytes>::registerManytomanyRecvFunction (int dispatch_id, pami_dispatch_manytomany_fn recv_func, void *async_arg)
      {
        return static_cast<T_Model*>(this)->registerManytomanyRecvFunction_impl (dispatch_id, recv_func, async_arg);
      }

      
    };
  };
};
#endif // __components_device_manytomanymodel_h__
