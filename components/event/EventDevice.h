/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/event/EventDevice.h
 * \brief ???
 */

#ifndef __components_event_EventDevice_h__
#define __components_event_EventDevice_h__

#include <pami.h>

namespace PAMI
{
  namespace Event
  {

    template < class T_EventFifo >
    class Device
    {
      public:

        template <class T_MemoryManager>
        inline void initialize (pami_context_t    context,
                                T_MemoryManager * mm,
                                char            * key)
        {
          _fifo.initialize(mm, key);
          _context = context;
        };

        inline void post (pami_work_t         * work,
                          pami_event_function   fn,
                          void                * cookie)
        {
          _fifo.produceEvent(*work, fn, cookie);
        };

        inline void poll ()
        {
          while (_fifo.consumeEvent(_context));
        }

      protected:

        T_EventFifo    _fifo;
        pami_context_t _context;

    };
  };
};
#endif // __components_event_EventDevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//





