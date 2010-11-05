/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/connection/ConnectionArray.h
 * \brief ???
 */

#ifndef __components_connection_ConnectionArray_h__
#define __components_connection_ConnectionArray_h__

#include "components/connection/ConnectionInterface.h"

#include "util/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Connection
  {
    template <class T_Device, unsigned T_MaxContexts = 64>
    class Array : public Interface<Connection::Array<T_Device, T_MaxContexts>, T_Device>
    {
      protected:

        inline void ** getArray () { return _array; };

        inline size_t endpoint2index (pami_endpoint_t endpoint)
        {
          TRACE_FN_ENTER();
          size_t task, offset;
          PAMI_ENDPOINT_INFO(endpoint, task, offset);

          PAMI_assert_debug(offset < T_MaxContexts);

          size_t peer = _device.task2peer (task);
          size_t index = peer + offset * _npeers;
          TRACE_FORMAT("endpoint = 0x%08x, task = %zu, offset = %zu, peer = %zu, index = %zu", (unsigned) endpoint, task, offset, peer, index)
          TRACE_FN_EXIT();
          return index;
        };

      public:

        inline Array (T_Device & device) :
            Interface<Connection::Array<T_Device>, T_Device> (device),
            _device (device),
            _array (NULL),
            _npeers (device.peers())
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("this = %p, &_manager = %p", this, & _manager);

          _manager.lock();

          Array * value = NULL;

          if (_manager.get (&device, value) == true)
            {
              _array = value->getArray();
              TRACE_FORMAT("_array = %p", _array);
            }
          else
            {
              size_t bytes = sizeof(void *) * device.peers() * device.getContextCount();
              _array = (void **) malloc (bytes);
              memset((void *)_array, 0, bytes);
              TRACE_FORMAT("_array = %p, bytes = %zu, device.peers() = %zu, device.getContextCount() = %zu", _array, bytes, device.peers(), device.getContextCount());
              _manager.set (&device, this);
            }

          _manager.unlock();

          TRACE_FN_EXIT();
        };

        inline void set_impl (pami_endpoint_t key, void * value)
        {
          TRACE_FN_ENTER();

          size_t index = endpoint2index (key);

          TRACE_FORMAT("this = %p, _array = %p, _array[%zu] = %p", this, (void *) _array, index, _array[index]);

          PAMI_assert_debug(_array[index] == NULL);
          _array[index] = value;

          TRACE_FN_EXIT();
        };

        inline void * get_impl (pami_endpoint_t key)
        {
          TRACE_FN_ENTER();

          size_t index = endpoint2index (key);

          TRACE_FORMAT("this = %p, _array = %p, _array[%zu] = %p", this, (void *) _array, index, _array[index]);

          PAMI_assert_debug(_array[index] != NULL);

          TRACE_FN_EXIT();
          return _array[index];
        };

        inline void clear_impl (pami_endpoint_t key)
        {
          TRACE_FN_ENTER();

          size_t index = endpoint2index (key);

          TRACE_FORMAT("this = %p, _array = %p, _array[%zu] = %p -> NULL", this, (void *) _array, index, _array[index]);

          _array[index] = NULL;

          TRACE_FN_EXIT();
        };

      private:

        T_Device  & _device;
        void     ** _array;
        size_t      _npeers;

        static typename PAMI::Connection::Interface<PAMI::Connection::Array<T_Device,T_MaxContexts>, T_Device>::Manager _manager;
    };


    template <class T_Device, unsigned T_MaxContexts>
    typename PAMI::Connection::Interface<PAMI::Connection::Array<T_Device,T_MaxContexts>, T_Device>::Manager
    PAMI::Connection::Array<T_Device,T_MaxContexts>::_manager;
  };
};

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_connection_ConnectionArray_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
