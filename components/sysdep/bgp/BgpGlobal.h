/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __xmi_components_sysdep_bgp_bgpglobal_h__
#define __xmi_components_sysdep_bgp_bgpglobal_h__
///
/// \file components/sysdep/bgp/BgpGlobal.h
/// \brief Blue Gene/P Global Object
///
/// This global object is constructed before main() and is a container class
/// for all other classes that need to be constructed and initialized before
/// the application starts.
///

#define PERS_SIZE 1024


#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "util/common.h"
#include "components/sysdep/bgp/BgpPersonality.h"
#include "components/mapping/bgp/BgpMapCache.h"

namespace XMI
{
  namespace SysDep
  {
    class BgpGlobal
    {
      public:

        inline BgpGlobal () :
          personality (),
          _mapcache (),
          _memptr (NULL),
          _memsize (0)
        {
          allocateMemory ();

          size_t bytes_used = _mapcache.init (personality,
                                              _memptr,
                                              _memsize);
        };



        inline ~BgpGlobal () {};

        inline size_t * getMapCache ()
        {
          return _mapcache.getMapCache();
        };

        inline size_t * getRankCache ()
        {
          return _mapcache.getRankCache();
        };

     private:

        inline void allocateMemory ()
        {
          char   * shmemfile = "/unique-xmi-shmem-file";
          size_t   bytes     = 1024*1024;
          size_t   pagesize  = 4096;

          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

          int fd, rc;
          size_t n = bytes;

          fd = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);
          if ( fd != -1 )
          {
            rc = ftruncate( fd, n );
            if ( rc != -1 )
            {
              void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
              if ( ptr != MAP_FAILED )
              {
                _memptr  = ptr;
                _memsize = n;
                return;
              }
            }
          }

          // There was a failure obtaining the shared memory segment, most
          // likely because the applicatino is running in SMP mode. Allocate
          // memory from the heap instead.
          //
          // TODO - verify the run mode is actually SMP.
          posix_memalign ((void **)&_memptr, 16, bytes);
          memset (_memptr, 0, bytes);
          _memsize = bytes;

          return;
        };


      public:

        BgpPersonality       personality;

      private:

        Mapping::BgpMapCache _mapcache;
        void            * _memptr;
        size_t            _memsize;
    }; // XMI::SysDep::BgpGlobal
  };   // XMI::SysDep
};     // XMI

extern XMI::SysDep::BgpGlobal __global;

#endif // __xmi_components_sysdep_bgp_bgpglobal_h__
