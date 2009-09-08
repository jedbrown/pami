/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/MessageModel.h
 * \brief ???
 */

#ifndef __components_devices_messagemodel_h__
#define __components_devices_messagemodel_h__

#include "components/devices/PacketModel.h"
#include "components/devices/MessageDevice.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model   Message model template class
      /// \param T_Device  Message device template class
      /// \param T_Object  Message object template class
      ///
      template <class T_Model, class T_Device, class T_Object>
      class MessageModel : public PacketModel<T_Model, T_Device, T_Object>
      {
        public:
          MessageModel (T_Device      & device,
                        xmi_context_t   context) :
            PacketModel<T_Model, T_Device, T_Object> (device, context)
          {};

          ~MessageModel () {};

          ///
          /// \brief Post a multiple packet contigous transfer operation
          ///
          /// The post message interface allows the message model
          /// and message device implementations to optimize for performance by
          /// avoiding the overhead to construct a transfer object. If
          /// the device has resources immediately available then the
          /// transfer operation may be directly posted, otherwise a transfer object
          /// may be constructed at the location specified to maintain the
          /// operation state until the completion callback is invoked.
          ///
          /// \note The size of the metadata to be copied into each packet
          ///       is an attribute of the specific message device associated
          ///       with this message model.
          ///
          /// \see XMI::Device::Interface::MessageDevice::getMessageMetadataSize()
          ///
          /// \param[in] obj          Location to store the transfer object
          /// \param[in] cb           Callback to invoke when the operation completes
          /// \param[in] target_rank  Global rank of the destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] payload      Virtual address of source buffer
          /// \param[in] bytes        Number of bytes to transfer
          ///
          /// \retval true  Transfer operation completed and the completion
          ///               callback was invoked
          ///
          /// \retval false Transfer operation did not complete and the
          ///               device must be advanced until the completion
          ///               callback is invoked
          ///
          inline bool postMessage (T_Object        * obj,
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t            target_rank,
                                   void            * metadata,
                                   size_t            metasize,
                                   void            * payload,
                                   size_t            bytes);
      };

      template <class T_Model, class T_Device, class T_Object>
      inline bool MessageModel<T_Model, T_Device, T_Object>::postMessage (T_Object        * obj,
                                                                   xmi_event_function   fn,
                                                                   void               * cookie,
                                                                   size_t            target_rank,
                                                                   void            * metadata,
                                                                   size_t            metasize,
                                                                   void            * payload,
                                                                   size_t            bytes)
      {
        return static_cast<T_Model*>(this)->postMessage_impl (obj, fn, cookie, target_rank,
                                                             metadata, metasize, payload, bytes);
      }
    };
  };
};

#endif // __components_devices_messagemodel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
