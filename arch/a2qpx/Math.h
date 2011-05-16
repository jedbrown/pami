/**
 * \file arch/a2qpx/Math.h
 * \brief ...
 */

#ifndef __arch_a2qpx_Math_h__
#define __arch_a2qpx_Math_h__

#include "arch/MathInterface.h"

namespace PAMI
{
  namespace Math
  {
#if 0
    template <>
    inline void sum<double> (double * dst, double * src, size_t count)
    {
      // platform-optimized double-sum math
      fprintf (stderr, "a2qpx double sum template specialization.. dst = %p, src = %p, count = %zu\n", dst, src, count);

      unsigned i;

      for (i = 0; i < count; i++)
        {
          fprintf (stderr, "a2qpx double sum template specialization .. src[%d] = %e, dst[%d] = %e -> %e\n", i, src[i], i, dst[i], dst[i] + src[i]);
          dst[i] += src[i];
        }
    }

    inline void sum (double (&dst)[64], double (&src)[64])
    {
      // platform-optimized double-sum 64 count math
      fprintf (stderr, "a2qpx sum 64\n");
    }
#endif
  };
};
#endif // __arch_a2qpx_Math_h__
