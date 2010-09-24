/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/connection/ConnectionInterface.h
/// \brief Connection interface class for reliable, deterministic eager protocols
///
#ifndef __components_connection_ConnectionInterface_h__
#define __components_connection_ConnectionInterface_h__

#include <stdlib.h>

#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"


#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
  namespace Connection
  {
    template <class T_Connection, class T_Device>
    class Interface
    {
      protected:

        class Manager
        {
          public:

            class Element : public Queue::Element
            {
              public:

                inline Element (T_Device     * key,
                                T_Connection * value) :
                Queue::Element (),
                _key (key),
                _value (value)
                {};

                inline ~Element () {};

                inline T_Device * getKey ()
                {
                  return _key;
                };

                inline T_Connection * getValue ()
                {
                  return _value;
                };

              private:

                T_Device     * _key;
                T_Connection * _value;
            };

            inline Manager ()
            {
              _lock.release ();
            };

            inline void lock ()
            {
              _lock.acquire ();
            };

            inline void unlock ()
            {
              _lock.release ();
            };

            inline void set (T_Device     * key,
                             T_Connection * value)
            {
              Element * element;
	      pami_result_t prc = __global.heap_mm->memalign((void **)&element, 0, sizeof(*element));
	      PAMI_assertf(prc == PAMI_SUCCESS, "alloc of Element failed");
              new (element) Element (key, value);

              _queue.push ((Queue::Element *) element);
            };


            inline bool get (T_Device      * key,
                             T_Connection *& value)
            {
              // Search for the connection for the device instance
              Element * element =
                (Element *) _queue.peek();

              while (element != NULL)
                {
                  if (element->getKey() == key)
                    {
                      value = element->getValue();
                      return true;
                    }

                  element = (Element *) _queue.next ((Queue::Element *) element);
                }

              // Did not find an existing connection.
              return false;
            };

          private:

            Queue _queue;
            Mutex::CounterMutex<Atomic::GccBuiltin> _lock;
        };

        inline Interface (T_Device & device) {};

        inline ~Interface () {};

      public:

        inline void set (pami_endpoint_t key, void * value);

        inline void * get (pami_endpoint_t key);

        inline void clear (pami_endpoint_t key);

    };  // class    PAMI::Connection::Interface

    template <class T_Connection, class T_Device>
    inline void Interface<T_Connection, T_Device>::set (pami_endpoint_t key, void * value)
    {
      static_cast<T_Connection*>(this)->set_impl(key, value);
    }

    template <class T_Connection, class T_Device>
    inline void * Interface<T_Connection, T_Device>::get (pami_endpoint_t key)
    {
      return static_cast<T_Connection*>(this)->get_impl(key);
    }

    template <class T_Connection, class T_Device>
    inline void Interface<T_Connection, T_Device>::clear (pami_endpoint_t key)
    {
      static_cast<T_Connection*>(this)->clear_impl(key);
    }
  };   // namespace PAMI::Connection
};     // namespace PAMI
#endif // __components_connection_ConnectionInterface_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
