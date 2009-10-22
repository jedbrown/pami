/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/SysDepInterface.h
/// \brief ???
///
#ifndef __common_SysDepInterface_h__
#define __common_SysDepInterface_h__

#include "sys/xmi.h"
#include "util/common.h"

#include "components/memory/MemoryManager.h"

namespace XMI
{
  namespace Interface
  {
    class SysDep
    {
      public:
        inline SysDep (Memory::MemoryManager & mm_ref) :
            mm (mm_ref)
        {
        };

        Memory::MemoryManager & mm;
    };	// class SysDep

    class NullSysDep
    {
    };	// class NullSysDep
  };	// namespace Interface
};	// namespace XMI
#endif // __components_sysdep_sysdep_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
