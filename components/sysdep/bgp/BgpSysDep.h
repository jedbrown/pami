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

#include "components/sysdep/SysDep.h"

#include "components/mapping/bgp/BgpMapping.h"
#include "components/memory/shmem/SharedMemoryManager.h"
#include "components/time/bgp/BgpTime.h"

#define NUM_CORES 4
#define NUM_THREADS 1

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

	class BgpSysDep : public SysDep<XMI::Memory::SharedMemoryManager, XMI::Mapping::BgpMapping, XMI::Time::BgpTime> {
	public:
		BgpSysDep() {}
		~BgpSysDep() {}

		XMI::SysDep::LockBoxFactory lockboxFactory(&this->mapping);
	}; // class BgpSysDep
  };
};
#endif // __components_sysdep_bgp_bgpsysdep_h__
