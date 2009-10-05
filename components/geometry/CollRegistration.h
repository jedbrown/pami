/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/geometry/CollRegistration.h
 * \brief ???
 */

#ifndef   __xmi_collregistration__h__
#define   __xmi_collregistration__h__

#include "sys/xmi.h"

namespace XMI
{
  namespace CollRegistration
  {
    template <class T_Collregistration,class T_Geometry, class T_Collfactory>
    class CollRegistration
    {
    public:
      inline CollRegistration()
        {
        }
      inline T_Collfactory * analyze(T_Geometry * geometry);

    }; // class CollRegistration

    template <class T_Collregistration, class T_Geometry, class T_Collfactory>
    inline T_Collfactory * CollRegistration<T_Collregistration,T_Geometry,T_Collfactory>::analyze(T_Geometry *geometry)
    {
      return static_cast<T_Collregistration*>(this)->analyze_impl(geometry);
    }


  }; // namespace CollRegistration
}; // namespace XMI

#endif
