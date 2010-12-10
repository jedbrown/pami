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
#include "components/atomic/CounterInterface.h"

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
    template < class T_Packet, class T_Atomic, unsigned T_Size = 128 >
    class LinearFifo : public Fifo <LinearFifo <T_Packet, T_Atomic, T_Size> >
    {
      public:

        typedef T_Packet Packet;

        friend class Fifo <LinearFifo <T_Packet, T_Atomic, T_Size> >;

        static const size_t mask = T_Size - 1;

        static const size_t packet_header_size = T_Packet::header_size;

        static const size_t packet_payload_size = T_Packet::payload_size;

        inline LinearFifo () :
            Fifo <LinearFifo <T_Packet, T_Atomic, T_Size> > (),
            _packet (NULL),
            _active (NULL),
            _head (NULL),
            _tail (),
            _last_packet_produced (0)
        {
          // Do a compile-time-assert that the fifo length is a power of two
          COMPILE_TIME_ASSERT(!(T_Size & (T_Size - 1)));
        };

        inline ~LinearFifo () {};

      protected:

        // ---------------------------------------------------------------------
        // PAMI::Fifo::Fifo interface implementation
        // ---------------------------------------------------------------------

        template <class T_MemoryManager>
        inline void initialize_impl (T_MemoryManager * mm,
                                     char            * key)
        {
          TRACE_ERR((stderr, ">> LinearFifo::initialize_impl(%p, \"%s\")\n", mm, key));

          // Initialize the tail atomic counter using the base key.
          char atomic_key[PAMI::Memory::MMKEYSIZE];
          snprintf (atomic_key, PAMI::Memory::MMKEYSIZE - 1, "%s-counter", key);
          _tail.init (mm, atomic_key);

          TRACE_ERR((stderr, "   LinearFifo::initialize_impl() after atomic init, before sync memalign\n"));

          // Allocate an array of packets with the provided memory manager
          // and key. The LinearFifo::packet_initialize() function is only
          // invoked by the first process to allocate from the memory manager
          // with this key. All other processes do not return from the
          // memalign until the first process has completed the memory
          // initialization.
          //
          // Allocation is for N packets, N active flags, 1 head counter,
          // and 1 wrap counter.

          size_t total_size = (sizeof(T_Packet) + sizeof(size_t)) * T_Size + sizeof(size_t) * 2;
          TRACE_ERR((stderr, "   LinearFifo::initialize_impl() before sync memalign, key = '%s', total size to allocate = %zu\n", key, total_size));

          pami_result_t rc;
          rc = mm->memalign ((void **)&_packet,
                             sizeof(T_Packet),
                             total_size,
                             key,
                             LinearFifo::packet_initialize,
                             NULL);
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to allocate LinearFifo shared memory resources");

          TRACE_ERR((stderr, "   LinearFifo::initialize_impl() after sync memalign\n"));

          _active = (size_t *) & _packet[T_Size];
          _head = &_active[T_Size];
          *(_head) = 0;

          TRACE_ERR((stderr, "<< LinearFifo::initialize_impl(%p, \"%s\"), _active = %p, _head = %p, *_head = %zu\n", mm, key, _active, _head, *_head));
        };

        inline void initialize_impl (LinearFifo<T_Packet, T_Atomic, T_Size> & fifo)
        {
          TRACE_ERR((stderr, ">> LinearFifo::initialize_impl(LinearFifo &)\n"));
          _tail.clone (fifo._tail);

          _packet = fifo._packet;
          _active = fifo._active;
          _head = fifo._head;
          _last_packet_produced = fifo._last_packet_produced;

          TRACE_ERR((stderr, "<< LinearFifo::initialize_impl(LinearFifo &), _packet = %p, _active = %p, _head = %p, *_head = %zu, _last_packet_produced = %zu\n", _packet, _active, _head, *_head, _last_packet_produced));
        };

        inline void dumpPacket (size_t index)
        {
          char tmp[10240];
          char * str = tmp;
          size_t i = 0;

          uint32_t * hdr = (uint32_t *) _packet[index].getHeader();
          str += sprintf(str, "LinearFifo::dumpPacket.header  [%p,%4zu] ", hdr, packet_header_size);
          size_t bytes = 0;

          while (bytes < packet_header_size)
            {
              str += sprintf(str, "%08x ", hdr[i++]);
              bytes += sizeof(uint32_t);

              if (i % 4 == 0)
                str += sprintf(str, "\nLinearFifo::dumpPacket.header  [%p]      ", &hdr[i]);
            }

          fprintf(stderr, "%s\n", tmp);

          str = tmp;
          uint32_t * payload = (uint32_t *) _packet[index].getPayload();
          str += sprintf(str, "LinearFifo::dumpPacket.payload [%p,%4zu] ", payload, packet_payload_size);
          bytes = 0;
          i = 0;

          while (bytes < packet_payload_size)
            {
              str += sprintf(str, "%08x ", payload[i++]);
              bytes += sizeof(uint32_t);

              if (i % 4 == 0)
                str += sprintf(str, "\nLinearFifo::dumpPacket.payload [%p]      ", &payload[i]);
            }

          fprintf(stderr, "%s\n", tmp);
        };

        inline size_t lastPacketProduced_impl ()
        {
          return _last_packet_produced;
        };

        inline size_t lastPacketConsumed_impl ()
        {
          return *_head - 1;
        };

        ///
        /// \param [in] packet Functor object that implements the PacketProducer interface
        ///
        /// \see PAMI::Fifo::Interface::PacketProducer
        ///
        template <class T_Producer>
        inline bool producePacket_impl (T_Producer & packet)
        {
          TRACE_ERR((stderr, ">> LinearFifo::producePacket_impl(T_Producer &)\n"));

          size_t index = _tail.fetch_and_inc ();

          TRACE_ERR((stderr, "   LinearFifo::producePacket_impl(T_Producer &), index = %zu\n", index));

          if (likely (index < T_Size))
            {
              //dumpPacket(index);
              packet.produce (_packet[index]);
              //dumpPacket(index);

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
              mem_barrier();
              _active[index] = 1;
              //mem_barrier();

              _last_packet_produced = index;

              TRACE_ERR((stderr, "<< LinearFifo::producePacket_impl(T_Producer &), return true\n"));
              return true;
            }

          TRACE_ERR((stderr, "<< LinearFifo::producePacket_impl(T_Producer &), return false\n"));
          return false;
        };

        ///
        /// \param [in] packet Functor object that implements the PacketConsumer interface
        ///
        /// \see PAMI::Fifo::Interface::PacketConsumer
        ///
        template <class T_Consumer>
        inline bool consumePacket_impl (T_Consumer & packet)
        {
          TRACE_ERR((stderr, ">> LinearFifo::consumePacket_impl(T_Consumer &)\n"));

          const size_t head = *(this->_head);
          size_t index = head & LinearFifo::mask;

          TRACE_ERR((stderr, "   LinearFifo::consumePacket_impl(T_Consumer &), head = %zu, index = %zu (LinearFifo::mask = %p)\n", head, index, (void *)LinearFifo::mask));
          if (_active[index] == 1)
            {
              //dumpPacket(head);
              packet.consume (_packet[index]);
              //dumpPacket(head);

              _active[index] = 0;
              *(this->_head) = head + 1;

              // If this packet is the last packet in the fifo, reset the tail
              // to the start of the fifo.
              if (index == (T_Size - 1))
                {
                  mem_barrier();
                  _tail.clear();
                }

              TRACE_ERR((stderr, "<< LinearFifo::consumePacket_impl(T_Consumer &), return true\n"));
              return true;
            }

          TRACE_ERR((stderr, "<< LinearFifo::consumePacket_impl(T_Consumer &), return false\n"));
          return false;
        };

      private:

        ///
        /// \brief Initialize the packet resources
        ///
        /// \see PAMI::Memory::MM_INIT_FN
        ///
        static void packet_initialize (void       * memory,
                                       size_t       bytes,
                                       const char * key,
                                       unsigned     attributes,
                                       void       * cookie)
        {
          TRACE_ERR((stderr, ">> LinearFifo::packet_initialize(%p, %zu, \"%s\", %d, %p)\n", memory, bytes, key, attributes, cookie));
          T_Packet * packet = (T_Packet *) memory;
          size_t * active = (size_t *) & packet[T_Size];

          size_t i;

          for (i = 0; i < T_Size; i++)
            {
              new (&packet[i]) T_Packet();
              active[i] = 0;
            }
          TRACE_ERR((stderr, "<< LinearFifo::packet_initialize(%p, %zu, \"%s\", %d, %p)\n", memory, bytes, key, attributes, cookie));
        }

        // -----------------------------------------------------------------
        // Located in shared memory
        // -----------------------------------------------------------------
        T_Packet     * _packet;
        size_t       * _active;
        size_t       * _head;

        // -----------------------------------------------------------------
        // Located in-place
        // -----------------------------------------------------------------
        T_Atomic       _tail;
        size_t         _last_packet_produced;
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
