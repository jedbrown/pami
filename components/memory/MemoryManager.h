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
/// \brief Base memory manager class
///
#ifndef __components_memory_MemoryManager_h__
#define __components_memory_MemoryManager_h__

#include "sys/xmi.h"
#include <sys/mman.h>
#include <errno.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace XMI
{
  namespace Memory
  {
    class MemoryManager
    {
      public:
        ///
        /// \brief Empty base memory manager constructor
        ///
        inline MemoryManager ()
        {
          TRACE_ERR((stderr, "%s\n", __PRETTY_FUNCTION__));
          init (NULL, 0);
        };

        ///
        /// \brief Base memory manager constructor with initial memory buffer
        ///
        /// \param[in] addr  Address of the memory to be managed
        /// \param[in] bytes Number of bytes of memory to manage
        ///
        inline MemoryManager (void * addr, size_t bytes)
        {
          TRACE_ERR((stderr, "%s(%p, %zd)\n", __PRETTY_FUNCTION__,addr,bytes));
          init (addr, bytes);
        };

        ///
        /// \brief Intialize a memory manager with a memory buffer
        ///
        /// \param[in] addr  Address of the memory to be managed
        /// \param[in] bytes Number of bytes of memory to manage
        ///
        inline void init (void * addr, size_t bytes)
        {
          TRACE_ERR((stderr, "%s(%p, %zd)\n", __PRETTY_FUNCTION__,addr,bytes));
          _base   = (uint8_t *) addr;
          _size   = bytes;
          _offset = 0;
        };

        ///
        /// \brief Memory syncronization
        ///
        /// \todo Remove? Why is this needed? The \c msync macros defined in
        ///       Arch.h should be sufficient.
        ///
        void sync()
        {
          static bool perr = false;
          int rc = msync((void*)_base, _size, MS_SYNC);
          if(!perr && rc) {
            perr=true;
            fprintf(stderr,  "MemoryManager::msync failed with %d, errno %d: %s\n", rc, errno, strerror(errno));
          }
        }

        ///
        /// \brief Allocate an aligned buffer of the memory.
        ///
        /// \param[out] memptr    Pointer to the allocated memory.
        /// \param[in]  alignment Requested buffer alignment - must be a power of 2.
        /// \param[in]  bytes     Number of bytes to allocate.
        ///
        inline xmi_result_t memalign (void ** memptr, size_t alignment, size_t bytes)
        {
          TRACE_ERR((stderr, "%s(%p, %zd, %zd)\n", __PRETTY_FUNCTION__,memptr,alignment,bytes));
          XMI_assert_debug(_base != NULL);
          XMI_assert((alignment & (alignment - 1)) == 0);

          size_t pad = 0;
          if (alignment > 0)
          {
            pad = ((size_t)_base + _offset) & (alignment - 1);
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
          TRACE_ERR((stderr, "%s XMI_ERROR !((%zd + %zd + %zd) <= %zd)\n",__PRETTY_FUNCTION__,_offset,pad,bytes,_size));
          return XMI_ERROR;
        };

        ///
        /// \brief Return the current maximum number of bytes that may be allocated
        ///
        /// \param[in] alignment Optional alignment parameter
        ///
        /// \return    Number of bytes available
        ///
        inline size_t available (size_t alignment = 1)
        {
          TRACE_ERR((stderr, "%s(%zd) _size %zd, _offset %zd\n", __PRETTY_FUNCTION__,alignment, _size, _offset));
          XMI_assert_debug((alignment & (alignment - 1)) == 0);

          size_t pad = 0;
          if (alignment > 0)
          {
            pad = ((size_t)_base + _offset) & (alignment - 1);
            if (pad > 0)
              pad = (alignment - pad);
          }

          return _size - _offset - pad;
        };

        ///
        /// \brief Return the size of the managed memory buffer
        ///
        /// \return    Number of bytes in the memory buffer
        ///
        inline size_t size ()
        {
          TRACE_ERR((stderr, "%s %zd\n", __PRETTY_FUNCTION__,_size));
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

#endif // __components_memory_MemoryManager_h__
