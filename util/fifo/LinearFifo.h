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

#ifndef __util_fifo_LinearFifo_h__
#define __util_fifo_LinearFifo_h__

#include <string.h>

#include "Arch.h"
#include "SysDep.h"

#include "components/atomic/Counter.h"

#include "util/fifo/Fifo.h"
#include "util/fifo/Packet.h"

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
                  T_Packet ()
              {
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
                size_t * header = (size_t *) T_Packet::getHeader ();
                header[(T_Packet::headerSize_impl / sizeof(size_t))-1] = active;
              };

              static const size_t public_header_bytes = T_Packet::headerSize_impl - sizeof(size_t);
          };
          
          typedef struct
          {
            size_t      active;
            T_Packet    packet;
          } linearfifo_packet_t;

        public:
          inline LinearFifo () :
              Fifo<LinearFifo <T_Atomic, T_Packet, T_FifoSize>, T_Packet> (),
              _head (0),
              _tail (),
              _inj_wrap_count (0),
              _last_rec_sequence (0)
          {
            fprintf (stderr, "LinearFifo() ... sizeof(T_Packet) = %zd, sizeof(LinearFifoPacket) = %zd\n", sizeof(T_Packet), sizeof(LinearFifoPacket));
          };

          inline ~LinearFifo () {};

          ///
          /// \brief Initialize the linear fifo with a specific packet buffer.
          ///
          inline void init_impl (SysDep & sysdep)
          {
            _head = 0;
            _tail.init (&sysdep);
            _tail.fetch_and_clear ();

            unsigned i;

            for (i = 0; i < T_FifoSize; i++)
            {
              _foo[i].active = 0;
              _foo[i].packet.clear ();
            }
          }

          inline T_Packet * nextInjPacket_impl (size_t & pktid)
          {
            pktid = _tail.fetch_and_inc ();
            TRACE_ERR((stderr, "(%zd) LinearFifo::nextInjPacket_impl() .. _tail.fetch_and_inc() => %zd, T_FifoSize = %d\n", DCMF_Messager_rank(), pktid, T_FifoSize));
            if (pktid < T_FifoSize)
              {
                // Set the packet sequence number at this time to avoid race
                // condition where a second packet is reserved after this
                // packet and then "produced" before this packet can be
                // produced.
                //_packet[index].setSequenceId (index + _inj_wrap_count * T_FifoSize);
                return (T_Packet *) &_foo[_head].packet;
              }

            return NULL;
          };

          inline T_Packet * nextRecPacket_impl ()
          {
            if (_foo[_head].active)
              return & _foo[_head].packet;

            return (T_Packet *) NULL;
          };

          inline void consumePacket_impl ()
          {
            //LinearFifoPacket * p = (LinearFifoPacket *) pkt;

            //p->consume ();
            _foo[_head].active = 0;
            _last_rec_sequence++;

            // If this packet is the last packet in the fifo, reset the tail
            // to the start of the fifo.
            //if (p == &_packet[T_FifoSize-1])
              //{
                //mem_sync ();
     //           _tail.fetch_and_clear ();
       //       }

            _head++;
            
            // If this packet is the last packet in the fifo, reset the tail
            // to the start of the fifo.
            if (_head == T_FifoSize)
            {
              _head = 0;
              mem_sync ();
              _tail.fetch_and_clear ();
            }
          };

          inline void producePacket_impl (size_t pktid)
          {
            _foo[pktid].active = 1;
//            LinearFifoPacket * p = (LinearFifoPacket *) pkt;
  //          p->produce ();
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
#if 0
          inline size_t getPacketSequenceId_impl (T_Packet * pkt)
          {
            LinearFifoPacket * p = (LinearFifoPacket *) pkt;
            return p->getSequenceId ();
          }
#endif
        protected:
          linearfifo_packet_t _foo[T_FifoSize];
//          T_Packet         _packet[T_FifoSize];
  //        size_t           _active[T_FifoSize];
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
