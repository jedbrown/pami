/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file p2p/protocols/send/eager/ConnectionArray.h
 * \brief ???
 */

#ifndef __p2p_protocols_send_eager_ConnectionArray_h__
#define __p2p_protocols_send_eager_ConnectionArray_h__

#include "p2p/protocols/send/eager/ConnectionInterface.h"

namespace XMI
{
  namespace Protocol
  {
    template <class T_Device, unsigned T_Size = 128>
    class ConnectionArray : public Interface::Connection<ConnectionArray<T_Device,T_Size>,T_Device>
    {
      private:

        struct connection_info_t
        {
          T_Device  * device_va;
          void     ** array;
        };

        static const size_t _maximum_context_count = 4;

      public:

        inline ConnectionArray (T_Device & device) :
          Interface::Connection<ConnectionArray<T_Device,T_Size>,T_Device> (device),
          _connection (NULL),
          _peers (device.peers())
        {
          // Search for an existing connection array for this device object
          unsigned i;
          for (i=0; i<T_Size; i++)
          {
            // The virtual address of the device object will uniquely identify
            // the device across all clients and contexts in the process.
            if (_c[i].device_va == &device)
            {
              _connection = _c[i].array;
              return;
            }
            else if (_c[i].device_va == (T_Device *) NULL)
            {
              // Allocate a new connection array for this device object
              _c[i].array = (void **) malloc (sizeof(void *) * _maximum_context_count * _peers);
              _c[i].device_va = &device;
              _connection = _c[i].array;

              unsigned j;
              for (j=0; j<(_maximum_context_count * _peers); j++)
                _connection[j] = NULL;

              return;
            }
          }

          // All connection arrays are allocated? abort.
          XMI_abort();
          return;
        };

        inline void set_impl (xmi_task_t task, size_t contextid, void * value)
        {
          XMI_assert_debug(task < _peers);
          XMI_assert_debug(contextid < _maximum_context_count);
          size_t index = (task << 2) | contextid;
          XMI_assert_debug(_connection[index] == NULL);
          _connection[index] = value;
        };

        inline void * get_impl (xmi_task_t task, size_t contextid)
        {
          XMI_assert_debug(task < _peers);
          XMI_assert_debug(contextid < _maximum_context_count);
          size_t index = (task << 2) | contextid;
          XMI_assert_debug(_connection[index] != NULL);
          return _connection[index];
        };

        inline void clear_impl (xmi_task_t task, size_t contextid)
        {
          XMI_assert_debug(task < _peers);
          XMI_assert_debug(contextid < _maximum_context_count);
          size_t index = (task << 2) | contextid;
          _connection[index] = NULL;
        };

      private:

        void ** _connection;
        size_t  _peers;

        static struct connection_info_t _c[T_Size];
    };

    template <class T_Device, unsigned T_Size>
    struct ConnectionArray<T_Device,T_Size>::connection_info_t ConnectionArray<T_Device,T_Size>::_c[T_Size];
  };
};



#endif // __p2p_protocols_send_eager_ConnectionArray_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

