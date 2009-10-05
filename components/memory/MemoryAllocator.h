/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
#ifndef __xmi_components_memory_memoryallocator_h__
#define __xmi_components_memory_memoryallocator_h__

#include "components/atomic/noop/Noop.h"

//#define USE_MEMALIGN

namespace XMI
{
  /// \todo Update to use a memory manager template parameter.
  template <unsigned T_ObjSize, unsigned T_ObjAlign, class T_Atomic = XMI::Atomic::Noop>
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
#ifdef USE_MEMALIGN
          posix_memalign ((void **)&object, T_ObjAlign, sizeof(memory_object_t) * 10);
#else
          object = (memory_object_t*)malloc(sizeof(memory_object_t)*10);
#endif
          // "return" the newly allocated objects to the pool of free objects.
          for (i=1; i<10; i++) returnObject ((void *) &object[i]);
        }

        unlock ();

        return (void *) object;
      };

      inline void returnObject (void * object)
      {
        lock ();

        memory_object_t * tmp = (memory_object_t *) object;
        tmp->next = _head;
        _head = tmp;

        unlock ();
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

#endif // __xmi_components_memory_memoryallocator_h__
