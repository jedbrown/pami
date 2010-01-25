/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/MulticastModel.h
 * \brief ???
 */

#ifndef __components_devices_MulticastModel_h__
#define __components_devices_MulticastModel_h__

#include <sys/uio.h>
#include "sys/xmi.h"
#include "util/common.h"

namespace XMI
{
  namespace Device
  {
    class MulticastUnspecifiedDevice
    {
    private:
      MulticastUnspecifiedDevice() {
      };
      ~MulticastUnspecifiedDevice(){
      };
    };
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model   Multicast model template class
      ///
      /// \see Multicast::Model
      ///
      template <class T_Model,unsigned T_StateBytes, class T_Device=XMI::Device::MulticastUnspecifiedDevice>
      class MulticastModel
      {
      public:
        MulticastModel (xmi_result_t &status)
          {
            COMPILE_TIME_ASSERT(T_Model::sizeof_msg == T_StateBytes);
            status = XMI_SUCCESS;
          };
        MulticastModel (xmi_result_t &status, T_Device &device)
          {
            COMPILE_TIME_ASSERT(T_Model::sizeof_msg == T_StateBytes);
            status = XMI_SUCCESS;
          };
        ~MulticastModel ()
          {
          };
        inline xmi_result_t postMulticast(uint8_t (&state)[T_StateBytes],
                                           xmi_multicast_t *mcast);
      }; // class MulticastModel

      template <class T_Model,unsigned T_StateBytes,class T_Device>
      xmi_result_t MulticastModel<T_Model, T_StateBytes,T_Device>::postMulticast(uint8_t (&state)[T_StateBytes],
                                                                         xmi_multicast_t *mcast)
      {
        return static_cast<T_Model*>(this)->postMulticast_impl(state, mcast);
      }

      ///
      /// \brief Add an active message dispatch to the multicast model.
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      ///
      /// \param T_Model   Multicast model template class
      ///
      /// \see Multicast::Model
      ///
      /** \todo We probably need client and context passed to the model */
      template <class T_Model,unsigned T_StateBytes,class T_Device=XMI::Device::MulticastUnspecifiedDevice>
      class AMMulticastModel : public MulticastModel<T_Model,T_StateBytes,T_Device>
      {
      public:
        AMMulticastModel (xmi_result_t &status) :
          MulticastModel<T_Model,T_StateBytes,T_Device> (status)
          {
          };
        AMMulticastModel (xmi_result_t &status, T_Device &device) :
          MulticastModel<T_Model,T_StateBytes,T_Device> (status, device)
          {
          };
        ~AMMulticastModel ()
          {
          };
        inline xmi_result_t registerMcastRecvFunction (int                        dispatch_id,
                                                       xmi_dispatch_multicast_fn  recv_func,
                                                       void                      *async_arg);
      }; // class AMMulticastModel
      template <class T_Model,unsigned T_StateBytes,class T_Device>
      xmi_result_t AMMulticastModel<T_Model,T_StateBytes,T_Device>::registerMcastRecvFunction (int                        dispatch_id,
                                                                                               xmi_dispatch_multicast_fn  recv_func,
                                                                                               void                      *async_arg)
      {
        return static_cast<T_Model*>(this)->registerMcastRecvFunction_impl (dispatch_id,
                                                                            recv_func,
                                                                            async_arg);
      }
    }; // namespace Interface
  }; // namespace Device
}; // namespace XMI
#endif // __components_devices_MulticastModel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
