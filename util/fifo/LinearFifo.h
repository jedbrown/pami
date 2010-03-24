/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/fifo/LinearFifo.h
 * \brief ???
 */

#ifndef __util_fifo_LinearFifo_h__
#define __util_fifo_LinearFifo_h__

#include <string.h>

#include "Arch.h"
#include "SysDep.h"

#include "components/atomic/Counter.h"

#include "util/fifo/Fifo.h"
#include "util/fifo/Packet.h"

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace PAMI
{
  namespace Fifo
  {
    template <class T_Atomic, class T_Packet, unsigned T_FifoSize>
    class LinearFifo : public Fifo<LinearFifo <T_Atomic, T_Packet, T_FifoSize>, T_Packet >
    {
      private:
        class LinearFifoPacket : public T_Packet
        {
          public:
            inline LinearFifoPacket () :
                T_Packet ()
            {
              TRACE_ERR((stderr, "%s: size %zd, fifosize %d, sizeof(LinearFifoPacket) %zd, sizeof(T_Atomic) %zd\n", __PRETTY_FUNCTION__,  sizeof(*this),  T_FifoSize, sizeof(LinearFifoPacket),sizeof(T_Atomic)));
              setActive (false);
            };

            inline ~LinearFifoPacket () {};

            inline void reset ()
            {
              T_Packet::clear();
              setActive (false);
            };

            inline bool isActive ()
            {
              size_t * header = (size_t *) T_Packet::getHeader ();
              return header[(T_Packet::headerSize_impl / sizeof(size_t))-1];
            };
            inline void setActive (bool active)
            {
              TRACE_ERR((stderr, "(%zd) >> LinearFifoPacket::setActive(%d)\n", __global.mapping.task(), active));
              size_t * header = (size_t *) T_Packet::getHeader ();
              header[(T_Packet::headerSize_impl / sizeof(size_t))-1] = active;
              TRACE_ERR((stderr, "(%zd)    LinearFifoPacket::setActive(%d), header = %p, header[%zd] = %zd\n", __global.mapping.task(), active, header, (T_Packet::headerSize_impl / sizeof(size_t)) - 1, header[(T_Packet::headerSize_impl / sizeof(size_t))-1]));
              TRACE_ERR((stderr, "(%zd) << LinearFifoPacket::setActive(%d)\n", __global.mapping.task(), active));
            };

            static const size_t public_header_bytes = T_Packet::headerSize_impl - sizeof(size_t);
        };

      public:
        inline LinearFifo () :
            Fifo<LinearFifo <T_Atomic, T_Packet, T_FifoSize>, T_Packet> (),
            _head (0),
            _tail (),
            _inj_wrap_count (0),
            _last_rec_sequence (0)
        {};

        inline ~LinearFifo () {};

        ///
        /// \brief Initialize the linear fifo with a specific packet buffer.
        ///
        inline void init_impl (Memory::MemoryManager *mm)
        {
          _head = 0;
          _tail.init (mm);
          _tail.fetch_and_clear ();

          unsigned i;

          for (i = 0; i < T_FifoSize; i++)
            {
              _packet[i].reset ();
            }
        }

        inline T_Packet * nextInjPacket_impl (size_t & pktid)
        {
          TRACE_ERR((stderr, "(%zd) LinearFifo::nextInjPacket_impl() >>\n", __global.mapping.task()));
          pktid = _tail.fetch_and_inc ();
          TRACE_ERR((stderr, "(%zd) LinearFifo::nextInjPacket_impl() .. _tail.fetch_and_inc() => %zd, T_FifoSize = %d\n", __global.mapping.task(), pktid, T_FifoSize));

          if (pktid < T_FifoSize)
            {
              return (T_Packet *) &_packet[pktid];
            }

          return NULL;
        };

        inline T_Packet * nextRecPacket_impl ()
        {
          //mem_barrier ();
          //mem_sync();
          TRACE_ERR((stderr, "(%zd) LinearFifo::nextRecPacket_impl() .. this = %p, _packet[%zd].isActive () = %d\n", __global.mapping.task(), this, _head, _packet[_head].isActive ()));
          if (_packet[_head].isActive ())
            return (T_Packet *) &_packet[_head];

          return (T_Packet *) NULL;
        };

        inline void consumePacket_impl ()
        {
          TRACE_ERR((stderr, "(%zd) LinearFifo::consumePacket_impl() .. this = %p, _packet[%zd].isActive () = %d\n", __global.mapping.task(), this, _head, _packet[_head].isActive ()));
                    //mem_barrier ();
          //mem_sync();
          _packet[_head].setActive (false);
          _last_rec_sequence++;

          _head++;

          // If this packet is the last packet in the fifo, reset the tail
          // to the start of the fifo.
          if (_head == T_FifoSize)
            {
              _head = 0;
              //mem_sync ();
                  mem_barrier ();
              _tail.fetch_and_clear ();
            }
          //mem_barrier ();
          //mem_sync();
        };

        inline void producePacket_impl (size_t pktid)
        {
          TRACE_ERR((stderr, "(%zd) >> LinearFifo::producePacket_impl(%zd)\n", __global.mapping.task(), pktid));

          // This memory barrier forces all previous memory operations to
          // complete (header writes, payload write, etc) before the packet is
          // marked 'active'.  As soon as the receiving process sees that the
          // 'active' attribute is set it will start to read the packet header
          // and payload data.
          //
          // If this memory barrier is done *after* the packet is marked
          // 'active', then the processor or memory system may still reorder
          // any pending writes before the barrier, which could result in the
          // receiving process reading the 'active' attribute and then reading
          // stale packet header/payload data.
          mem_barrier ();
          //mem_sync();
          _packet[pktid].setActive (true);
          //mem_barrier ();
          //mem_sync();

          TRACE_ERR((stderr, "(%zd) << LinearFifo::producePacket_impl(%zd)\n", __global.mapping.task(), pktid));
        };

        inline size_t nextInjSequenceId_impl ()
        {
          size_t index = _tail.fetch ();

          if (index < T_FifoSize)
            return index + _inj_wrap_count * T_FifoSize;

          return (_inj_wrap_count + 1) * T_FifoSize;
        }

        inline size_t lastRecSequenceId_impl ()
        {
          return _last_rec_sequence;
        }

        static const size_t packet_header_size = T_Packet::headerSize_impl - sizeof(size_t);
        static const size_t packet_payload_size = T_Packet::payloadSize_impl;

      protected:
        LinearFifoPacket _packet[T_FifoSize];
        size_t           _head;
        T_Atomic         _tail;

        size_t           _inj_wrap_count;
        size_t           _last_rec_sequence;
    };
  };
};
#undef TRACE_ERR
#endif // __util_fifo_linearfifo_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
