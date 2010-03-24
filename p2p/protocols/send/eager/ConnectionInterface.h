/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/eager/ConnectionInterface.h
/// \brief Connection class for reliable, deterministic eager protocols
///
#ifndef __p2p_protocols_send_eager_ConnectionInterface_h__
#define __p2p_protocols_send_eager_ConnectionInterface_h__

#include <stdlib.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
  namespace Protocol
  {
    namespace Interface
    {
      template <class T_Connection, class T_Device>
      class Connection
      {
        public:
          inline Connection (T_Device & device) {}
          inline ~Connection () {}

          inline void set (pami_task_t task, size_t contextid, void * value);

          inline void * get (pami_task_t task, size_t contextid);

          inline void clear (pami_task_t task, size_t contextid);
      };

      template <class T_Connection, class T_Device>
      inline void Connection<T_Connection, T_Device>::set (pami_task_t task, size_t contextid, void * value)
      {
        static_cast<T_Connection*>(this)->set_impl(task, contextid, value);
      }

      template <class T_Connection, class T_Device>
      inline void * Connection<T_Connection, T_Device>::get (pami_task_t task, size_t contextid)
      {
        return static_cast<T_Connection*>(this)->get_impl(task, contextid);
      }

      template <class T_Connection, class T_Device>
      inline void Connection<T_Connection, T_Device>::clear (pami_task_t task, size_t contextid)
      {
        static_cast<T_Connection*>(this)->clear_impl(task, contextid);
      }
    };
  };
};
#endif // __p2p_protocols_send_eager_ConnectionInterface_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
