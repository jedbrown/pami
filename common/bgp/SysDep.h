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
#ifndef __common_bgp_SysDep_h__
#define __common_bgp_SysDep_h__

#define XMI_SYSDEP_CLASS XMI::SysDep

#include "common/SysDepInterface.h"

// These must be defined early
#include "Platform.h"

namespace XMI
{
    // TODO -- need to make a different shared memory manager so that multiple
    // contexts can get shared memory .. right now the second context to call
    // mm.init() will fail.

    typedef Interface::SysDep SysDep;
};
#endif // __components_sysdep_bgp_bgpsysdep_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
