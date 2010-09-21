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

#include "components/memory/MemoryManager.h"
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
              TRACE_ERR((stderr, "%s: size %zu, fifosize %d, sizeof(LinearFifoPacket) %zu, sizeof(T_Atomic) %zu\n", __PRETTY_FUNCTION__,  sizeof(*this),  T_FifoSize, sizeof(LinearFifoPacket),sizeof(T_Atomic)));
#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
#else // !__pami_target_bgq__
              setActive (false);
#endif // !__pami_target_bgq__
            };

            inline ~LinearFifoPacket () {};

            inline void reset ()
            {
              T_Packet::clear();
#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
#else // !__pami_target_bgq__
              setActive (false);
#endif // !__pami_target_bgq__
            };

#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
            static const size_t public_header_bytes = T_Packet::headerSize_impl;
#else // !__pami_target_bgq__
            inline bool isActive ()
            {
              size_t * header = (size_t *) T_Packet::getHeader ();
              return header[(T_Packet::headerSize_impl / sizeof(size_t))-1];
            };
            inline void setActive (bool active)
            {
              TRACE_ERR((stderr, "(%zu) >> LinearFifoPacket::setActive(%d)\n", __global.mapping.task(), active));
              size_t * header = (size_t *) T_Packet::getHeader ();
              header[(T_Packet::headerSize_impl / sizeof(size_t))-1] = active;
              TRACE_ERR((stderr, "(%zu)    LinearFifoPacket::setActive(%d), header = %p, header[%zu] = %zu\n", __global.mapping.task(), active, header, (T_Packet::headerSize_impl / sizeof(size_t)) - 1, header[(T_Packet::headerSize_impl / sizeof(size_t))-1]));
              TRACE_ERR((stderr, "(%zu) << LinearFifoPacket::setActive(%d)\n", __global.mapping.task(), active));
            };

            static const size_t public_header_bytes = T_Packet::headerSize_impl - sizeof(size_t);
#endif // !__pami_target_bgq__
        };

      public:
        inline LinearFifo () :
            Fifo<LinearFifo <T_Atomic, T_Packet, T_FifoSize>, T_Packet> (),
            _head (0),
            _tail (),
#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
      _active(NULL),
#else // !__pami_target_bgq__
#endif // !__pami_target_bgq__
            _inj_wrap_count (0),
            _last_rec_sequence (0)
        {};

        inline ~LinearFifo () {};

        ///
        /// \brief Initialize the linear fifo with a specific packet buffer.
        ///
        inline void init_impl (size_t clientid)
        {
          _head = 0;
          _tail.init ();
          _tail.fetch_and_clear ();

          unsigned i;

          for (i = 0; i < T_FifoSize; i++)
            {
              _packet[i].reset ();
            }
#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
	// since we currently wakeup all contexts if any are changed, this is fine.
	__global._wuRegion_mm->memalign((void **)&_active, sizeof(void *),
					T_FifoSize * sizeof(*_active),
					NULL, PAMI::Memory::MemoryManager::memzero, NULL);
	PAMI_assertf(_active, "Out of WAC Region memory allocating FIFO active flags");
#endif // __pami_target_bgq__
        }

        inline T_Packet * nextInjPacket_impl (size_t & pktid)
        {
          TRACE_ERR((stderr, "(%zu) LinearFifo::nextInjPacket_impl() >>\n", __global.mapping.task()));
          pktid = _tail.fetch_and_inc ();
          TRACE_ERR((stderr, "(%zu) LinearFifo::nextInjPacket_impl() .. _tail.fetch_and_inc() => %zu, T_FifoSize = %d\n", __global.mapping.task(), pktid, T_FifoSize));

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
#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
    if (_active[_head])
#else // !__pami_target_bgq__
          TRACE_ERR((stderr, "(%zu) LinearFifo::nextRecPacket_impl() .. this = %p, _packet[%zu].isActive () = %d\n", __global.mapping.task(), this, _head, _packet[_head].isActive ()));
          if (_packet[_head].isActive ())
#endif // !__pami_target_bgq__
            return (T_Packet *) &_packet[_head];

          return (T_Packet *) NULL;
        };

        inline void consumePacket_impl ()
        {
#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
    _active[_head] = false;
#else // !__pami_target_bgq__
          TRACE_ERR((stderr, "(%zu) LinearFifo::consumePacket_impl() .. this = %p, _packet[%zu].isActive () = %d\n", __global.mapping.task(), this, _head, _packet[_head].isActive ()));
                    //mem_barrier ();
          //mem_sync();
          _packet[_head].setActive (false);
#endif // !__pami_target_bgq__
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
          TRACE_ERR((stderr, "(%zu) >> LinearFifo::producePacket_impl(%zu)\n", __global.mapping.task(), pktid));

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
#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
    _active[pktid] = true;
#else // !__pami_target_bgq__
          _packet[pktid].setActive (true);
#endif // !__pami_target_bgq__
          //mem_barrier ();
          //mem_sync();

          TRACE_ERR((stderr, "(%zu) << LinearFifo::producePacket_impl(%zu)\n", __global.mapping.task(), pktid));
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
#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
  volatile bool *_active;
#else // !__pami_target_bgq__
#endif // !__pami_target_bgq__

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
