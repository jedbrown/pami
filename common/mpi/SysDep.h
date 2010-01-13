/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/mpi/SysDep.h
 * \brief ???
 */

#ifndef __common_mpi_SysDep_h__
#define __common_mpi_SysDep_h__

#undef __bgp__
#undef __bgq__

#define XMI_SYSDEP_CLASS XMI::SysDep

#include "common/SysDepInterface.h"
#include "Platform.h"

namespace XMI
{
    typedef Interface::SysDep SysDep;
};
#endif // __components_sysdep_mpi_mpisysdep_h__
