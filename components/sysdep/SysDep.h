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

#include "xmi.h"

namespace XMI
{
  namespace SysDep
  {
    namespace Interface
    {
      ///
      /// \param T_Mapping Platform-specific mapping class
      ///
      template <class T_Mapping, class T_Memory>
      class SysDep
      {
        public:

          inline SysDep () {};

          T_Mapping & mapping;
          T_Memory  & mm;
      }
    };
  };
};
#endif // __components_sysdep_sysdep_h__
