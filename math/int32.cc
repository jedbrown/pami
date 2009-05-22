/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int32.cc
 * \brief Default C math routines for 32 bit signed integer operations.
 */

#include "math_bg_math.h"
#include "internal.h"


void _core_int32_band(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_bor(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_bxor(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_land(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_lor(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_lxor(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_max(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_min(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_prod(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_sum(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_conv(uint32_t *dst, const int32_t *src, int count) {
#define OP(a) ((a)+(0x80000000UL))

#define TYPE uint32_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_not(int32_t *dst, const int32_t *src, int count) {
#define OP(a) (~(a))

#define TYPE int32_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_conv_not(uint32_t *dst, const int32_t *src, int count) {
#define OP(a) (~((a)+(0x80000000UL)))

#define TYPE uint32_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_int32_maxloc(int32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 > src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a < srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		dst[n].a = srcs[m][n].a;
		dst[n].b = srcs[m][n].b;
	}
}

void _core_int32_int32_minloc(int32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 < src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a > srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		dst[n].a = srcs[m][n].a;
		dst[n].b = srcs[m][n].b;
	}
}

void _core_int32_pre_all(uint32_t *dst, const int32_t *src, int count) {
  _core_int32_conv(dst, src, count);
}

void _core_int32_post_all(int32_t *dst, const uint32_t *src, int count) {
  _core_uint32_conv((uint32_t *)dst, src, count);
}

void _core_int32_pre_min(uint32_t *dst, const int32_t *src, int count) {
  _core_int32_conv_not(dst, src, count);
}

void _core_int32_post_min(int32_t *dst, const uint32_t *src, int count) {
  _core_uint32_conv_not((uint32_t *)dst, src, count);
}

void _core_int32_int32_pre_maxloc(uint32_int32_t *dst, const int32_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x80000000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = (uint32_t)src[n].a + shift;
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int32_int32_post_maxloc(int32_int32_t *dst, const uint32_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x80000000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = src[n].a - shift;
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}

void _core_int32_int32_pre_minloc(uint32_int32_t *dst, const int32_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x80000000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~((uint32_t)src[n].a + shift);
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int32_int32_post_minloc(int32_int32_t *dst, const uint32_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x80000000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(src[n].a - shift);
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}

void _core_int32_band_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_band((int32_t *)dst, srcs, nsrc, count);
}

void _core_int32_band_unmarshall(int32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (unsigned)count * sizeof(int32_t));
}

void _core_int32_bor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_bor((int32_t *)dst, srcs, nsrc, count);
}

void _core_int32_bor_unmarshall(int32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int32_t));
}

void _core_int32_bxor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_bxor((int32_t *)dst, srcs, nsrc, count);
}

void _core_int32_bxor_unmarshall(int32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int32_t));
}

void _core_int32_land_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_land((int32_t *)dst, srcs, nsrc, count);
}

void _core_int32_land_unmarshall(int32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int32_t));
}

void _core_int32_lor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_lor((int32_t *)dst, srcs, nsrc, count);
}

void _core_int32_lor_unmarshall(int32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int32_t));
}

void _core_int32_lxor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_lxor((int32_t *)dst, srcs, nsrc, count);
}

void _core_int32_lxor_unmarshall(int32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int32_t));
}

void _core_int32_max_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	uint32_t *d = dst;
	register unsigned shift = 0x80000000UL;
	for (; n < count; n++) {
		int32_t val = MAX(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MAX(val, srcs[o][n]);
		}
		d[n] = ((uint32_t)val + shift);
	}
}

void _core_int32_max_unmarshall(int32_t *dst, const uint32_t *src, int count) {
  _core_uint32_conv((uint32_t *)dst, src, count);
}

void _core_int32_int32_maxloc_marshall(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	uint32_int32_t *d = dst;
	register unsigned shift = 0x80000000UL;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 > src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a < srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		d[n].a = ((uint32_t)srcs[m][n].a + shift);
		d[n].b = ~(srcs[m][n].b);
	}
}

void _core_int32_int32_maxloc_unmarshall(int32_int32_t *dst, const uint32_int32_t *src, int count) {
  _core_int32_int32_post_maxloc(dst, src, count);
}

void _core_int32_min_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	register unsigned shift = 0x80000000UL;
	uint32_t *d=(uint32_t *)dst;
	for (; n < count; n++) {
		int32_t val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		d[n] = ~((uint32_t)val + shift);
	}
}

void _core_int32_min_unmarshall(int32_t *dst, const uint32_t *src, int count) {
  _core_uint32_conv_not((uint32_t *)dst, src, count);
}

void _core_int32_int32_minloc_marshall(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	uint32_int32_t *d = (uint32_int32_t *)dst;
	register unsigned shift = 0x80000000UL;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 < src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a > srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		d[n].a = ~((uint32_t)srcs[m][n].a + shift);
		d[n].b = ~(srcs[m][n].b);
	}
}

void _core_int32_int32_minloc_unmarshall(int32_int32_t *dst, const uint32_int32_t *src, int count) {
  _core_int32_int32_post_minloc(dst, src, count);
}

void _core_int32_sum_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_sum((int32_t *)dst, srcs, nsrc, count);
}

void _core_int32_sum_unmarshall(int32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int32_t));
}
