/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/PacketDevice.h
 * \brief ???
 */

#ifndef __components_devices_packetdevice_h__
#define __components_devices_packetdevice_h__

#include "xmi.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \brief Packet device dispatch receive function signature
      ///
      /// \note The packet device \b must provide base address pointers in the
      ///       iovec elements that are 16 byte aligned. This allows the receive
      ///       function implementation to optimize how it provides the user data
      ///       to the application.
      ///
      /// \param[in] metadata       Pointer to network header metadata that is
      ///                           not part of the normal packet payload.
      /// \param[in] payload        Pointer to the raw packet payload.
      /// \param[in] bytes          Number of valid bytes of packet payload.
      /// \param[in] recv_func_parm Registered dispatch clientdata
      ///
      /// \todo Define return value(s)
      ///
      /// \return ???
      ///
      typedef int (*RecvFunction_t) (void   * metadata,
                                     void   * payload,
                                     size_t   bytes,
                                     void   * recv_func_parm);

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
          /// \brief Read data from a channel
          ///
          /// It is only required to read data from the channel if the
          /// PacketDevice::requiresRead() method returns \c true
          ///
          /// \attention All packet device derived classes \b must
          ///            implement the readData_impl() method.
          ///
          /// \param[in] dst      Destination buffer to read data to
          /// \param[in] bytes    Number of bytes to read
          ///
          /// \todo Define return value(s)
          ///
          /// \see requiresRead()
          ///
          /// \return 0 on success, !0 on failure
          ///
          inline int readData (void * dst, size_t bytes);

          ///
          /// \brief Return the packet device implementation read attribute.
          ///
          /// \attention All packet device derived classes \b must
          ///            implement the requiresRead_impl() method.
          ///
          /// \see readData()
          ///
          /// \retval true  The dispatch function \b must use readData()
          /// \retval false The dispatch function will have direct access to the data.
          ///
          inline bool requiresRead ();

          ///
          /// \brief Get the maximum packet metadata size in bytes.
          ///
          /// \attention All packet device derived classes \b must
          ///            implement the getPacketMetadataSize_impl() method.
          ///
          /// \return Maximum packet metadata size in bytes
          ///
          inline size_t getPacketMetadataSize ();

          ///
          /// \brief Get the maximum packet payload size in bytes.
          ///
          /// \attention All packet device derived classes \b must
          ///            implement the getPacketPayloadSize_impl() method.
          ///
          /// \return Maximum packet payload size in bytes
          ///
          inline size_t getPacketPayloadSize ();

      };

      template <class T>
      inline int PacketDevice<T>::readData (int channel, void * buf, size_t length)
      {
        return static_cast<T*>(this)->readData_impl(channel, buf, length);
      }

      template <class T>
      inline bool PacketDevice<T>::requiresRead ()
      {
        return static_cast<T*>(this)->requiresRead_impl();
      }

      template <class T>
      inline size_t PacketDevice<T>::getPacketMetadataSize ()
      {
        return static_cast<T*>(this)->getPacketMetadataSize_impl ();
      }

      template <class T>
      inline size_t PacketDevice<T>::getPacketPayloadSize ()
      {
        return static_cast<T*>(this)->getPacketPayloadSize_impl ();
      }
    };
  };
};

#endif // __components_devices_packetdevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
