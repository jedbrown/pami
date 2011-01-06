/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/fifo/wrap/WrapFifo.h
 * \brief ???
 */

#ifndef __components_fifo_wrap_WrapFifo_h__
#define __components_fifo_wrap_WrapFifo_h__

#include <string.h>

#include "Arch.h"

#include "components/memory/MemoryManager.h"
#include "components/atomic/CounterInterface.h"

#include "components/fifo/FifoInterface.h"
#include "components/fifo/PacketInterface.h"

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif
#define FIFO_FULL 0x8000000000000000
namespace PAMI
{
  namespace Fifo
  {
    template < class T_Packet, class T_Atomic, unsigned T_Size = 128 >
      class WrapFifo : public PAMI::Fifo::Interface::Fifo <PAMI::Fifo::WrapFifo <T_Packet, T_Atomic,T_Size> >
    {
      public:

        typedef T_Packet Packet;

        friend class Interface::Fifo <WrapFifo <T_Packet, T_Atomic, T_Size> >;

        static const size_t mask = T_Size - 1;
        static const size_t packet_header_size = T_Packet::header_size;
        static const size_t packet_payload_size = T_Packet::payload_size;

        inline WrapFifo () :
          Interface::Fifo <WrapFifo <T_Packet, T_Atomic, T_Size> > (),
          _packet (NULL),
          _active (NULL),
          _head (NULL),
          _bounded_counter (),
          _last_packet_produced (0)
      {
        // How to compile-time-assert that the fifo length is a power of two?
        //
        // The following code is pretty lame .. but it works.
        COMPILE_TIME_ASSERT((T_Size==1)||(T_Size==2)||(T_Size==4)||(T_Size==8)||(T_Size==16)||(T_Size==32)||(T_Size==64)||(T_Size==128)||(T_Size==256)||(T_Size==1024)||(T_Size==2048)||(T_Size==4096)||(T_Size==8192)||(T_Size==16384)||(T_Size==32768));
      };

        inline ~WrapFifo () {};

      protected:

        // ---------------------------------------------------------------------
        // PAMI::Fifo::Fifo interface implementation
        // ---------------------------------------------------------------------

        inline void initialize_impl (PAMI::Memory::MemoryManager * mm,
            char                        * key)
        {
          TRACE_ERR((stderr, ">> WrapFifo::initialize_impl(%p, \"%s\")\n", mm, key));

          // Initialize the tail atomic counter using the base key.
          char atomic_key[PAMI::Memory::MMKEYSIZE];
          snprintf (atomic_key, PAMI::Memory::MMKEYSIZE - 1, "%s-counter", key);
          _bounded_counter.init (mm, atomic_key);
          _bounded_counter.set_fifo_bounds(0, T_Size);

          TRACE_ERR((stderr, "   WrapFifo::initialize_impl() after atomic init, before sync memalign\n"));

          // Allocate an array of packets with the provided memory manager
          // and key. The WrapFifo::packet_initialize() function is only
          // invoked by the first process to allocate from the memory manager
          // with this key. All other processes do not return from the
          // memalign until the first process has completed the memory
          // initialization.
          //
          // Allocation is for N packets, N active flags, 1 head counter,
          // and 1 wrap counter.

          mm->memalign ((void **)&_packet,
              sizeof(T_Packet),
              (sizeof(T_Packet) + sizeof(size_t)) * T_Size +
              sizeof(size_t) * 2,
              key,
              WrapFifo::packet_initialize,
              NULL);

          TRACE_ERR((stderr, "   WrapFifo::initialize_impl() after sync memalign\n"));

          _active = (size_t *) & _packet[T_Size];
          _head = &_active[T_Size];
          *(_head) = 0;

          TRACE_ERR((stderr, "<< WrapFifo::initialize_impl(%p, \"%s\"), _active = %p, _head = %p, *_head = %zu\n", mm, key, _active, _head, *_head));
        };

        inline void initialize_impl (WrapFifo<T_Packet, T_Atomic> & fifo)
        {


          //PAMI_abortf("This type of initialization is not yet implemented\n");
          TRACE_ERR((stderr, ">> WrapFifo::initialize_impl(WrapFifo &)\n"));
          _bounded_counter.clone (fifo._bounded_counter);

          _packet = fifo._packet;
          _active = fifo._active;
          _head = fifo._head;
          _last_packet_produced = fifo._last_packet_produced;

          TRACE_ERR((stderr, "<< WrapFifo::initialize_impl(WrapFifo &), _packet = %p, _active = %p, _head = %p, *_head = %zu, _last_packet_produced = %zu\n", _packet, _active, _head, *_head, _last_packet_produced));
        };

