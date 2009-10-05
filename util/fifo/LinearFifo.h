/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/fifo/LinearFifo.h
 * \brief ???
 */

#ifndef __util_fifo_linearfifo_h__
#define __util_fifo_linearfifo_h__

#include <string.h>

#include "components/atomic/Counter.h"

#include "Fifo.h"
#include "Packet.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace XMI
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
                  T_Packet (),
                  _active (0),
                  _sequence (0)
              {};

              inline ~LinearFifoPacket () {};

              inline void reset ()
              {
                _active = 0;
                T_Packet::clear();
              };

              inline bool isActive () { return _active; };
              inline void consume () { _active = 0; };
              inline void produce () { _active = 1; };

              inline void setSequenceId (size_t n) { _sequence = n; }
              inline size_t getSequenceId () { return _sequence; }

            protected:
              size_t _active __attribute__ ((aligned (16)));
              size_t _sequence;
          };

        public:
          inline LinearFifo () :
              Fifo<LinearFifo <T_Atomic, T_Packet, T_FifoSize>, T_Packet> (),
            //  _packet (NULL),
              _head (0),
              _tail (),
              _inj_wrap_count (0),
              _last_rec_sequence (0)
          {};

          inline ~LinearFifo () {};

          ///
          /// \brief Initialize the linear fifo with a specific packet buffer.
          ///
          inline void init_impl (XMI_SYSDEP_CLASS &sysdep)
          {
//            _npackets = bytes / sizeof(LinearFifoPacket);

//            size_t size = sizeof(LinearFifoPacket) * T_FifoSize;

  //          TRACE_ERR((stderr, "(%zd) LinearFifo() .. before scratchpad_dynamic_area_memalign()\n", DCMF_Messager_rank()));
    //        void * tmp = sysdep.memoryManager.scratchpad_dynamic_area_memalign(16, size);
 //           _packet = (LinearFifoPacket *) addr;

            _head = 0;
            _tail.init (&sysdep);
            _tail.fetch_and_clear ();

            unsigned i;

            for (i = 0; i < T_FifoSize; i++)
              _packet[i].reset ();
          }

          inline T_Packet * nextInjPacket_impl ()
          {
            size_t index = _tail.fetch_and_inc ();
            TRACE_ERR((stderr, "(%zd) LinearFifo::nextInjPacket_impl() .. _tail.fetch_and_inc() => %zd, T_FifoSize = %d, &_packet[0] = %p\n", DCMF_Messager_rank(), index, T_FifoSize, &_packet[0]));
            if (index < T_FifoSize)
              {
                // Set the packet sequence number at this time to avoid race
                // condition where a second packet is reserved after this
                // packet and then "produced" before this packet can be
                // produced.
                _packet[index].setSequenceId (index + _inj_wrap_count * T_FifoSize);
                return (T_Packet *) &_packet[index];
              }

            return NULL;
          };

          inline T_Packet * nextRecPacket_impl ()
          {
            T_Packet * pkt = NULL;

            if (_packet[_head].isActive())
              {
                pkt = (T_Packet *) & _packet[_head];
                _head++;

                if (_head == T_FifoSize) _head = 0;
              }

            return pkt;
          };

          inline void consumePacket_impl (T_Packet * pkt)
          {
            LinearFifoPacket * p = (LinearFifoPacket *) pkt;

            p->consume ();
            _last_rec_sequence++;

            // If this packet is the last packet in the fifo, reset the tail
            // to the start of the fifo.
            if (p == &_packet[T_FifoSize-1])
              {
                _tail.fetch_and_clear ();
              }
          };

          inline void producePacket_impl (T_Packet * pkt)
          {
            LinearFifoPacket * p = (LinearFifoPacket *) pkt;
            p->produce ();
            mem_barrier ();
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

          inline size_t getPacketSequenceId_impl (T_Packet * pkt)
          {
            LinearFifoPacket * p = (LinearFifoPacket *) pkt;
            return p->getSequenceId ();
          }

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
