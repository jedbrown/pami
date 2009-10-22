/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/memory/MemoryManager.h
/// \brief Common templatized memory manager class
///
/// The class defined in this file uses C++ templates. C++ templates
/// require all source code to be #include'd from a header file.
///
#ifndef __components_memory_MemoryManager_h__
#define __components_memory_MemoryManager_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace Memory
  {
    namespace Interface
    {
      template <class T>
      class MemoryManager
      {
        public:
          inline MemoryManager () {};

          ///
          /// \brief Allocate an aligned buffer of the memory.
          ///
          /// \param[out] memptr    Pointer to the allocated memory.
          /// \param[in]  alignment Requested buffer alignment - must be a power of 2.
          /// \param[in]  bytes     Number of bytes to allocate.
          ///
          inline xmi_result_t memalign(void ** memptr, size_t alignment, size_t bytes);
      };

      template <class T>
      inline xmi_result_t MemoryManager<T>::memalign(void ** memptr, size_t alignment, size_t bytes)
      {
        return static_cast<T*>(this)->memalign_impl (memptr, alignment, bytes);
      }
    };
  };
};

#endif // __xmi_components_memory_memorymanager_h__
