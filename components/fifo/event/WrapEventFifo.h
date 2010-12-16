/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/fifo/event/WrapEventFifo.h
 * \brief ???
 */

#ifndef __components_fifo_event_WrapEventFifo_h__
#define __components_fifo_event_WrapEventFifo_h__

#include <pami.h>

#include "components/fifo/EventFifoInterface.h"

#include "Arch.h"

namespace PAMI
{
  namespace Fifo
  {
    namespace Event
    {

      template < class T_Atomic, unsigned T_Size>
      class Wrap : public Interface < Linear<T_Atomic, T_Size> >
      {
        public:

          friend class Interface < Wrap<T_Atomic, T_Size> >;

          static const size_t event_fifo_state_bytes = 32; // sizeof(???);

          inline Wrap () :
              Interface < Wrap<T_Atomic, T_Size> > ()
          {
          };

          inline ~Wrap () {};

        protected:

          // ---------------------------------------------------------------------
          // PAMI::Fifo::Event::Interface implementation
          // ---------------------------------------------------------------------

          template <class T_MemoryManager>
          inline void initialize_impl (T_MemoryManager * mm, char * key)
          {
          };

          template <typename T, unsigned N>
          inline bool produceEvent_impl (T                     (&state)[N],
                                         pami_event_function   fn,
                                         void                * cookie)
          {
            //COMPILE_TIME_ASSERT(sizeof(???)<=(sizeof(T)*N));
            return true;
          };

          inline bool consumeEvent_impl (pami_context_t context)
          {
            return true;
          };
      };
    };
  };
};
#endif // __components_fifo_event_WrapEventFifo_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
