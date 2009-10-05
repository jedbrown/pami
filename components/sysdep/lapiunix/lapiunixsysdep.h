/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/sysdep/lapi/lapisysdep.h
 * \brief ???
 */

#ifndef __components_sysdep_lapi_lapisysdep_h__
#define __components_sysdep_lapi_lapisysdep_h__

#define XMI_SYSDEP_CLASS XMI::SysDep::LAPISysDep

#include "components/sysdep/SysDep.h"
#include "components/mapping/lapi/lapimapping.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include "components/time/lapi/lapitime.h"
#include "components/topology/lapi/lapitopology.h"

namespace XMI
{
  namespace SysDep
  {
    typedef SysDep<XMI::Memory::HeapMemoryManager, XMI::Mapping::LAPIMapping, XMI::Time::LAPITime, XMI::Topology::LAPI> LAPISysDep;
  };
};
#endif // __components_sysdep_lapi_lapisysdep_h__
