/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/bgp/SysDep.h
/// \brief ???
///
#ifndef __components_sysdep_bgp_bgpsysdep_h__
#define __components_sysdep_bgp_bgpsysdep_h__

#define XMI_SYSDEP_CLASS XMI::SysDep

#include "common/SysDep.h"

#define ENABLE_LOCKBOX

// These must be defined early
#define NUM_CORES 4
#define NUM_SMT 1

#include "Topology.h"
#include "Mapping.h"
#include "components/memory/shmem/SharedMemoryManager.h"
#include "Time.h"
#include "common/bgp/LockBoxFactory.h"

namespace XMI
{
    // TODO -- need to make a different shared memory manager so that multiple
    // contexts can get shared memory .. right now the second context to call
    // mm.init() will fail.


    class SysDep : public Interface::SysDep<XMI::Memory::SharedMemoryManager, XMI::Mapping, XMI::Time, XMI::Topology>
    {
      public:
        SysDep()
#ifdef ENABLE_LOCKBOX
          :
          lockboxFactory(&this->mapping)
#endif
        {}

        ~SysDep() {}

#ifdef ENABLE_LOCKBOX
        XMI::Atomic::BGP::LockBoxFactory lockboxFactory;
#endif
    }; // class SysDep
};
#endif // __components_sysdep_bgp_bgpsysdep_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
