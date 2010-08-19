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
#include <semaphore.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace PAMI
{
  namespace Memory
  {
    class SharedMemoryManager : public MemoryManager
    {
      public:

        inline SharedMemoryManager (const char * name, size_t bytes) :
          MemoryManager ()
        {
          char * jobstr = getenv ("PAMI_JOB_ID");
          if (jobstr)
            snprintf (_shmemfile, 1023, "/pami-client-%s-%s", jobstr, name);
          else
            snprintf (_shmemfile, 1023, "/pami-client-%s", name);

          // Round up to the page size
          //size_t pagesize  = 4096;
          //size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

          TRACE_ERR((stderr, "SharedMemoryManager() .. bytes = %zu\n", bytes));
          int fd = shm_open (_shmemfile, O_CREAT | O_RDWR, 0600);
          TRACE_ERR((stderr, "SharedMemoryManager() .. after shm_open, fd = %d\n", fd));
          if ( fd != -1 )
          {
            int rc = ftruncate (fd, bytes);
            TRACE_ERR((stderr, "SharedMemoryManager() .. after ftruncate(%d,%zu), rc = %d\n", fd,n,rc));
            if (rc != -1)
            {
              void * ptr = mmap (NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
              TRACE_ERR((stderr, "SharedMemoryManager() .. after mmap, ptr = %p, MAP_FAILED = %p\n", ptr, MAP_FAILED));
              if ( ptr != MAP_FAILED )
              {
                TRACE_ERR((stderr, "SharedMemoryManager() .. \"%s\" %zu bytes mapped at %p\n", shmemfile, bytes, ptr));
                _base = ptr;
                _size = bytes;
                TRACE_ERR((stderr, "SharedMemoryManager() .. _location = %p, _size = %zu\n", _location, _size));

                // Use posix semaphores as a platform-neutral way of
                // initializing the shared memory area.  The first process in
                // will clear the memory so that all processes start using
                // shared memory from a known state.
                //
                // This allows items like shared variables or atomic counters to
                // be placed in the shared memory area without the need to
                // separately initialize the variable or counter - the initial
                // value will be '0'.
                char semfile[2][sizeof(_shmemfile)+12];
                snprintf (semfile[0], sizeof(_shmemfile)+12, "%s-semaphore0", _shmemfile);
                snprintf (semfile[1], sizeof(_shmemfile)+12, "%s-semaphore1", _shmemfile);
                size_t p, np;
                __global.mapping.nodePeers (np);
fprintf (stderr, "[%zu] SharedMemoryManager() .. np = %zu\n", __global.mapping.task(), np);
                sem_t * semaphore0 = sem_open (semfile[0], O_CREAT, 0644, 1);
                if (semaphore0 != SEM_FAILED)
                {
                  sem_t * semaphore1 = sem_open (semfile[1], O_CREAT, 0644, 0);
                  if (semaphore1 != SEM_FAILED)
                  {
                    while ((sem_wait(semaphore0) == -1) && errno == EINTR);

                    // test the value of semaphore1, if
                    int value;
                    sem_getvalue (semaphore1, &value);
                    if (value == 0)
                    {
                      // First process in gets to do the initialization
fprintf (stderr, "[%zu] SharedMemoryManager() .. memset\n", __global.mapping.task());
                      memset (_base, 0, bytes);
                    }

                    // increment and close semaphore1
                    sem_post (semaphore1);
                    sem_close (semaphore1);

                    // increment and close semaphore0
                    sem_post (semaphore0);
                    sem_close (semaphore0);

                    if (value == (np-1))
                    {
                      // Last process in gets to tear down the semaphores
                      rc = sem_unlink (semfile[1]);
fprintf (stderr, "[%zu] SharedMemoryManager() .. unlink 1 = %d\n", __global.mapping.task(), rc);
                      rc = sem_unlink (semfile[0]);
fprintf (stderr, "[%zu] SharedMemoryManager() .. unlink 0 = %d\n", __global.mapping.task(), rc);
                    }
                  }
                }





/*
                  // Allow other processes to proceed now that the shared memory
                  // area is initialized to a known state, increment the counter
                  // by the number of waiting processes.
                  //size_t p, np;
                  //__global.mapping.nodePeers (np);
                  //for (p=0; p<np; p++)
                    //sem_post (semaphore);

while ((sem_wait(semaphore) == -1) && errno == EINTR);
                  // Destroy the semaphore after all processes are done.
                  sem_close (semaphore);
                  sem_unlink (semfile);
fprintf (stderr, "[%zu] SharedMemoryManager() .. done\n", __global.mapping.task());
                }
                else
                {
fprintf (stderr, "[%zu] SharedMemoryManager() .. waiting\n", __global.mapping.task());
                  // Another process has already created the semaphore and is
                  // initializing the shared memory area. Open the semaphore and
                  // wait until the shared memory initialization is complete.
                  // This will decrement the counter once and unblock when the
                  // value is zero.
                  semaphore = sem_open (semfile, 0);
                  while ((sem_wait(semaphore) == -1) && errno == EINTR);
                  sem_close (semaphore);
fprintf (stderr, "[%zu] SharedMemoryManager() .. done\n", __global.mapping.task());
                }
*/
                return;
              }
            }
          }
          TRACE_ERR((stderr,"%s:%d Failed to create shared memory (rc=%d, ptr=%p, n=%zu) errno %d %s\n",__FILE__,__LINE__, rc, ptr, n, errno, strerror(errno)));
          TRACE_ERR((stderr, "SharedMemoryManager() .. FAILED, fake shmem on the heap\n"));

#ifdef USE_MEMALIGN
          posix_memalign ((void **)&_base, 16, bytes);
#else
          _base = (void*)malloc(bytes);
#endif
          _size = bytes;

          return;
        }

        inline ~SharedMemoryManager ()
        {
          //if (_shmemfile[0] != NULL)
          {
            shm_unlink (_shmemfile);
            //_shmemfile[0] = NULL;
          }
        };

        inline void init (void * addr, size_t bytes)
        {
          PAMI_abort();
        };

        protected:

          char _shmemfile[1024];
    };
  };
};
#undef TRACE_ERR
#endif // __pami_components_memory_shmem_sharedmemorymanager_h__
