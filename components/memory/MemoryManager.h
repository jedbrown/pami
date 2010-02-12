/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
    class MemoryManager
    {
      public:
        inline MemoryManager ()
        {
          init (NULL, 0);
        };

        inline MemoryManager (void * addr, size_t bytes)
        {
          init (addr, bytes);
        };

        inline void init (void * addr, size_t bytes)
        {
          _base   = (uint8_t *) addr;
          _size   = bytes;
          _offset = 0;
        };

        ///
        /// \brief Allocate an aligned buffer of the memory.
        ///
        /// \param[out] memptr    Pointer to the allocated memory.
        /// \param[in]  alignment Requested buffer alignment - must be a power of 2.
        /// \param[in]  bytes     Number of bytes to allocate.
        ///
        inline xmi_result_t memalign (void ** memptr, size_t alignment, size_t bytes)
        {
          XMI_assert_debug(_base != NULL);
          XMI_assert((alignment & (alignment - 1)) == 0);

          size_t pad = 0;
          if (alignment > 0)
          {
            pad = _offset & (alignment - 1);
            if (pad > 0)
            pad = (alignment - pad);
          }

          if ((_offset + pad + bytes) <= _size)
          {
            _offset += pad;
            *memptr =  (void *) ((size_t)_base + _offset);
            _offset += bytes;
            return XMI_SUCCESS;
          }
          return XMI_ERROR;

        };

        inline size_t available (size_t alignment = 1)
        {
          XMI_assert_debug((alignment & (alignment - 1)) == 0);
          return _size - _offset - alignment;
        };

        inline size_t size ()
        {
          XMI_assert_debug(_base != NULL);
          return _size;
        };

      private:
        void * _base;
        size_t _size;
        size_t _offset;
    };
  };
};

#endif // __xmi_components_memory_memorymanager_h__
