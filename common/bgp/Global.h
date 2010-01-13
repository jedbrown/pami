/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_bgp_Global_h__
#define __common_bgp_Global_h__
///
/// \file common/bgp/Global.h
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

#include "Platform.h"
#include "util/common.h"
#include "common/GlobalInterface.h"
#include "common/bgp/BgpPersonality.h"
#include "common/bgp/BgpMapCache.h"
#include "Mapping.h"
#include "Topology.h"
#include "Wtime.h"
#include "common/bgp/LockBoxFactory.h"

namespace XMI
{
    class Global : public Interface::Global<XMI::Global>
    {
      public:

        inline Global () :
	  Interface::Global<XMI::Global>(),
          personality (),
	  mapping(personality),
          _mapcache (),
          _memptr (NULL),
          _memsize (0),
	  lockboxFactory()
        {
	  Interface::Global<XMI::Global>::time.init(personality.clockMHz());
          //allocateMemory ();

          char   * shmemfile = "/unique-xmi-global-shmem-file";
          size_t   bytes     = 1024*1024;
          size_t   pagesize  = 4096;

          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

          int fd, rc;
          size_t n = bytes;

	  // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
          rc = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);
          XMI_assertf(rc != -1, "shm_open(\"%s\", O_CREAT | O_RDWR, 0600) failed, errno=%d\n", shmemfile, errno)
	  fd = rc;
          rc = ftruncate( fd, n );
          XMI_assertf(rc != -1, "ftruncate(%d, %zd) failed, errno=%d\n", fd, n, errno);
          void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
          if (ptr == MAP_FAILED) {
		XMI_assertf(errno == ENOMEM, "mmap(NULL, %zd, PROT_READ | PROT_WRITE, MAP_SHARED, %d, 0) failed, errno = %d\n", n, fd, errno);
		// assert(mode == SMP)
		close(fd);
		shm_unlink(shmemfile);
		fd = -1;
		// just get some memory... remember, this is a ctor and is called pre-main
		ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		XMI_assertf(ptr != MAP_FAILED, "mmap(NULL, %zd, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) failed, errno = %d\n", n, errno);
	  }

          size_t bytes_used = _mapcache.init (personality, ptr, n);
          // Round up to the page size
          size = (bytes_used + pagesize - 1) & ~(pagesize - 1);
	  if (fd != -1) {
                // Truncate to this size.
                rc = ftruncate( fd, size );
          	XMI_assertf(rc != -1, "ftruncate(%d, %zd) failed, errno=%d\n", fd, size, errno);
	  } else {
		void *v = ptr;
		ptr = mremap(v, n, size, 0);
		XMI_assertf(ptr != MAP_FAILED, "mremap(%p, %zd, %zd, 0) failed, errno = %d\n", v, n, size, errno);
	  }
          _memptr  = ptr;
          _memsize = bytes_used;
	  mapping.init(_mapcache, personality);
	  lockboxFactory.init(&mapping);
	  xmi_coord_t ll, ur;
	  size_t min, max;

	  XMI::Topology::static_init(&mapping);
	  _mapcache.getMappingInit(ll, ur, min, max);
	  size_t rectsize = 1;
	  for (unsigned d = 0; d < mapping.globalDims(); ++d) {
		rectsize *= (ur.u.n_torus.coords[d] - ll.u.n_torus.coords[d] + 1);
	  }
	  if (mapping.size() == rectsize) {
		new (&topology_global) XMI::Topology(&ll, &ur);
	  } else if (mapping.size() == max - min + 1) {
		new (&topology_global) XMI::Topology(min, max);
	  } else {
		XMI_abortf("failed to build global-world topology %zd::%zd(%d) / %zd..%zd", mapping.size(), rectsize, mapping.globalDims(), min, max);
	  }
	  topology_global.subTopologyLocalToMe(&topology_local);
        }



        inline ~Global () {};

      public:

        XMI::BgpPersonality	personality;
	XMI::Mapping		mapping;

      private:

        XMI::BgpMapCache	_mapcache;
        void			*_memptr;
        size_t			_memsize;

      public:

	XMI::Atomic::BGP::LockBoxFactory lockboxFactory;

  };   // class Global
};     // namespace XMI

extern XMI::Global __global;

#endif // __xmi_components_sysdep_bgp_bgpglobal_h__
