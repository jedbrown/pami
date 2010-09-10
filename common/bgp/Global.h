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
#include "common/bgp/LockBoxFactory.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include "components/memory/shmem/SharedMemoryManager.h"

namespace PAMI
{
    class Global : public Interface::Global<PAMI::Global>
    {
      public:

        inline Global () :
          Interface::Global<PAMI::Global>(),
          personality (),
          mapping(personality),
	  mm(),
          _mapcache (),
          lockboxFactory()
        {
          //Interface::Global<PAMI::Global>::time.init(personality.clockMHz());
          time.init (personality.clockMHz());

	  new (&heap_mm) PAMI::Memory::HeapMemoryManager();
	  if (personality.tSize() == 1) {
		// There is no shared memory, so don't try. Fake using heap.
		new (&shared_mm) PAMI::Memory::HeapMemoryManager();
	  } else {
		new (&shared_mm) PAMI::Memory::SharedMemoryManager();
	  }


          //allocateMemory ();

          char   * shmemfile = "/unique-pami-global-shmem-file";
          size_t   bytes     = 1024*1024;
          size_t   pagesize  = 4096;

          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

	  mm.init(&shared_mm, size, 1, 0, shmemfile);

          lockboxFactory.init(&mapping);
          pami_coord_t ll, ur;
          size_t min, max;
          unsigned char tl[PAMI_MAX_DIMS];

          PAMI::Topology::static_init(&mapping);
          _mapcache.getMappingInit(ll, ur, min, max);
          size_t rectsize = 1;
          for (unsigned d = 0; d < mapping.globalDims(); ++d) {
                rectsize *= (ur.u.n_torus.coords[d] - ll.u.n_torus.coords[d] + 1);
          }
          tl[0] = personality.isTorusX();
          tl[1] = personality.isTorusY();
          tl[2] = personality.isTorusZ();
          tl[3] = 1;
          if (mapping.size() == rectsize) {
                new (&topology_global) PAMI::Topology(&ll, &ur, tl);
          } else if (mapping.size() == max - min + 1) {
                new (&topology_global) PAMI::Topology(min, max);
          } else {
                PAMI_abortf("failed to build global-world topology %zu::%zu(%d) / %zu..%zu", mapping.size(), rectsize, mapping.globalDims(), min, max);
          }
          topology_global.subTopologyLocalToMe(&topology_local);
        }



        inline ~Global () {};

      public:

        PAMI::BgpPersonality	personality;
        PAMI::Mapping		mapping;
	PAMI::Memory::MemoryManager mm;

      private:

        PAMI::BgpMapCache	_mapcache;

      public:

        PAMI::Atomic::BGP::LockBoxFactory lockboxFactory;

  };   // class Global
};     // namespace PAMI

extern PAMI::Global __global;

#endif // __pami_components_sysdep_bgp_bgpglobal_h__
