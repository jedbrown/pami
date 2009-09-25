/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/sysdep/bgq/BgqSysDep.h
/// \brief ???
///
#ifndef __components_sysdep_bgq_bgqsysdep_h__
#define __components_sysdep_bgq_bgqsysdep_h__

#define XMI_SYSDEP_CLASS XMI::SysDep::BgqSysDep

#include "components/sysdep/SysDep.h"

// These must be defined early
#define NUM_CORES 16
#define NUM_SMT 4

#include "components/mapping/bgq/BgqMapping.h"
#include "components/memory/shmem/SharedMemoryManager.h"
#include "components/time/bgq/BgqTime.h"
#include "components/topology/bgq/BgqTopology.h"

namespace XMI
{
  namespace SysDep
  {
    // TODO -- need to make a different shared memory manager so that multiple
    // contexts can get shared memory .. right now the second context to call
    // mm.init() will fail.
    typedef SysDep<XMI::Memory::SharedMemoryManager, XMI::Mapping::BgqMapping, XMI::Time::BgqTime, XMI::Topology::BgqTopology> BgqSysDep;
 //  typedef SysDep<XMI::Memory::SharedMemoryManager,
   //                XMI::Mapping::BgqMapping<XMI::Memory::SharedMemoryManager > BgqSysDep;
//    class BgqSysDep : public SysDep<XMI::Memory::SharedMemoryManager<4096>,
  //                                  XMI::Mapping::BgqMapping<XMI::Memory::SharedMemoryManager<4096> >
    //{
      //public:
    //};
  };
};
#endif // __components_sysdep_bgq_bgqsysdep_h__
