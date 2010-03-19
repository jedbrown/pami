/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/fifo/Fifo.h
 * \brief ???
 */

#ifndef __util_fifo_Fifo_h__
#define __util_fifo_Fifo_h__

#include "SysDep.h"

#ifndef TRACE
#define TRACE(x)
#endif

namespace XMI
{
  namespace Fifo
  {
    template <class T_Fifo, class T_Packet>
    class Fifo
    {
      public:

        typedef T_Packet PacketObject;

        Fifo ()
        {
          getPacketHeaderSize ();
          getPacketPayloadSize ();
        };

        ~Fifo () {};

        ///
        /// \brief Initialize the fifo
        ///
        inline void init (Memory::MemoryManager *mm);

        inline size_t getPacketHeaderSize ();
        inline size_t getPacketPayloadSize ();

        inline T_Packet * nextInjPacket (size_t & pktid);
        inline T_Packet * nextRecPacket ();

        inline void consumePacket ();
        inline void producePacket (size_t pktid);

        inline size_t nextInjSequenceId ();
        inline size_t lastRecSequenceId ();
    };

    template <class T_Fifo, class T_Packet>
    void Fifo<T_Fifo, T_Packet>::init (Memory::MemoryManager *mm)
    {
      static_cast<T_Fifo*>(this)->init_impl (mm);
    }

    template <class T_Fifo, class T_Packet>
    size_t Fifo<T_Fifo, T_Packet>::getPacketHeaderSize ()
    {
      return T_Fifo::packet_header_size;
    }

    template <class T_Fifo, class T_Packet>
    size_t Fifo<T_Fifo, T_Packet>::getPacketPayloadSize ()
    {
      return T_Fifo::packet_payload_size;
    }

    template <class T_Fifo, class T_Packet>
    T_Packet * Fifo<T_Fifo, T_Packet>::nextInjPacket (size_t & pktid)
    {
      return (T_Packet *) static_cast<T_Fifo*>(this)->nextInjPacket_impl (pktid);
    }

    template <class T_Fifo, class T_Packet>
    T_Packet * Fifo<T_Fifo, T_Packet>::nextRecPacket ()
    {
      return (T_Packet *) static_cast<T_Fifo*>(this)->nextRecPacket_impl ();
    }

    template <class T_Fifo, class T_Packet>
    void Fifo<T_Fifo, T_Packet>::consumePacket ()
    {
      static_cast<T_Fifo*>(this)->consumePacket_impl ();
    }

    template <class T_Fifo, class T_Packet>
    void Fifo<T_Fifo, T_Packet>::producePacket (size_t pktid)
    {
      static_cast<T_Fifo*>(this)->producePacket_impl (pktid);
    }

    template <class T_Fifo, class T_Packet>
    size_t Fifo<T_Fifo, T_Packet>::nextInjSequenceId ()
    {
      return static_cast<T_Fifo*>(this)->nextInjSequenceId_impl ();
    }

    template <class T_Fifo, class T_Packet>
    size_t Fifo<T_Fifo, T_Packet>::lastRecSequenceId ()
    {
      return static_cast<T_Fifo*>(this)->lastRecSequenceId_impl ();
    }
  };
};
#endif // __util_fifo_fifo_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
