/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/fifo/Packet.h
 * \brief ???
 */

#ifndef __util_fifo_Packet_h__
#define __util_fifo_Packet_h__

#ifndef TRACE
#define TRACE(x)
#endif

namespace PAMI
{
  namespace Fifo
  {
    ///
    /// \brief Packet interface class
    ///
    /// \tparam T_Packet Packet implementation class
    ///
    template <class T_Packet>
    class Packet
    {
      public:

        ///
        /// \brief Number of available bytes in the packet header for application data
        ///
        /// \note All fifo implementation classes must define the static constant
        ///       \c headerSize_impl.
        ///
        static const size_t header_size  = T_Packet::headerSize_impl;

        ///
        /// \brief Number of available bytes in the packet payload for application data
        ///
        /// \note All fifo implementation classes must define the static constant
        ///       \c payloadSize_impl.
        ///
        static const size_t payload_size = T_Packet::payloadSize_impl;

        Packet () {};
        ~Packet () {};

        ///
        /// \brief Clear all header and payload data in a packet
        ///
        inline void clear ();

        ///
        /// \brief Retrieve a pointer to the packet header location.
        ///
        inline void * getHeader ();

        ///
        /// \brief Copy a packet header into a destination buffer.
        ///
        /// Packet::header_size bytes will be copied into the destination
        /// buffer.
        ///
        /// \param [in] dst Destination buffer for the header data
        ///
        inline void copyHeader (void * dst);

        ///
        /// \brief Write a packet header from a source buffer.
        ///
        /// Packet::header_size bytes will be copied from the source buffer.
        ///
        /// \param [in] src Source buffer for the header data
        ///
        inline void writeHeader (void * src);

        ///
        /// \brief Retrieve a pointer to the packet payload location.
        ///
        inline void * getPayload ();

        ///
        /// \brief Copy a packet payload into a destination buffer.
        ///
        /// Packet::payload_size bytes will be copied into the destination
        /// buffer.
        ///
        /// \param [in] dst Destination buffer for the payload data
        ///
        inline void copyPayload (void * dst);
    };

    template <class T_Packet>
    void Packet<T_Packet>::clear ()
    {
      return static_cast<T_Packet*>(this)->clear_impl ();
    }

    template <class T_Packet>
    void * Packet<T_Packet>::getHeader ()
    {
      return static_cast<T_Packet*>(this)->getHeader_impl ();
    }

    template <class T_Packet>
    void Packet<T_Packet>::copyHeader (void * dst)
    {
      static_cast<T_Packet*>(this)->copyHeader_impl (dst);
    }

    template <class T_Packet>
    void Packet<T_Packet>::writeHeader (void * src)
    {
      static_cast<T_Packet*>(this)->writeHeader_impl (src);
    }

    template <class T_Packet>
    void * Packet<T_Packet>::getPayload ()
    {
      return static_cast<T_Packet*>(this)->getPayload_impl ();
    }

    template <class T_Packet>
    void Packet<T_Packet>::copyPayload (void * dst)
    {
      static_cast<T_Packet*>(this)->copyPayload_impl (dst);
    }
  };
};
#undef TRACE
#endif // __util_fifo_Packet_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
