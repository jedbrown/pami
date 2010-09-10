/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __common_GlobalInterface_h__
#define __common_GlobalInterface_h__

///
/// \file common/GlobalInterface.h
/// \brief Common base global object
///
/// This global object is constructed before main() and is a container class
/// for all other classes that need to be constructed and initialized before
/// the application starts.
///

#include "Platform.h"
#include "util/common.h"
#include "Topology.h"
#include "Ptime.h"
#include "components/memory/MemoryManager.h"

namespace PAMI
{
namespace Interface
{
    template<class T_Global>
    class Global
    {
      public:

        inline Global() :
          time(),
          topology_global(),
          topology_local(),
	  heap_mm(),
	  shared_mm()
        {
                ENFORCE_CLASS_MEMBER(T_Global,mapping);
        }

        inline ~Global () {};

      public:

        PAMI::Time	time;
        PAMI::Topology	topology_global;
        PAMI::Topology	topology_local;
	PAMI::Memory::MemoryManager heap_mm;	// re-constructed later
	PAMI::Memory::MemoryManager shared_mm;	// re-constructed later

        /// Global location for the processor name
        char processor_name[128];

  };   // class Global
};     // namespace Interface
};     // namespace PAMI

#endif // __pami_common_global_h__
