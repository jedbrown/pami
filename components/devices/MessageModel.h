/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
#include "util/common.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model       Message model template class
      /// \param T_Device      Message device template class
      /// \param T_StateBytes  Transfer state object size in bytes
      ///
      template <class T_Model, class T_Device, unsigned T_StateBytes>
      class MessageModel : public PacketModel<T_Model, T_Device, T_StateBytes>
      {
        public:
          MessageModel (T_Device      & device,
                        xmi_context_t   context) :
              PacketModel<T_Model, T_Device, T_StateBytes> (device, context)
          {
            // This compile time assert verify that the specific message model
            // class, T_Model, has correctly specified the same value for the
            // 'transfer state bytes' template parameter and constant.
            COMPILE_TIME_ASSERT(T_Model::message_model_state_bytes == T_StateBytes);
          };

          ~MessageModel () {};

          ///
          /// \brief Returns the deterministic network attribute of this model
          ///
          /// A deterministic network "routes" all communication in a fixed,
          /// deterministic, way such that messages are received by the remote
          /// task in the same order as the messages were sent by the local
          /// task.
          ///
          /// \attention All message model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'bool deterministic_message_model'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'deterministic_message_model' constant.
          ///
          static const bool isMessageDeterministic ();

          ///
          /// \brief Returns the reliable network attribute of this model
          ///
          /// A reliable network will not drop packets during the packet
          /// transfer. Protocols written to a reliable message model
          /// implementation may assume that all messages sent using the
          /// message interface methods of the model \b will arrive at the
          /// destination task.
          ///
          /// \attention All message model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'bool reliable_message_model'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'reliable_message_model' constant.
          ///
          static const bool isMessageReliable ();

          ///
          /// \brief Returns the maximum metadata bytes attribute of this model.
          ///
          /// Certain packet-based hardware may provide a contiguous area in
          /// packet network header that may be initialized and transfered with
          /// the packet to its destination. This attribute specifies the
          /// maximum number of bytes that may be sent in the packet metadata
          /// using any of the post* methods of the message model interface.
          ///
          /// The message model interface will deliver the \b same metadata in
          /// all packet dispatch function callbacks.
          ///
          /// A message model implementation may return zero as the number of
          /// message metadata bytes supported.
          ///
          /// \attention All message model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'size_t message_model_metadata_bytes'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'packet_model_metadata_bytes' constant.
          ///
          static const size_t getMessageMetadataBytes ();

          ///
          /// \brief Returns the maximum payload bytes attribute of this model.
          ///
          /// Packet-based network hardware provides a contiguous payload area
          /// within each packet transfered to the destination task. This
          /// attribute specifies the maximum number of bytes that may be sent
          /// in the packet payload using any of the post* methods of the
          /// message model interface.
          ///
          /// A message model implementation may return zero as the number of
          /// message payload bytes supported.
          ///
          /// \attention All message model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'size_t message_model_payload_bytes'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'message_model_payload_bytes' constant.
          ///
          static const size_t getMessagePayloadBytes ();

          ///
          /// \brief Returns the transfer state bytes attribute of this model.
          ///
          /// Typically a message device will require some amount of temporary
          /// storage to be used during the transfer of the message. This
          /// attribute returns the number of bytes that must be provided to
          /// the various message post* methods.
          ///
          /// A message model implementation may return zero as the number of
          /// packet transfer state bytes required.
          ///
          /// \attention All message model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'size_t message_model_state_bytes'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'message_model_state_bytes' constant.
          ///
          static const size_t getMessageTransferStateBytes ();

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
          /// \see getMessageMetadataSize()
          ///
          /// \param[in] state        Location to store the transfer object
          /// \param[in] fn           Event function to invoke when the operation completes
          /// \param[in] cookie       Opaque cookie data to pass as the cookie parameter of the event function.
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
          inline bool postMessage (uint8_t              (&state)[T_StateBytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   size_t               target_rank,
                                   void               * metadata,
                                   size_t               metasize,
                                   void               * payload,
                                   size_t               bytes);
      };

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const bool MessageModel<T_Model, T_Device, T_StateBytes>::isMessageDeterministic ()
      {
        return T_Model::deterministic_message_model;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const bool MessageModel<T_Model, T_Device, T_StateBytes>::isMessageReliable ()
      {
        return T_Model::reliable_message_model;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const size_t MessageModel<T_Model, T_Device, T_StateBytes>::getMessageMetadataBytes ()
      {
        return T_Model::message_model_metadata_bytes;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const size_t MessageModel<T_Model, T_Device, T_StateBytes>::getMessagePayloadBytes ()
      {
        return T_Model::message_model_payload_bytes;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const size_t MessageModel<T_Model, T_Device, T_StateBytes>::getMessageTransferStateBytes ()
      {
        return T_Model::message_model_state_bytes;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool MessageModel<T_Model, T_Device, T_StateBytes>::postMessage (uint8_t              (&state)[T_StateBytes],
                                                                              xmi_event_function   fn,
                                                                              void               * cookie,
                                                                              size_t               target_rank,
                                                                              void               * metadata,
                                                                              size_t               metasize,
                                                                              void               * payload,
                                                                              size_t               bytes)
      {
        return static_cast<T_Model*>(this)->postMessage_impl (state, fn, cookie, target_rank,
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
