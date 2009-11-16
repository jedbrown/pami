/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/PacketInterface.h
 * \brief ???
 */

#ifndef __components_devices_PacketInterface_h__
#define __components_devices_PacketInterface_h__

#include <sys/uio.h>

#include "sys/xmi.h"
#include "util/common.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \brief Packet device dispatch receive function signature
      ///
      /// \param[in] metadata       Pointer to network header metadata that is
      ///                           not part of the normal packet payload.
      /// \param[in] payload        Pointer to the raw packet payload.
      /// \param[in] bytes          Number of valid bytes of packet payload.
      /// \param[in] recv_func_parm Registered dispatch clientdata
      /// \param[in] cookie         Device cookie
      ///
      /// \todo Define return value(s)
      ///
      /// \return ???
      ///
      typedef int (*RecvFunction_t) (void   * metadata,
                                     void   * payload,
                                     size_t   bytes,
                                     void   * recv_func_parm,
                                     void   * cookie);

      ///
      /// \brief Packet device interface.
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T Packet device template class
      ///
      template <class T>
      class PacketDevice
      {
        public:
          PacketDevice() {}
          ~PacketDevice() {}

          ///
          /// \brief Read data from a packet device
          ///
          /// \attention All packet device derived classes \b must
          ///            implement the readData_impl() method.
          ///
          /// \param[in] dst      Destination buffer
          /// \param[in] bytes    Number of bytes to read
          /// \param[in] cookie   Device cookie
          ///
          /// \todo Define return value(s)
          ///
          /// \return 0 on success, !0 on failure
          ///
          inline int read (void * dst, size_t bytes, void * cookie);
      };

      template <class T>
      inline int PacketDevice<T>::read (void * dst, size_t bytes, void * cookie)
      {
        return static_cast<T*>(this)->read_impl(dst, bytes, cookie);
      }

      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model       Packet model template class
      /// \param T_Device      Packet device template class
      /// \param T_StateBytes  Transfer state object size in bytes
      ///
      /// \see Packet::Model
      /// \see Packet::Device
      ///
      template <class T_Model, class T_Device, unsigned T_StateBytes>
      class PacketModel
      {
        public:

          /// \brief Packet model interface constructor
          ///
          /// \param[in] device  Packet device reference
          /// \param[in] context Communication context
          ///
          PacketModel (T_Device & device, xmi_client_t client, size_t context)
          {
            // This compile time assert verify that the specific packet model
            // class, T_Model, has correctly specified the same value for the
            // 'transfer state bytes' template parameter and constant.
            COMPILE_TIME_ASSERT(T_Model::packet_model_state_bytes == T_StateBytes);
          };

          ~PacketModel () {};

          ///
          /// \brief Returns the deterministic network attribute of this model
          ///
          /// A deterministic network "routes" all communication in a fixed,
          /// deterministic, way such that messages are received by the remote
          /// task in the same order as the messages were sent by the local
          /// task.
          ///
          /// \attention All packet model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'bool deterministic_packet_model'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'deterministic_packet_model' constant.
          ///
          static const bool isPacketDeterministic ();

          ///
          /// \brief Returns the reliable network attribute of this model
          ///
          /// A reliable network will not drop packets during the packet
          /// transfer. Protocols written to a reliable packet model
          /// implementation may assume that all packets sent using the
          /// packet interface methods of the model \b will arrive at the
          /// destination task.
          ///
          /// \attention All packet model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'bool reliable_packet_model'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'reliable_packet_model' constant.
          ///
          static const bool isPacketReliable ();

          ///
          /// \brief Returns the maximum metadata bytes attribute of this model for single-packet transfers.
          ///
          /// Certain packet-based hardware may provide a contiguous area in
          /// packet network header that may be initialized and transfered with
          /// the packet to its destination. This attribute specifies the
          /// maximum number of bytes that may be sent in the packet metadata
          /// using any of the post* methods of the packet model interface.
          ///
          /// A packet model implementation may return zero as the number of
          /// single-packet metadata bytes supported.
          ///
          /// \attention All packet model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'size_t packet_model_metadata_bytes'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'packet_model_metadata_bytes' constant.
          ///
          static const size_t getPacketMetadataBytes ();

          ///
          /// \brief Returns the maximum metadata bytes attribute of this model for multi-packet transfers.
          ///
          /// Certain packet-based hardware may provide a contiguous area in
          /// packet network header that may be initialized and transfered with
          /// each packet of a multi-packet message to its destination. This
          /// attribute specifies the maximum number of bytes that may be sent
          /// in the packet metadata using any of the post* methods of the
          /// packet model interface.
          ///
          /// A packet model implementation may return zero as the number of
          /// multi-packet metadata bytes supported.
          ///
          /// \attention All packet model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'size_t packet_model_multi_metadata_bytes'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'packet_model_metadata_bytes' constant.
          ///
          static const size_t getPacketMultiMetadataBytes ();

          ///
          /// \brief Returns the maximum payload bytes attribute of this model.
          ///
          /// Packet-based network hardware provides a contiguous payload area
          /// within each packet transfered to the destination task. This
          /// attribute specifies the maximum number of bytes that may be sent
          /// in the packet payload using any of the post* methods of the
          /// packet model interface.
          ///
          /// A packet model implementation may return zero as the number of
          /// packet payload bytes supported.
          ///
          /// \attention All packet model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'size_t packet_model_payload_bytes'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'packet_model_payload_bytes' constant.
          ///
          static const size_t getPacketPayloadBytes ();

          ///
          /// \brief Returns the transfer state bytes attribute of this model.
          ///
          /// Typically a packet device will require some amount of temporary
          /// storage to be used during the transfer of the packet. This
          /// attribute returns the number of bytes that must be provided to
          /// the various packet post* methods.
          ///
          /// \attention All packet model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'size_t packet_model_state_bytes'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'packet_model_state_bytes' constant.
          ///
          static const size_t getPacketTransferStateBytes ();

          ///
          /// \brief Base packet model initializer
          ///
          /// The packet device implementation will use the appropriate receive
          /// function depending on the packet device "requires read" attribute.
          ///
          /// \see XMI::Device::Interface::PacketDevice::readData()
          ///
          /// \param[in] dispatch              Dispatch set identifier
          /// \param[in] direct_recv_func      Receive function for direct-access packet devices
          /// \param[in] direct_recv_func_parm Receive function clientdata for direct-access packet devices
          /// \param[in] read_recv_func        Receive function for read-access packet devices
          /// \param[in] read_recv_func_parm   Receive function clientdata for read-access packet devices
          ///
          xmi_result_t init (size_t           dispatch,
                             RecvFunction_t   direct_recv_func,
                             void           * direct_recv_func_parm,
                             RecvFunction_t   read_recv_func,
                             void           * read_recv_func_parm);

          ///
          /// \brief Immediate post of a single packet transfer operation
          ///
          /// The post packet immediate interface allows the packet model
          /// and packet device implementations to optimize for performance by
          /// avoiding the overhead to construct a packet message object. If
          /// the packet device has resources immediately available then the
          /// packet may be directly posted, otherwise a packet message object
          /// may be constructed at the location specified to maintain the
          /// operation state until the completion callback is invoked.
          ///
          /// \note The packet device will only transfer a single packet. The
          ///       payload data will be truncated if more bytes than a single
          ///       packet can transfer is specified as the payload.
          ///
          /// \see XMI::Device::Interface::PacketDevice::getPacketPayloadSize()
          ///
          /// \note The size of the metadata to be copied into the packet
          ///       is an attribute of the specific packet device associated
          ///       with this packet model.
          ///
          /// \see XMI::Device::Interface::PacketDevice::getPacketMetadataSize()
          ///
          /// \param[in] target_rank  Global rank of the packet destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] iov          Array of iovec elements to transfer
          ///
          /// \retval true  All data has been immediately sent
          /// \retval false Resources are not immediately available to complete the transfer
          ///
          template <unsigned T_Niov>
          inline bool postPacket (size_t         target_rank,
                                  void         * metadata,
                                  size_t         metasize,
                                  struct iovec   (&iov)[T_Niov]);

          ///
          /// \brief Post a single packet transfer operation with completion event notification
          ///
          /// \note The packet device will only transfer a single packet. The
          ///       payload data will be truncated if more bytes than a single
          ///       packet can transfer is specified as the payload.
          ///
          /// \see XMI::Device::Interface::PacketDevice::getPacketPayloadSize()
          ///
          /// \note The size of the metadata to be copied into the packet
          ///       is an attribute of the specific packet device associated
          ///       with this packet model.
          ///
          /// \see XMI::Device::Interface::PacketDevice::getPacketMetadataSize()
          ///
          /// \param[in] state        Byte array reference for the packet transfer state
          /// \param[in] cb           Callback to invoke when the operation completes
          /// \param[in] target_rank  Global rank of the packet destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] payload      Virtual address of source buffer
          /// \param[in] bytes        Number of bytes to transfer
          ///
          /// \retval true  Packet operation completed and the completion
          ///               callback was invoked
          ///
          /// \retval false Packet operation did not complete and the packet
          ///               device must be advanced until the completion
          ///               callback is invoked
          ///
          inline bool postPacket (uint8_t              (&state)[T_StateBytes],
                                  xmi_event_function   fn,
                                  void               * cookie,
                                  size_t               target_rank,
                                  void               * metadata,
                                  size_t               metasize,
                                  struct iovec       * iov,
                                  size_t               niov);

          template <unsigned T_Niov>
          inline bool postPacket (uint8_t              (&state)[T_StateBytes],
                                  xmi_event_function   fn,
                                  void               * cookie,
                                  size_t               target_rank,
                                  void               * metadata,
                                  size_t               metasize,
                                  struct iovec         (&iov)[T_Niov]);

          ///
          /// \brief Post a multiple packet transfer operation
          ///
          /// Transfer one or more contiguous source buffers to the destination
          /// task as a stream of individual packets.  The metadata is copied
          /// into each packet header and is unchanged during the duration of
          /// the transfer.
          ///
          /// The post multi-packet interface allows the packet model and
          /// device implementations to optimize for performance by avoiding
          /// the overhead to construct a transfer state object. If
          /// the device has resources immediately available then the
          /// transfer operation may be directly posted, otherwise a transfer state object
          /// may be constructed at the location specified to maintain the
          /// operation state until the completion callback is invoked.
          ///
          /// \note The size of the metadata to be copied into the packet
          ///       is an attribute of the specific packet device associated
          ///       with this packet model.
          ///
          /// \see XMI::Device::Interface::PacketDevice::getMultiPacketMetadataSize()
          ///
          /// \param[in] state        Location to store the transfer object
          /// \param[in] fn           Event function to invoke when the operation completes
          /// \param[in] cookie       Opaque data to provide as the cookie parameter of the event function.
          /// \param[in] target_rank  Global rank of the destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] iov          Array of iovec elements to transfer
          ///
          /// \retval true  Transfer operation completed and the completion
          ///               callback was invoked
          ///
          /// \retval false Transfer operation did not complete and the
          ///               device must be advanced until the completion
          ///               callback is invoked
          ///
          template <unsigned T_Niov>
          inline bool postMultiPacket (uint8_t              (&state)[T_StateBytes],
                                       xmi_event_function   fn,
                                       void               * cookie,
                                       size_t               target_rank,
                                       void               * metadata,
                                       size_t               metasize,
                                       struct iovec         (&iov)[T_Niov]);

      };

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const bool PacketModel<T_Model, T_Device, T_StateBytes>::isPacketDeterministic ()
      {
        return T_Model::deterministic_packet_model;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const bool PacketModel<T_Model, T_Device, T_StateBytes>::isPacketReliable ()
      {
        return T_Model::reliable_packet_model;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const size_t PacketModel<T_Model, T_Device, T_StateBytes>::getPacketMetadataBytes ()
      {
        return T_Model::packet_model_metadata_bytes;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const size_t PacketModel<T_Model, T_Device, T_StateBytes>::getPacketMultiMetadataBytes ()
      {
        return T_Model::packet_model_multi_metadata_bytes;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const size_t PacketModel<T_Model, T_Device, T_StateBytes>::getPacketPayloadBytes ()
      {
        return T_Model::packet_model_payload_bytes;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const size_t PacketModel<T_Model, T_Device, T_StateBytes>::getPacketTransferStateBytes ()
      {
        return T_Model::packet_model_state_bytes;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      xmi_result_t PacketModel<T_Model, T_Device, T_StateBytes>::init (size_t           dispatch,
                                                                       RecvFunction_t   direct_recv_func,
                                                                       void           * direct_recv_func_parm,
                                                                       RecvFunction_t   read_recv_func,
                                                                       void           * read_recv_func_parm)
      {
        return static_cast<T_Model*>(this)->init_impl (dispatch,
                                                       direct_recv_func,
                                                       direct_recv_func_parm,
                                                       read_recv_func,
                                                       read_recv_func_parm);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      template <unsigned T_Niov>
      inline bool PacketModel<T_Model, T_Device, T_StateBytes>::postPacket (size_t         target_rank,
                                                                            void         * metadata,
                                                                            size_t         metasize,
                                                                            struct iovec   (&iov)[T_Niov])
      {
        return static_cast<T_Model*>(this)->postPacket_impl (target_rank, metadata, metasize, iov);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool PacketModel<T_Model, T_Device, T_StateBytes>::postPacket (uint8_t              (&state)[T_StateBytes],
                                                                            xmi_event_function   fn,
                                                                            void               * cookie,
                                                                            size_t               target_rank,
                                                                            void               * metadata,
                                                                            size_t               metasize,
                                                                            struct iovec       * iov,
                                                                            size_t               niov)
      {
        return static_cast<T_Model*>(this)->postPacket_impl (state, fn, cookie, target_rank,
                                                             metadata, metasize, iov, niov);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      template <unsigned T_Niov>
      inline bool PacketModel<T_Model, T_Device, T_StateBytes>::postPacket (uint8_t              (&state)[T_StateBytes],
                                                                            xmi_event_function   fn,
                                                                            void               * cookie,
                                                                            size_t               target_rank,
                                                                            void               * metadata,
                                                                            size_t               metasize,
                                                                            struct iovec         (&iov)[T_Niov])
      {
        return static_cast<T_Model*>(this)->postPacket_impl (state, fn, cookie, target_rank,
                                                             metadata, metasize, iov);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      template <unsigned T_Niov>
      inline bool PacketModel<T_Model, T_Device, T_StateBytes>::postMultiPacket (uint8_t              (&state)[T_StateBytes],
                                                                                 xmi_event_function   fn,
                                                                                 void               * cookie,
                                                                                 size_t               target_rank,
                                                                                 void               * metadata,
                                                                                 size_t               metasize,
                                                                                 struct iovec         (&iov)[T_Niov])
      {
        return static_cast<T_Model*>(this)->postMultiPacket_impl (state, fn, cookie, target_rank,
                                                                  metadata, metasize, iov);
      }
    };
  };
};
#endif // __components_devices_PacketInterface_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
