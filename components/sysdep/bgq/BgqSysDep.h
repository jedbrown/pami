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

#include "components/sysdep/SysDep.h"

#include "components/mapping/bgq/BgqMapping.h"
#include "components/memory/shmem/SharedMemoryManager.h"
#include "components/time/bgq/BgqTime.h"


namespace XMI
{
  namespace SysDep
  {
    typedef SysDep<XMI::Memory::SharedMemoryManager, XMI::Mapping::BgqMapping, XMI::Time::BgqTime> BgqSysDep;
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
