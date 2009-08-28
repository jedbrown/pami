/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/sysdep/bgp/BgpSysdep.h
/// \brief ???
///
#ifndef __components_sysdep_bgp_bgpsysdep_h__
#define __components_sysdep_bgp_bgpsysdep_h__

#include "components/sysdep/SysDep.h"

#include "components/mapping/bgp/BgpMapping.h"
#include "components/memory/shmem/SharedMemoryManager.h"


namespace XMI
{
  namespace SysDep
  {
    typedef Interface::SysDep<Mapping::BgpMapping, Memory::SharedMemoryManager> BgpSysDep;
  };
};
#endif // __components_sysdep_bgp_bgpsysdep_h__
