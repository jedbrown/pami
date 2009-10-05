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

#ifndef __util_fifo_fifo_h__
#define __util_fifo_fifo_h__

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
          inline void init (XMI_SYSDEP_CLASS &sysdep);

          inline T_Packet * nextInjPacket ();
          inline T_Packet * nextRecPacket ();

          inline void consumePacket (T_Packet * pkt);
          inline void producePacket (T_Packet * pkt);

          inline size_t nextInjSequenceId ();
          inline size_t lastRecSequenceId ();
          inline size_t getPacketSequenceId (T_Packet * pkt);
      };

      template <class T_Fifo, class T_Packet>
      void Fifo<T_Fifo, T_Packet>::init (XMI_SYSDEP_CLASS &sysdep)
      {
        static_cast<T_Fifo*>(this)->init_impl (sysdep);
      }

      template <class T_Fifo, class T_Packet>
      T_Packet * Fifo<T_Fifo, T_Packet>::nextInjPacket ()
      {
        return static_cast<T_Fifo*>(this)->nextInjPacket_impl ();
      }

      template <class T_Fifo, class T_Packet>
      T_Packet * Fifo<T_Fifo, T_Packet>::nextRecPacket ()
      {
        return static_cast<T_Fifo*>(this)->nextRecPacket_impl ();
      }

      template <class T_Fifo, class T_Packet>
      void Fifo<T_Fifo, T_Packet>::consumePacket (T_Packet * pkt)
      {
        static_cast<T_Fifo*>(this)->consumePacket_impl (pkt);
      }

      template <class T_Fifo, class T_Packet>
      void Fifo<T_Fifo, T_Packet>::producePacket (T_Packet * pkt)
      {
        static_cast<T_Fifo*>(this)->producePacket_impl (pkt);
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

      template <class T_Fifo, class T_Packet>
      size_t Fifo<T_Fifo, T_Packet>::getPacketSequenceId (T_Packet * pkt)
      {
        return static_cast<T_Fifo*>(this)->getPacketSequenceId_impl (pkt);
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
