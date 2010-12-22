/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/memory/MemoryAllocator.h
/// \brief Common templatized memory allocator class
///
/// The class defined in this file uses C++ templates. C++ templates
/// require all source code to be #include'd from a header file.
///
#ifndef __components_memory_MemoryAllocator_h__
#define __components_memory_MemoryAllocator_h__

#include "components/memory/MemoryManager.h"
#include "components/atomic/noop/Noop.h"

#include "util/trace.h"

#undef  DO_TRACE_ENTEREXIT
#define DO_TRACE_ENTEREXIT 0
#undef  DO_TRACE_DEBUG    
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  /// \todo Update to use a memory manager template parameter.
  template <unsigned T_ObjSize, unsigned T_ObjAlign, unsigned T_PREALLOC=8, class T_Atomic = PAMI::Atomic::Noop>
  class MemoryAllocator
  {
    protected:

      typedef struct memory_object
      {
        uint8_t             object[T_ObjSize];
        union
        {
          struct memory_object * next;
          uint8_t                pad[T_ObjAlign];
        };
      } memory_object_t;

    public:

      inline MemoryAllocator () :
        _mutex (),
        _head (NULL)
      {}

      inline void * allocateObject ()
      {
        TRACE_FN_ENTER();
        lock ();

        memory_object_t * object = _head;
        if (object != NULL)
        {
          _head  = _head->next;
        }
        else
        {
          // Allocate and construct a new set of objects
          unsigned i;
          pami_result_t rc;
	  rc = PAMI::Memory::MemoryManager::heap_mm->memalign(
			(void **)&object, T_ObjAlign, sizeof(memory_object_t) * T_PREALLOC);
          PAMI_assertf(rc==PAMI_SUCCESS, "alloc failed for context\n");
          // "return" the newly allocated objects to the pool of free objects.
          for (i=1; i<T_PREALLOC; i++) returnObject ((void *) &object[i]);
        }

        unlock ();

        TRACE_FORMAT("<%p>", object);
        TRACE_FN_EXIT();
        return (void *) object;
      };

      inline void returnObject (void * object)
      {
        TRACE_FN_ENTER();
        lock ();

        TRACE_FORMAT("<%p>", object);
        memory_object_t * tmp = (memory_object_t *) object;
        tmp->next = _head;
        _head = tmp;

        unlock ();
        TRACE_FN_EXIT();
      };

      static const size_t objsize = T_ObjSize;

    protected:

      inline void lock ()
      {
        while (!_mutex.compare_and_swap (0, 1));
      }

      inline void unlock ()
      {
        _mutex.fetch_and_clear ();
      }

      T_Atomic          _mutex;

      memory_object_t * _head;
  };
};
#undef  DO_TRACE_ENTEREXIT
#define DO_TRACE_ENTEREXIT 0
#undef  DO_TRACE_DEBUG    
#define DO_TRACE_DEBUG     0

#endif // __pami_components_memory_memoryallocator_h__
