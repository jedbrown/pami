/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
          Fifo () {};
          ~Fifo () {};

          ///
          /// \brief Initialize the fifo
          ///
          inline void init (SysDep &sysdep);

          inline T_Packet * nextInjPacket (size_t & pktid);
          inline T_Packet * nextRecPacket ();

          inline void consumePacket ();
          inline void producePacket (size_t pktid);

          inline size_t nextInjSequenceId ();
          inline size_t lastRecSequenceId ();
#if 0
          inline size_t getPacketSequenceId (T_Packet * pkt);
#endif
      };

      template <class T_Fifo, class T_Packet>
      void Fifo<T_Fifo, T_Packet>::init (SysDep &sysdep)
      {
        static_cast<T_Fifo*>(this)->init_impl (sysdep);
      }

      template <class T_Fifo, class T_Packet>
      T_Packet * Fifo<T_Fifo, T_Packet>::nextInjPacket (size_t & pktid)
      {
        return static_cast<T_Fifo*>(this)->nextInjPacket_impl (pktid);
      }

      template <class T_Fifo, class T_Packet>
      T_Packet * Fifo<T_Fifo, T_Packet>::nextRecPacket ()
      {
        return static_cast<T_Fifo*>(this)->nextRecPacket_impl ();
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
#if 0
      template <class T_Fifo, class T_Packet>
      size_t Fifo<T_Fifo, T_Packet>::getPacketSequenceId (T_Packet * pkt)
      {
        return static_cast<T_Fifo*>(this)->getPacketSequenceId_impl (pkt);
      }
#endif
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
