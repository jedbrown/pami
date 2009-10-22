/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/lapiunix/SysDep.h
 * \brief ???
 */

#ifndef __components_sysdep_lapi_lapisysdep_h__
#define __components_sysdep_lapi_lapisysdep_h__

#undef __bgp__
#undef __bgq__
#define XMI_SYSDEP_CLASS XMI::SysDep


#include "common/SysDepInterface.h"
#include "Platform.h"
#include "Mapping.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include "Wtime.h"
#include "Topology.h"

namespace XMI
{
  typedef Interface::SysDep<XMI::Memory::HeapMemoryManager> SysDep;
};
#endif // __components_sysdep_lapi_lapisysdep_h__
