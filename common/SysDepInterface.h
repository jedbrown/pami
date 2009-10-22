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

namespace XMI
{
  namespace Interface
  {
    ///
    /// \param T_Memory   Platform-specific memory manager class
    /// \param T_Mapping  Platform-specific mapping class
    /// \param T_Time     Platform-specific time class
    /// \param T_Topology Platform-specific topology class
    ///
    template <class T_Memory>
    class SysDep
    {
      public:
        inline SysDep () :
            mm ()
        {
        };

        T_Memory   mm;
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
