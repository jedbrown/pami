/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/memory/shmem/SharedMemoryManager.h
/// \brief Shared memory manager class
///
/// The class defined in this file uses C++ templates. C++ templates
/// require all source code to be #include'd from a header file.
///
#ifndef __components_memory_shmem_SharedMemoryManager_h__
#define __components_memory_shmem_SharedMemoryManager_h__

#include "components/memory/MemoryManager.h"

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace XMI
{
  namespace Memory
  {
    //template <unsigned T_PageSize>
    class SharedMemoryManager : public Interface::MemoryManager<SharedMemoryManager>
    {
      public:
        inline SharedMemoryManager () :
          Interface::MemoryManager<SharedMemoryManager> (),
          _location (NULL),
          _size (0),
          _offset (0)
        {
          const char   * shmemfile = "/unique-xmi-shmem-file";
          size_t   bytes     = 1024*1024;
          size_t   pagesize  = 4096;

          xmi_result_t result = XMI_ERROR;

          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);
          //size_t size = (bytes + T_PageSize - 1) & ~(T_PageSize - 1);
//          size_t size = (bytes + 4096 - 1) & ~(4096 - 1);

          int fd, rc;
          size_t n = size;
          void * ptr = NULL;

          TRACE_ERR((stderr, "SharedMemoryManager() .. size = %zd\n", size));
          fd = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);
          TRACE_ERR((stderr, "SharedMemoryManager() .. after shm_open, fd = %d\n", fd));
          if ( fd != -1 )
          {
            rc = ftruncate( fd, n );
            TRACE_ERR((stderr, "SharedMemoryManager() .. after ftruncate(%d,%zd), rc = %d\n", fd,n,rc));
            if ( rc != -1 )
            {
              ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
              TRACE_ERR((stderr, "SharedMemoryManager() .. after mmap, ptr = %p, MAP_FAILED = %p\n", ptr, MAP_FAILED));
              if ( ptr != MAP_FAILED )
              {
                TRACE_ERR((stderr, "SharedMemoryManager:shmem file <%s> %zd bytes mapped at %p\n", shmemfile, n, ptr));
                _location = ptr;
                _size     = n;
                TRACE_ERR((stderr, "SharedMemoryManager() .. _location = %p, _size = %zd\n", _location, _size));
                result = XMI_SUCCESS;
                return;
              }
            }
          }
#ifdef ENABLE_MAMBO_WORKAROUNDS
          fprintf(stderr,"%s:%d Failed to create shared memory (rc=%d, ptr=%p, n=%zd) errno %d %s\n",__FILE__,__LINE__, rc, ptr, n, errno, strerror(errno));
#endif
          TRACE_ERR((stderr, "SharedMemoryManager() .. FAILED, fake shmem on the heap\n"));

#ifdef USE_MEMALIGN
          posix_memalign ((void **)&_location, 16, n);
#else
          _location = (void*)malloc(n);
#endif
          _size = n;

          return;
        }

        inline xmi_result_t memalign_impl (void   ** memptr,
                                           size_t    alignment,
                                           size_t    bytes)
        {
          // Currently, shmem areas are not "free'd".
          assert((alignment & (alignment - 1)) == 0);

          void * addr = NULL;
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
            *memptr =  (void *) ((size_t)_location + _offset);
            _offset += bytes;
            return XMI_SUCCESS;
          }
          return XMI_ERROR;

        }

        protected:

          void * _location;
          size_t _size;
          size_t _offset;
    };
  };
};
#undef TRACE_ERR
#endif // __xmi_components_memory_shmem_sharedmemorymanager_h__
