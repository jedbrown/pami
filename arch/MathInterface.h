/**
 * \file arch/MathInterface.h
 * \brief ...
 */

#ifndef __arch_MathInterface_h__
#define __arch_MathInterface_h__

#include <stdlib.h>

#undef TRACE_ERR
#define TRACE_ERR(x) // fprintf x

namespace PAMI
{
  namespace Math
  {
    template <typename T>
    inline T GCD(T a, T b)
    {
      T t;

      while (b != 0)
        {
          t = b;
          b = a % b;
          a = t;
        }

      return a;
    }


    template <typename T>
    static void copy (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template copy, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        dst[i] = src[i];
    }

    template <typename T, unsigned N>
    inline void copy (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template copy, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        dst[i] = src[i];
    }

    template <typename T>
    inline void max (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template max, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        if (src[i] > dst[i])
          dst[i] = src[i];
    }

    template <typename T, unsigned N>
    inline void max (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template max, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        if (src[i] > dst[i])
          dst[i] = src[i];
    }

    template <typename T>
    inline void min (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template min, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        if (src[i] < dst[i])
          dst[i] = src[i];
    }

    template <typename T, unsigned N>
    inline void min (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template min, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        if (src[i] < dst[i])
          dst[i] = src[i];
    }

    template <typename T>
    inline void sum (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template sum, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        dst[i] += src[i];
    }

    template <typename T, unsigned N>
    inline void sum (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template sum, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        dst[i] += src[i];
    }

    template <typename T>
    inline void prod (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template prod, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        dst[i] *= src[i];
    }

    template <typename T, unsigned N>
    inline void prod (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template prod, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        dst[i] *= src[i];
    }

    template <typename T>
    inline void land (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template land, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        dst[i] = dst[i] && src[i];
    }

    template <typename T, unsigned N>
    inline void land (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template land, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        dst[i] = dst[i] && src[i];
    }

    template <typename T>
    inline void lor (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template lor, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        dst[i] = dst[i] || src[i];
    }

    template <typename T, unsigned N>
    inline void lor (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template lor, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        dst[i] = dst[i] || src[i];
    }

    template <typename T>
    inline void lxor (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template lxor, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        dst[i] = dst[i] ^ src[i];
    }

    template <typename T, unsigned N>
    inline void lxor (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template lxor, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        dst[i] = dst[i] ^ src[i];
    }

    template <typename T>
    inline void band (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template band, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        dst[i] = dst[i] & src[i];
    }

    template <typename T, unsigned N>
    inline void band (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template band, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        dst[i] = dst[i] & src[i];
    }

    template <typename T>
    inline void bor (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template bor, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        dst[i] = dst[i] | src[i];
    }

    template <typename T, unsigned N>
    inline void bor (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template bor, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        dst[i] = dst[i] | src[i];
    }

    template <typename T>
    inline void bxor (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template bxor, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        dst[i] = dst[i] ^ src[i];
    }

    template <typename T, unsigned N>
    inline void bxor (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template bxor, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        dst[i] = dst[i] ^ src[i];
    }

    template <typename T>
    inline void minloc (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template minloc, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        if (src[i].value < dst[i].value)
          dst[i] = src[i];
    }

    template <typename T, unsigned N>
    inline void minloc (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template minloc, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        if (src[i].value < dst[i].value)
          dst[i] = src[i];
    }

    template <typename T>
    inline void maxloc (T * dst, T * src, size_t count)
    {
      TRACE_ERR((stderr, "template maxloc, count = %zu\n", count));
      unsigned i;

      for (i = 0; i < count; i++)
        if (src[i].value > dst[i].value)
          dst[i] = src[i];
    }

    template <typename T, unsigned N>
    inline void maxloc (T (&dst)[N], T (&src)[N])
    {
      TRACE_ERR((stderr, "template maxloc, N = %d\n", N));
      unsigned i;

      for (i = 0; i < N; i++)
        if (src[i].value > dst[i].value)
          dst[i] = src[i];
    }

  };  // namespace Math
};    // namespace PAMI

#endif // __arch_MathInterface_h__

