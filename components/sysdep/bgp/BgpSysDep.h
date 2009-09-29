/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/sysdep/bgp/BgpSysDep.h
/// \brief ???
///
#ifndef __components_sysdep_bgp_bgpsysdep_h__
#define __components_sysdep_bgp_bgpsysdep_h__

#define XMI_SYSDEP_CLASS XMI::SysDep::BgpSysDep

#include "components/sysdep/SysDep.h"

#undef ENABLE_LOCKBOX

// These must be defined early
#define NUM_CORES 4
#define NUM_SMT 1

#include "components/topology/bgp/BgpTopology.h"
#include "components/mapping/bgp/BgpMapping.h"
#include "components/memory/shmem/SharedMemoryManager.h"
#include "components/time/bgp/BgpTime.h"
#include "components/sysdep/bgp/LockBoxFactory.h"

namespace XMI
{
  namespace SysDep
  {
    // TODO -- need to make a different shared memory manager so that multiple
    // contexts can get shared memory .. right now the second context to call
    // mm.init() will fail.


#if 0
    typedef SysDep<XMI::Memory::SharedMemoryManager, XMI::Mapping::BgpMapping, XMI::Time::BgpTime> BgpSysDep;
//  typedef SysDep<XMI::Memory::SharedMemoryManager,
    //                XMI::Mapping::BgpMapping<XMI::Memory::SharedMemoryManager > BgpSysDep;
//    class BgpSysDep : public SysDep<XMI::Memory::SharedMemoryManager<4096>,
    //                                  XMI::Mapping::BgpMapping<XMI::Memory::SharedMemoryManager<4096> >
    //{
    //public:
    //};
#endif

    class BgpSysDep : public SysDep<XMI::Memory::SharedMemoryManager, XMI::Mapping::BgpMapping, XMI::Time::BgpTime, XMI::Topology::BgpTopology>
    {
      public:
        BgpSysDep()
#ifdef ENABLE_LOCKBOX
          :
          lockboxFactory(&this->mapping)
#endif
        {}

        ~BgpSysDep() {}

#ifdef ENABLE_LOCKBOX
        XMI::Atomic::BGP::LockBoxFactory lockboxFactory;
#endif
    }; // class BgpSysDep
  };
};
#endif // __components_sysdep_bgp_bgpsysdep_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
