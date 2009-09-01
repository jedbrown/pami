/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
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
#ifndef __xmi_components_memory_shmem_sharedmemorymanager_h__
#define __xmi_components_memory_shmem_sharedmemorymanager_h__

#include "components/memory/MemoryManager.h"

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

//#include <spi/bgp_SPI.h>
//#include <cnk/bgp_SysCall_Extensions.h>

namespace XMI
{
  namespace Memory
  {
    //template <unsigned T_PageSize = 4096>
    class SharedMemoryManager : public Interface::MemoryManager<SharedMemoryManager>
    {
      public:
        inline SharedMemoryManager () :
          Interface::MemoryManager<SharedMemoryManager> (),
          _location (NULL),
          _size (0),
          _offset (0)
        {
        char   * shmemfile = "/unique-xmi-shmem-file";
                                    size_t   bytes     = 1024*1024;
          xmi_result_t result = XMI_ERROR;

          // Round up to the page size
//          size_t size = (bytes + T_PageSize - 1) & ~(T_PageSize - 1);
          size_t size = (bytes + 4096 - 1) & ~(4096 - 1);
          
          int fd, rc;
          size_t n;

          fd = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);
          if ( fd != -1 )
          {
            rc = ftruncate( fd, n );
            if ( rc != -1 )
            {
              void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
              if ( ptr != MAP_FAILED )
              {
                _location = ptr;
                _size     = n;
                result = XMI_SUCCESS;
                return;
              }
            }
          }
          
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

#endif // __xmi_components_memory_shmem_sharedmemorymanager_h__


