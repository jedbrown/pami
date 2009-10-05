/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/fifo/Packet.h
 * \brief ???
 */

#ifndef __util_fifo_packet_h__
#define __util_fifo_packet_h__

#ifndef TRACE
#define TRACE(x)
#endif

namespace XMI
{
    namespace Fifo
    {
      template <class T_Packet>
      class Packet
      {
        public:
          Packet () {};
          ~Packet () {};

          inline void clear ();

          inline void * getHeader ();
          size_t headerSize ();
          inline void copyHeader (void * addr);

          inline void * getPayload ();
          size_t payloadSize ();
          inline void copyPayload (void * addr);
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
      size_t Packet<T_Packet>::headerSize ()
      {
        return T_Packet::headerSize_impl;
      }

      template <class T_Packet>
      void Packet<T_Packet>::copyHeader (void * addr)
      {
        static_cast<T_Packet*>(this)->copyHeader_impl (addr);
      }

      template <class T_Packet>
      void * Packet<T_Packet>::getPayload ()
      {
        return static_cast<T_Packet*>(this)->getPayload_impl ();
      }

      template <class T_Packet>
      size_t Packet<T_Packet>::payloadSize ()
      {
        return T_Packet::payloadSize_impl;
      }

      template <class T_Packet>
      void Packet<T_Packet>::copyPayload (void * addr)
      {
        static_cast<T_Packet*>(this)->copyPayload_impl (addr);
      }
  };
};
#undef TRACE
#endif // __util_fifo_packet_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
