/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/sysdep/Sysdep.h
/// \brief ???
///
#ifndef __components_sysdep_sysdep_h__
#define __components_sysdep_sysdep_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace SysDep
  {
    ///
    /// \param T_Mapping Platform-specific mapping class
    ///
//    template <class T_Memory, class T_Mapping>
    template <class T_Memory, unsigned T_Foo>
    class SysDep
    {
      public:
        inline SysDep () :
          mm ()//,
          //mapping ()
        {
          //mapping.init (mm);
        };

        T_Memory  mm;
        //T_Mapping mapping;
    };
  };
};
#endif // __components_sysdep_sysdep_h__
