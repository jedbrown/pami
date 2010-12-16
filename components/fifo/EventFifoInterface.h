/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/fifo/EventFifoInterface.h
 * \brief ???
 */

#ifndef __components_fifo_EventFifoInterface_h__
#define __components_fifo_EventFifoInterface_h__

#include <pami.h>

namespace PAMI
{
  namespace Fifo
  {
    namespace Event
    {
      ///
      /// \brief Event fifo interface class
      ///
      /// \tparam T_Fifo Event fifo implementation class
      ///
      template <class T_Fifo>
      class Interface
      {
        protected:

          Interface () {};

          ~Interface () {};

        public:

          ///
          /// \brief Returns the transfer state bytes attribute.
          ///
          /// Typically an event fifo will require some amount of temporary
          /// storage to be used during the transfer of the event. This
          /// attribute returns the number of bytes that must be provided to
          /// the produceEvent() method.
          ///
          /// \attention All event fifo interface derived classes \b must
          ///            contain a public static const data member named
          ///            'size_t event_fifo_state_bytes'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'event_fifo_state_bytes' constant.
          ///
          static const size_t getEventFifoStateBytes ();

          ///
          /// \brief Initialize an event fifo using a memory manager and unique key.
          ///
          /// The memory manager may be used by an event fifo implementation to
          /// construct the actual fifo event array in a known location.
          ///
          /// After initialization the event fifo is available for use.
          ///
          /// \param [in] mm  Memory manager to allocate internal fifo objects
          /// \param [in] key Unique key that identifies the fifo
          ///
          template <class T_MemoryManager>
          inline void initialize (T_MemoryManager * mm, char * key);

          ///
          /// \brief Produce an event into the fifo.
          ///
          /// \param [in] state  Event state
          /// \param [in] fn     Event function
          /// \param [in] cookie Event cookie
          ///
          /// \retval true  The event was produced
          /// \retval false The event was not produced
          ///
          template <typename T, unsigned N>
          inline bool produceEvent (T                     (&state)[N],
                                    pami_event_function   fn,
                                    void                * cookie);

          ///
          /// \brief Consume an event from the fifo.
          ///
          /// \param [in] context Context to use when invoking the event function
          ///
          /// \retval true  The event was consumed
          /// \retval false The event was not consumed
          ///
          inline bool consumeEvent (pami_context_t context);
      };

      template <class T_Fifo>
      const size_t Interface<T_Fifo>::getEventFifoStateBytes ()
      {
        return T_Fifo::event_fifo_state_bytes;
      }

      template <class T_Fifo>
      template <class T_MemoryManager>
      void Interface<T_Fifo>::initialize (T_MemoryManager * mm, char * key)
      {
        return static_cast<T_Fifo*>(this)->initialize_impl (mm, key);
      }

      template <class T_Fifo>
      template <typename T, unsigned N>
      bool Interface<T_Fifo>::produceEvent (T                     (&state)[N],
                                            pami_event_function   fn,
                                            void                * cookie)
      {
        return static_cast<T_Fifo*>(this)->produceEvent_impl (state, fn, cookie);
      }

      template <class T_Fifo>
      bool Interface<T_Fifo>::consumeEvent (pami_context_t context)
      {
        return static_cast<T_Fifo*>(this)->consumeEvent_impl (context);
      }
    };
  };
};
#endif // __components_fifo_EventFifoInterface_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
