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
#include <pami.h>
#include "util/common.h"

namespace PAMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model   Multicast model template class
      ///
      /// \see Multicast::Model
      ///
      template <class T_Model,class T_Device,unsigned T_StateBytes>
      class MulticastModel
      {
      public:
        MulticastModel (T_Device &device, pami_result_t &status)
          {
            COMPILE_TIME_ASSERT(T_Model::sizeof_msg <= T_StateBytes);
            status = PAMI_SUCCESS;
          };
        ~MulticastModel ()
          {
          };
        inline pami_result_t postMulticast(uint8_t (&state)[T_StateBytes],
                                           pami_multicast_t *mcast);
      }; // class MulticastModel

      template <class T_Model,class T_Device,unsigned T_StateBytes>
      pami_result_t MulticastModel<T_Model,T_Device, T_StateBytes>::postMulticast(uint8_t (&state)[T_StateBytes],
                                                                         pami_multicast_t *mcast)
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
      template <class T_Model,class T_Device,unsigned T_StateBytes>
      class AMMulticastModel : public MulticastModel<T_Model,T_Device,T_StateBytes>
      {
      public:
        AMMulticastModel (T_Device &device, pami_result_t &status) :
          MulticastModel<T_Model,T_Device,T_StateBytes> (device, status)
          {
          };
        ~AMMulticastModel ()
          {
          };
        inline pami_result_t registerMcastRecvFunction (int                        dispatch_id,
                                                       pami_dispatch_multicast_fn  recv_func,
                                                       void                      *async_arg);
      }; // class AMMulticastModel
      template <class T_Model,class T_Device,unsigned T_StateBytes>
      pami_result_t AMMulticastModel<T_Model,T_Device,
                                    T_StateBytes>::registerMcastRecvFunction (int                        dispatch_id,
                                                                                               pami_dispatch_multicast_fn  recv_func,
                                                                                               void                      *async_arg)
      {
        return static_cast<T_Model*>(this)->registerMcastRecvFunction_impl (dispatch_id,
                                                                            recv_func,
                                                                            async_arg);
      }
    }; // namespace Interface
  }; // namespace Device
}; // namespace PAMI
#endif // __components_devices_MulticastModel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
