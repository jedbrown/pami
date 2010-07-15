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

#include <pami.h>

namespace PAMI
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
      inline pami_result_t analyze(size_t         context_id,
                                  T_Geometry    *geometry,
				  int phase = 0);
    };

    template <class T_Collregistration, class T_Geometry>
    inline pami_result_t CollRegistration<T_Collregistration,T_Geometry>::analyze(size_t      context_id,
                                                                                 T_Geometry *geometry,
										 int phase)
    {
      return static_cast<T_Collregistration*>(this)->analyze_impl(context_id, geometry, phase);
    }
  }; // namespace CollRegistration
}; // namespace PAMI

#endif
