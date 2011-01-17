/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_lapiunix_Global_h__
#define __common_lapiunix_Global_h__
///
/// \file common/lapiunix/Global.h
/// \brief Global Object
///
/// This global object is constructed before main() and is a container class
/// for all other classes that need to be constructed and initialized before
/// the application starts.
///

#include "Platform.h"
#include "util/common.h"
#include "common/GlobalInterface.h"
#include "Mapping.h"
#include "Topology.h"
#include "common/lapiunix/lapifunc.h"
#include <map>
#include "components/memory/heap/HeapMemoryManager.h"
#include "components/memory/shmem/SharedMemoryManager.h"

namespace PAMI
{
    class Global : public Interface::Global<PAMI::Global>
    {
      public:

        inline Global () :
          Interface::Global<PAMI::Global>(),
          mapping()
        {
          const char   * shmemfile = "/pami-global-shmem";
          // LAPI::Time gets its own clockMHz
          time.init(0);
	  heap_mm = new (_heap_mm) PAMI::Memory::HeapMemoryManager();
          PAMI::Memory::MemoryManager::heap_mm = heap_mm;
	  // get jobid from POE?
	  size_t jobid = 0;
	  if(getenv("MP_PARTITION"))
            jobid = atoi(getenv("MP_PARTITION"));
	  shared_mm = new (_shared_mm) PAMI::Memory::SharedMemoryManager(jobid, heap_mm);
          PAMI::Memory::MemoryManager::shared_mm = shared_mm;
	  mm.init(shared_mm, 1*1024*1024, 1, 1, 0, "/pami-lapi-global-shmem");
          PAMI::Memory::MemoryManager::shm_mm    = &mm;
            {
              size_t min=0, max=0;
              mapping.init(min, max);
            }
      };
    inline ~Global () {};
  public:
    PAMI::Mapping		   mapping;
    PAMI::Memory::GenMemoryManager mm;
    std::map<lapi_handle_t,void*> _context_to_device_table;
    std::map<lapi_handle_t,void*> _id_to_device_table;    
  };   // class Global
};     // namespace PAMI

extern PAMI::Global __global;

#endif // __pami_common_lapiunix_global_h__
