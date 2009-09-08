/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/MessageDevice.h
 * \brief ???
 */

#ifndef __components_devices_messagedevice_h__
#define __components_devices_messagedevice_h__

#include "PacketDevice.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \brief Mesage device interface.
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T Message device template class
      ///
      template <class T>
      class MessageDevice : public PacketDevice<T>
      {
        public:
          MessageDevice () :
           PacketDevice<T> ()
         {};

          ~MessageDevice() {};

          ///
          /// \brief Get the maximum packet metadata size in bytes.
          ///
          /// \attention All message device derived classes \b must
          ///            implement the getMessageMetadataSize_impl() method.
          ///
          /// \return Maximum message metadata size in bytes
          ///
          inline size_t getMessageMetadataSize ();

          ///
          /// \brief Set the opaque connection data for a channel.
          ///
          /// The packet device connection resource allows the receive state
          /// to be preserved between packets received on a channel from the
          /// the same origin rank. The connection resources must be allocated
          /// by the packet device during device initialization.
          ///
          /// For example, a shared memory device will only receive packets
          /// from the other ranks that have access to the same shared memory
          /// segment; a torus device may receive packets from all ranks
          /// defined by the partition; and a tcp/ip device may receive packets
          /// from an unbounded number of ranks since ranks may dynamically
          /// join and drop from a job.
          ///
          /// Each device needs to implement a connection data stucture
          /// specific to the device to optimize performance, scalability, and
          /// memory usage.
          ///
          /// \attention All packet device derived classes \b must
          ///            implement the setConnection_impl() method.
          ///
          /// \param[in] rank    Connection rank, typically the origin global rank
          /// \param[in] arg     Channel connection argument
          ///
          /// \see allocateConnection
          /// \see getConnection
          ///
          inline void setConnection (size_t rank, void * arg);

          ///
          /// \brief Get the opaque connection data for a channel.
          ///
          /// The packet device connection resource allows the receive state
          /// to be preserved between packets received on a channel from the
          /// the same origin rank. The connection resources must be allocated
          /// by the packet device during device initialization.
          ///
          /// For example, a shared memory device will only receive packets
          /// from the other ranks that have access to the same shared memory
          /// segment; a torus device may receive packets from all ranks
          /// defined by the partition; and a tcp/ip device may receive packets
          /// from an unbounded number of ranks since ranks may dynamically
          /// join and drop from a job.
          ///
          /// Each device needs to implement a connection data stucture
          /// specific to the device to optimize performance, scalability, and
          /// memory usage.
          ///
          /// \attention All packet device derived classes \b must
          ///            implement the getConnection_impl() method.
          ///
          /// \param[in] rank    Connection rank, typically the origin global rank
          ///
          /// \return Channel connection argument
          ///
          /// \see allocateConnection
          /// \see setConnection
          ///
          inline void * getConnection (size_t rank);
      };

      template <class T>
      inline size_t MessageDevice<T>::getMessageMetadataSize ()
      {
        return static_cast<T*>(this)->getMessageMetadataSize_impl ();
      }

      template <class T>
      inline void MessageDevice<T>::setConnection (size_t rank, void * arg)
      {
        static_cast<T*>(this)->setConnection_impl (rank, arg);
      }

      template <class T>
      inline void * MessageDevice<T>::getConnection (size_t rank)
      {
        return static_cast<T*>(this)->getConnection_impl (rank);
      }
    };
  };
};

#endif // __components_devices_messagedevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
