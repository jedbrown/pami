/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/sysdep/SysDep.h
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
    /// \param T_Memory  Platform-specific memory manager class
    /// \param T_Mapping Platform-specific mapping class
    /// \param T_Time    Platform-specific time class
    ///
    template <class T_Memory, class T_Mapping, class T_Time>
    class SysDep
    {
      public:
        inline SysDep () :
          mm (),
          mapping (),
          time ()
        {
          mapping.init ();
        };

        T_Memory  mm;
        T_Mapping mapping;
        T_Time    time;
    };

    class NullSysDep
    {
    }; // class NullSysDep
  };
};
#endif // __components_sysdep_sysdep_h__
