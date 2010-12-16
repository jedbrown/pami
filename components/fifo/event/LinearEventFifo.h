/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/fifo/event/LinearEventFifo.h
 * \brief ???
 */

#ifndef __components_fifo_event_LinearEventFifo_h__
#define __components_fifo_event_LinearEventFifo_h__

#include <pami.h>

#include "components/atomic/CounterInterface.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/fifo/EventFifoInterface.h"

#include "Arch.h"

namespace PAMI
{
  namespace Fifo
  {
    namespace Event
    {

      ///
      /// \todo Decide if the mutex class should be a template parameter
      ///
      /// \todo Add a 'stop event packet' to the end of the fifo to remove
      ///       'this' pointer accesses in the consume critical path
      ///
      template < class T_Atomic, unsigned T_Size>
      class Linear : public Interface < Linear<T_Atomic, T_Size> >
      {
        public:

          friend class Interface < Linear<T_Atomic, T_Size> >;

          typedef struct packet_t
          {
            pami_event_function   fn;
            void                * cookie;
            packet_t            * next; // only used in overflow list
            size_t                active;
          } packet_t  __attribute__((__aligned__(16)));

          static const size_t event_fifo_state_bytes = sizeof(packet_t);

          inline Linear () :
              Interface < Linear<T_Atomic, T_Size> > (),
              _head (&_packet[0]),
              _tail_index (),
              _overflow_tail_ptr (&_overflow_head)
          {
            // Do a compile-time-assert that the fifo length is a power of two
            //COMPILE_TIME_ASSERT(!(T_Size & (T_Size - 1)));

            _overflow_head.fn     = overflow_head_function;
            _overflow_head.cookie = NULL;
            _overflow_head.next   = NULL;
            _overflow_head.active = 0;

            size_t i;

            for (i = 0; i < T_Size; i++)
              {
                _packet[i].fn     = NULL;
                _packet[i].cookie = NULL;
                _packet[i].next   = NULL;
                _packet[i].active = 0;
              }

            _packet[T_Size-1].fn     = (pami_event_function) reset_event_function;
            _packet[T_Size-1].cookie = (void *) this;
            _packet[T_Size-1].next   = NULL;
            _packet[T_Size-1].active = 1;

          };

          inline ~Linear () {};

        protected:

          // ---------------------------------------------------------------------
          // PAMI::Fifo::Event::Interface interface implementation
          // ---------------------------------------------------------------------

          template <class T_MemoryManager>
          inline void initialize_impl (T_MemoryManager * mm, char * key)
          {
            // Initialize the tail_index atomic counter using the base key.
            char atomic_key[PAMI::Memory::MMKEYSIZE];
            snprintf (atomic_key, PAMI::Memory::MMKEYSIZE - 1, "%s-counter", key);
            _tail_index.init (mm, atomic_key);

            // Initialize the overflow mutex using the base key.
            char mutex_key[PAMI::Memory::MMKEYSIZE];
            snprintf (mutex_key, PAMI::Memory::MMKEYSIZE - 1, "%s-mutex", key);
            _overflow_mutex.init (mm, mutex_key);
          };

          template <typename T, unsigned N>
          inline bool produceEvent_impl (T                     (&state)[N],
                                         pami_event_function   fn,
                                         void                * cookie)
          {
            COMPILE_TIME_ASSERT(sizeof(packet_t) <= (sizeof(T)*N));

            //PAMI_assertf(fn!=NULL,"fn = %p !!\n", fn);

            size_t index = _tail_index.fetch_and_inc ();

            if (likely (index < T_Size - 1))
              {
                _packet[index].fn     = fn;
                _packet[index].cookie = cookie;
                _packet[index].active = 1;

                return true;
              }

            _overflow_mutex.acquire();

            // Now that the overflow mutex is aquired, check one more
            // time to make sure that the fifo wasn't cleared
            index = _tail_index.fetch_and_inc ();

            if (index < T_Size - 1)
              {
                _overflow_mutex.release();
                _packet[index].fn     = fn;
                _packet[index].cookie = cookie;
                _packet[index].active = 1;

                return true;
              }

            // The 'overflow tail pointer' will never be NULL because
            // a 'noop' element is always at the head of the list.
            // Because of this the 'tail pointer is null' empty list
            // logic can be removed from this critical path.
            //
            // See the reset_event_function(), invoked by the last
            // packet event in the event fifo, for more details.
            packet_t * event = (packet_t * ) state;
            event->fn        = fn;
            event->cookie    = cookie;
            event->next      = NULL;

            _overflow_tail_ptr->next = event;
            _overflow_tail_ptr = event;

            _overflow_mutex.release();

            return true;
          };

