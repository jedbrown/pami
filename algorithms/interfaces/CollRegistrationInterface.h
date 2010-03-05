/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/interfaces/CollRegistrationInterface.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_CollRegistrationInterface_h__
#define __algorithms_interfaces_CollRegistrationInterface_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace CollRegistration
  {
    template <class T_Collregistration, class T_Geometry>
    class CollRegistration
    {
    public:
      inline CollRegistration()
        {
        }
      inline xmi_result_t analyze(size_t         context_id,
                                  T_Geometry    *geometry);
    };

    template <class T_Collregistration, class T_Geometry>
    inline xmi_result_t CollRegistration<T_Collregistration,T_Geometry>::analyze(size_t      context_id,
                                                                                 T_Geometry *geometry)
    {
      return static_cast<T_Collregistration*>(this)->analyze_impl(context_id, geometry);
    }
  }; // namespace CollRegistration
}; // namespace XMI

#endif
