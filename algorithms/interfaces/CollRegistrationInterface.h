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
      /**
       *  This routine performs a local analyze, which populates
       *  a 64 bit unsigned integer (array) with some data.
       *  The out value is meant to be reduced and input into
       *  some reduction routine on the geometry.
       *  This allows the analyze routine to obtain global information
       *  The local analyze may populate the geometry with some
       *  algorithms that aren't dependent on global knowledge
       */
      inline pami_result_t register_local(size_t         context_id,
					  T_Geometry    *geometry,
					  uint64_t      *out_val,
					  int            &nelem);
      
      /**
       *  Once a global reduction or exchange has happened on the
       *  unsigned integer (array), the integer (array) is passed into
       *  the analyze_global routine.  The analyze_global routine
       *  will populate the list of algorithms with optimized routines
       */
      inline pami_result_t receive_global(size_t         context_id,
					  T_Geometry    *geometry,
					  uint64_t      *in_val,
					  int            nelem);
            
      /** Can we deprecate/remove this? */
      inline pami_result_t analyze(size_t         context_id,
                                   T_Geometry    *geometry,
                                   int            phase=0,
                                   uint64_t       *inout_val=NULL);

      inline pami_result_t analyze(size_t         context_id,
                                   T_Geometry    *geometry,
                                   uint64_t      *inout_val,
                                   int           *inout_nelem,
                                   int            phase=0);
    };

    template <class T_Collregistration, class T_Geometry>
    inline pami_result_t CollRegistration<T_Collregistration,T_Geometry>::register_local(size_t      context_id,
											 T_Geometry *geometry,
											 uint64_t   *out_val,
											 int         &n)
    {
      return static_cast<T_Collregistration*>(this)->register_local_impl(context_id, geometry, out_val, n);
    }

    template <class T_Collregistration, class T_Geometry>
    inline pami_result_t CollRegistration<T_Collregistration,T_Geometry>::receive_global(size_t      context_id,
                                                                                         T_Geometry *geometry,
                                                                                         uint64_t   *in_val,
											 int         n)
    {
      return static_cast<T_Collregistration*>(this)->receive_global_impl(context_id, geometry, in_val, n);
    }

    template <class T_Collregistration, class T_Geometry>
    inline pami_result_t CollRegistration<T_Collregistration,T_Geometry>::analyze(size_t      context_id,
                                                                                  T_Geometry *geometry,
                                                                                  int         phase,
                                                                                  uint64_t    *inout_val)
    {
      return static_cast<T_Collregistration*>(this)->analyze_impl(context_id, geometry, phase, inout_val);
    }

    template <class T_Collregistration, class T_Geometry>
    inline pami_result_t CollRegistration<T_Collregistration,T_Geometry>::analyze(size_t         context_id,
                                                                                  T_Geometry    *geometry,
                                                                                  uint64_t      *inout_val,
                                                                                  int           *inout_nelem,
                                                                                  int            phase)
    {
      return static_cast<T_Collregistration*>(this)->analyze_impl(context_id, geometry,
                                                                  inout_val,inout_nelem,
                                                                  phase);
    }


  }; // namespace CollRegistration
}; // namespace PAMI

#endif
