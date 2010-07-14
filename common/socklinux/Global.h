/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_socklinux_Global_h__
#define __common_socklinux_Global_h__
///
/// \file common/socklinux/Global.h
/// \brief Sockets on Linux Global Object
///
/// This global object is constructed before main() and is a container class
/// for all other classes that need to be constructed and initialized before
/// the application starts.
///

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "Platform.h"
#include "util/common.h"
#include "common/GlobalInterface.h"
#include "Mapping.h"
#include "Topology.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  // fprintf x
#endif

namespace PAMI
{
    class Global : public Interface::Global<PAMI::Global>
    {
      public:

        inline Global () :
          _memptr (NULL),
          _memsize (0)
        {
          TRACE_ERR((stderr, ">> Global::Global()\n"));

          Interface::Global<PAMI::Global>::time.init(0);
          pami_coord_t ll, ur;
          size_t min, max, num;
          size_t *ranks;
          size_t   bytes     = 1024*1024;
          size_t   pagesize  = 4096;

          snprintf (_shmemfile, 1023, "/unique-pami-global-shmem-file");

          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

          int fd, rc;
          size_t n = size;

          // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
          TRACE_ERR((stderr, "Global() .. size = %zu\n", size));
          rc = shm_open (_shmemfile, O_CREAT | O_RDWR, 0600);
          TRACE_ERR((stderr, "Global() .. after shm_open, fd = %d\n", fd));
          if ( rc != -1 )
          {
            fd = rc;
            rc = ftruncate( fd, n );
            TRACE_ERR((stderr, "Global() .. after ftruncate(%d,%zu), rc = %d\n", fd,n,rc));
            if ( rc != -1 )
            {
              void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
              TRACE_ERR((stderr, "Global() .. after mmap, ptr = %p, MAP_FAILED = %p\n", ptr, MAP_FAILED));
              if ( ptr != MAP_FAILED )
              {
                _memptr  = ptr;
                _memsize = n;

                TRACE_ERR((stderr, "Global() .. _memptr = %p, _memsize = %zu\n", _memptr, _memsize));

                // Round up to the page size
                size = (bytes + pagesize - 1) & ~(pagesize - 1);

                // Truncate to this size.
                rc = ftruncate( fd, size );
                TRACE_ERR((stderr, "Global() .. after second ftruncate(%d,%zu), rc = %d\n", fd,n,rc));
              } else { rc = -1; }
            }
          }

          if (rc == -1) {
                  // There was a failure obtaining the shared memory segment, most
                  // likely because the application is running in SMP mode. Allocate
                  // memory from the heap instead.
                  //
                  // TODO - verify the run mode is actually SMP.
                  rc = posix_memalign ((void **)&_memptr, 16, bytes);
                  memset (_memptr, 0, bytes);
                  _memsize = bytes;
                  TRACE_ERR((stderr, "Global() .. FAILED, fake shmem on the heap, _memptr = %p, _memsize = %zu\n", _memptr, _memsize));
          }

          mapping.init(min, max, num, &ranks);
          PAMI::Topology::static_init(&mapping);
          new (&topology_global) PAMI::Topology(min, max);
          topology_global.subTopologyLocalToMe(&topology_local);
          TRACE_ERR((stderr, "<< Global::Global()\n"));

          return;
        };



        inline ~Global ()
        {
          shm_unlink (_shmemfile);
        };

        inline size_t size ()
        {
          return mapping.size();
        }

     private:

       public:

        PAMI::Mapping         mapping;

      private:
        char _shmemfile[1024];

        void           * _memptr;
        size_t           _memsize;
        size_t           _size;
    }; // PAMI::Global
};     // PAMI

extern PAMI::Global __global;
#undef TRACE_ERR
#endif // __common_socklinux_Global_h__