        inline void dumpPacket (size_t index)
        {
          char tmp[10240];
          char * str = tmp;
          size_t i = 0;

          uint32_t * hdr = (uint32_t *) _packet[index].getHeader();
          str += sprintf(str, "WrapFifo::dumpPacket.header  [%p,%4zu] ", hdr, packet_header_size_impl);
          size_t bytes = 0;

          while (bytes < packet_header_size_impl)
          {
            str += sprintf(str, "%08x ", hdr[i++]);
            bytes += sizeof(uint32_t);

            if (i % 4 == 0)
              str += sprintf(str, "\nWrapFifo::dumpPacket.header  [%p]      ", &hdr[i]);
          }

          fprintf(stderr, "%s\n", tmp);

          str = tmp;
          uint32_t * payload = (uint32_t *) _packet[index].getPayload();
          str += sprintf(str, "WrapFifo::dumpPacket.payload [%p,%4zu] ", payload, packet_payload_size_impl);
          bytes = 0;
          i = 0;

          while (bytes < packet_payload_size_impl)
          {
            str += sprintf(str, "%08x ", payload[i++]);
            bytes += sizeof(uint32_t);

            if (i % 4 == 0)
              str += sprintf(str, "\nWrapFifo::dumpPacket.payload [%p]      ", &payload[i]);
          }

          fprintf(stderr, "%s\n", tmp);
        };

        static const size_t packet_header_size_impl = T_Packet::header_size;

        static const size_t packet_payload_size_impl = T_Packet::payload_size;

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
            TRACE_ERR((stderr, ">> WrapFifo::producePacket_impl(T_Producer &)\n"));

            uint64_t tail = this->_bounded_counter.fetch_and_inc_bounded ();

            //TRACE_ERR((stderr, "   WrapFifo::producePacket_impl(T_Producer &), index = %zu\n", index));

            if (likely (tail != FIFO_FULL))
            {

              size_t  index =	tail & WrapFifo::mask;
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

              //_last_packet_produced = index;
              _last_packet_produced = tail;

              TRACE_ERR((stderr, "<< WrapFifo::producePacket_impl(T_Producer &), return true\n"));
              return true;
            }

            TRACE_ERR((stderr, "<< WrapFifo::producePacket_impl(T_Producer &), return false\n"));
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
            TRACE_ERR((stderr, ">> WrapFifo::consumePacket_impl(T_Consumer &)\n"));

            const size_t head = *(this->_head);
            size_t index = head & WrapFifo::mask;

            TRACE_ERR((stderr, "   WrapFifo::consumePacket_impl(T_Consumer &), head = %zu, index = %zu (WrapFifo::mask = %p)\n", head, index, (void *)WrapFifo::mask));
            if (_active[index] == 1)
            {
              //dumpPacket(head);
              packet.consume (_packet[index]);
              //dumpPacket(head);

              _active[index] = 0;
              *(this->_head) = head + 1;

              //increment the upper bound everytime a packet is consumed..ok to be incremented in chunks
              this->_bounded_counter.fetch_and_inc_upper_bound();
#if 0
              // If this packet is the last packet in the fifo, reset the tail
              // to the start of the fifo.
              if (index == (T_Size - 1))
              {
                mem_barrier();
                _bounded_counter.clear();
              }
#endif
              TRACE_ERR((stderr, "<< WrapFifo::consumePacket_impl(T_Consumer &), return true\n"));
              return true;
            }

            TRACE_ERR((stderr, "<< WrapFifo::consumePacket_impl(T_Consumer &), return false\n"));
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
          TRACE_ERR((stderr, ">> WrapFifo::packet_initialize(%p, %zu, \"%s\", %d, %p)\n", memory, bytes, key, attributes, cookie));
          T_Packet * packet = (T_Packet *) memory;
          size_t * active = (size_t *) & packet[T_Size];

          size_t i;

          for (i = 0; i < T_Size; i++)
          {
            new (&packet[i]) T_Packet();
            active[i] = 0;
          }
          TRACE_ERR((stderr, "<< WrapFifo::packet_initialize(%p, %zu, \"%s\", %d, %p)\n", memory, bytes, key, attributes, cookie));
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
        T_Atomic       _bounded_counter;
        size_t         _last_packet_produced;
    };

  };
};
#undef TRACE_ERR
#endif // __util_fifo_wrapfifo_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
