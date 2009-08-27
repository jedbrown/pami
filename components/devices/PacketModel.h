/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/PacketModel.h
 * \brief ???
 */

#ifndef __components_device_packetmodel_h__
#define __components_device_packetmodel_h__

#include <sys/uio.h>

#include "xmi.h"

#include "PacketDevice.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model   Packet model template class
      /// \param T_Device  Packet device template class
      /// \param T_Object  Packet object template class
      ///
      /// \see Packet::Model
      /// \see Packet::Device
      ///
      template <class T_Model, class T_Device, class T_Object>
      class PacketModel
      {
        public:
          /// \param[in] device                Packet device reference
          PacketModel (T_Device & device) {};
          ~PacketModel () {};

          ///
          /// \brief Base packet model initializer
          ///
          /// The packet device implementation will use the appropriate receive
          /// function depending on the packet device "requires read" attribute.
          ///
          /// \see XMI::Device::Interface::PacketDevice::requiresRead()
          /// \see XMI::Device::Interface::PacketDevice::readData()
          ///
          /// \param[in] direct_recv_func      Receive function for direct-access packet devices
          /// \param[in] direct_recv_func_parm Receive function clientdata for direct-access packet devices
          /// \param[in] read_recv_func        Receive function for read-access packet devices
          /// \param[in] read_recv_func_parm   Receive function clientdata for read-access packet devices
          ///
          bool init (RecvFunction_t   direct_recv_func,
                     void           * direct_recv_func_parm,
                     RecvFunction_t   read_recv_func,
                     void           * read_recv_func_parm);

          ///
          /// \brief Post a single packet contigous transfer operation
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
          /// \param[in] obj          Location to store the packet transfer state
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
          inline bool postPacket (T_Object           * obj,
                                  xmi_event_function   fn,
                                  void               * cookie,
                                  size_t               target_rank,
                                  void               * metadata,
                                  size_t               metasize,
                                  void               * payload,
                                  size_t               bytes);

          ///
          /// \brief Post a single packet non-contiguous transfer operation
          ///
          /// \param[in] obj          Location to store the packet transfer state
          /// \param[in] cb           Callback to invoke when the operation completes
          /// \param[in] target_rank  Global rank of the packet destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] payload0     Virtual address of the first source buffer
          /// \param[in] bytes0       Number of bytes to transfer from the first buffer
          /// \param[in] payload1     Virtual address of the second source buffer
          /// \param[in] bytes1       Number of bytes to transfer from the second buffer
          ///
          inline bool postPacket (T_Object        * obj,
                                  xmi_event_function   fn,
                                  void               * cookie,
                                  size_t            target_rank,
                                  void            * metadata,
                                  size_t            metasize,
                                  void            * payload0,
                                  size_t            bytes0,
                                  void            * payload1,
                                  size_t            bytes1);

          ///
          /// \brief Post a single packet non-contiguous transfer operation
          ///
          /// \param[in] obj          Location to store the packet transfer state
          /// \param[in] cb           Callback to invoke when the operation completes
          /// \param[in] target_rank  Global rank of the packet destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] payload      Virtual address of source buffer
          /// \param[in] bytes        Number of bytes to transfer
          ///
          inline bool postPacket (T_Object        * obj,
                                  xmi_event_function   fn,
                                  void               * cookie,
                                  size_t            target_rank,
                                  void            * metadata,
                                  size_t            metasize,
                                  struct iovec    * iov,
                                  size_t            niov);
      };

      template <class T_Model, class T_Device, class T_Object>
      bool PacketModel<T_Model, T_Device, T_Object>::init (RecvFunction_t   direct_recv_func,
                                                     void           * direct_recv_func_parm,
                                                     RecvFunction_t   read_recv_func,
                                                     void           * read_recv_func_parm)
      {
        return static_cast<T_Model*>(this)->init_impl (direct_recv_func,
                                                       direct_recv_func_parm,
                                                       read_recv_func,
                                                       read_recv_func_parm);
      }

      template <class T_Model, class T_Device, class T_Object>
      inline bool PacketModel<T_Model, T_Device, T_Object>::postPacket (T_Object        * obj,
                                                                  xmi_event_function   fn,
                                                                  void               * cookie,
                                                                  size_t            target_rank,
                                                                  void            * metadata,
                                                                  size_t            metasize,
                                                                  void            * payload,
                                                                  size_t            bytes)
      {
        return static_cast<T_Model*>(this)->postPacket_impl (obj, fn, cookie, target_rank,
                                                             metadata, metasize, payload, bytes);
      }

      template <class T_Model, class T_Device, class T_Object>
      inline bool PacketModel<T_Model, T_Device, T_Object>::postPacket (T_Object        * obj,
                                                                  xmi_event_function   fn,
                                                                  void               * cookie,
                                                                  size_t            target_rank,
                                                                  void            * metadata,
                                                                  size_t            metasize,
                                                                  void            * payload0,
                                                                  size_t            bytes0,
                                                                  void            * payload1,
                                                                  size_t            bytes1)
      {
        return static_cast<T_Model*>(this)->postPacket_impl (obj, fn, cookie, target_rank,
                                                             metadata, metasize,
                                                             payload0, bytes0,
                                                             payload1, bytes1);
      }

      template <class T_Model, class T_Device, class T_Object>
      inline bool PacketModel<T_Model, T_Device, T_Object>::postPacket (T_Object        * obj,
                                                                  xmi_event_function   fn,
                                                                  void               * cookie,
                                                                  size_t            target_rank,
                                                                  void            * metadata,
                                                                  size_t            metasize,
                                                                  struct iovec    * iov,
                                                                  size_t            niov)
      {
        return static_cast<T_Model*>(this)->postPacket_impl (obj, fn, cookie, target_rank,
                                                             metadata, metasize, iov, niov);
      }
    };
  };
};
#endif // __components_device_packetmodel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