          ///
          /// \brief Consume (invoke) an event from the fifo
          ///
          inline bool consumeEvent_impl (pami_context_t context)
          {
            packet_t * head = _head;

            if (unlikely(head->active == 0)) return false;

            mem_sync();

            while (head->active)
              {
                head->active = 0;
                _head++;
//PAMI_assertf(head->fn!=NULL,"head->fn is NULL! .. head = %p, _packet = %p, _head = %p\n",head, _packet, _head);
                head->fn (context, head->cookie, PAMI_SUCCESS);

                head = _head;
              }

            return true;
          };

          ///
          /// \brief Debug 'noop' event function
          ///
          /// The overflow head 'noop' element is initialized to this 'safe'
          /// debug event function which should never be invoked.
          ///
          static void overflow_head_function (pami_context_t   context,
                                              void           * cookie,
                                              pami_result_t    result)
          {
            PAMI_abortf("ERROR - Should never get here.\n");
            return;
          };

          static void reset_event_function (pami_context_t   context,
                                            void           * cookie,
                                            pami_result_t    result)
          {
            PAMI::Fifo::Event::Linear<T_Atomic, T_Size> * fifo =
              (PAMI::Fifo::Event::Linear<T_Atomic, T_Size> *) cookie;

            // Clear the tail index. This allows other producers to write to
            // the, now empty, event fifo.
            fifo->_tail_index.clear();

            // Reset the last event packet in the fifo (this event) to active;
            // this allows the reset function to be executed on the _next_ fifo
            // wrap condition.
            fifo->_packet[T_Size-1].active = 1;

            // Reset the head event packet pointer to the begining of the event fifo.
            fifo->_head = fifo->_packet;

            // =========================
            // Begin overflow processing
            // =========================

            fifo->_overflow_mutex.acquire();

            // Return if there are no overflow events on the list.
            if (fifo->_overflow_head.next == NULL)
              {
                fifo->_overflow_mutex.release();
                return;
              }


            // The next event after the 'noop' head event is always the
            // first 'real' event to be processed
            packet_t * ptr = fifo->_overflow_head.next;

            // Reset the tail pointer and the 'noop' head event next pointer
            // By maintaining a 'noop' head event the linked list is never
            // empty and the 'head ptr is null' logic can be removed from
            // the critical path.
            fifo->_overflow_head.next = NULL;
            fifo->_overflow_tail_ptr = & (fifo->_overflow_head);

            // Events may be posted to the overflow list as soon as the
            // overflow mutex is relased
            fifo->_overflow_mutex.release();

            while (ptr != NULL)
              {
                // Cache the event fn and event cookie in case the execution
                // of the event function causes the memory used by the event
                // object, pointed to by 'ptr', is free'd and the 'next'
                // field has been altered.
                pami_event_function fn = ptr->fn;
                void * cookie = ptr->cookie;
                ptr = ptr->next;

                fn (context, cookie, PAMI_SUCCESS);
              }
          };

        private:

          packet_t     _packet[T_Size] __attribute__((__aligned__(16)));
          packet_t   * _head;
          T_Atomic     _tail_index;

          Mutex::IndirectCounter<T_Atomic> _overflow_mutex;
          packet_t                         _overflow_head;
          packet_t *                       _overflow_tail_ptr;
      };
    };
  };
};
#endif // __components_fifo_event_LinearEventFifo_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
