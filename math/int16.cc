/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int16.cc
 * \brief Default C math routines for 16 bit signed integer operations.
 */

#include "math_bg_math.h"
#include "internal.h"


void _core_int16_band(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_bor(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_bxor(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_land(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_lor(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_lxor(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_max(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_min(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_prod(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_sum(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_conv(uint16_t *dst, const int16_t *src, int count) {
#define OP(a) ((a)+(0x8000))

#define TYPE uint16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_not(int16_t *dst, const int16_t *src, int count) {
#define OP(a) (~(a))

#define TYPE int16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_conv_not(uint16_t *dst, const int16_t *src, int count) {
#define OP(a) (~((a)+(0x8000)))

#define TYPE uint16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_int32_maxloc(int16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
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

void _core_int16_int32_minloc(int16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
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


void _core_int16_pre_all(uint16_t *dst, const int16_t *src, int count) {
  _core_int16_conv(dst, src, count);
}

void _core_int16_post_all(int16_t *dst, const uint16_t *src, int count) {
  _core_uint16_conv((uint16_t *)dst, src, count);
}

void _core_int16_pre_min(uint16_t *dst, const int16_t *src, int count) {
  _core_int16_conv_not(dst, src, count);
}

void _core_int16_post_min(int16_t *dst, const uint16_t *src, int count) {
  _core_uint16_conv_not((uint16_t *)dst, src, count);
}

void _core_int16_int32_pre_maxloc(uint16_int32_t *dst, const int16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = src[n].a + shift;
      dst[n].z = 0;
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int16_int32_post_maxloc(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = src[n].a - shift;
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}

void _core_int16_int32_pre_minloc(uint16_int32_t *dst, const int16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(src[n].a + shift);
      dst[n].z = 0;
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int16_int32_post_minloc(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(src[n].a - shift);
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}

void _core_int16_band_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_band((int16_t *)dst, srcs, nsrc, count);
}

void _core_int16_band_unmarshall(int16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int16_t));
}

void _core_int16_bor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_bor((int16_t *)dst, srcs, nsrc, count);
}

void _core_int16_bor_unmarshall(int16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int16_t));
}

void _core_int16_bxor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_bxor((int16_t *)dst, srcs, nsrc, count);
}

void _core_int16_bxor_unmarshall(int16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int16_t));
}

void _core_int16_land_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_land((int16_t *)dst, srcs, nsrc, count);
}

void _core_int16_land_unmarshall(int16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int16_t));
}

void _core_int16_lor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_lor((int16_t *)dst, srcs, nsrc, count);
}

void _core_int16_lor_unmarshall(int16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int16_t));
}

void _core_int16_lxor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_lxor((int16_t *)dst, srcs, nsrc, count);
}

void _core_int16_lxor_unmarshall(int16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int16_t));
}

void _core_int16_max_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	uint16_t *d = dst;
	register unsigned shift = 0x8000UL;
	for (; n < count; n++) {
		int16_t val = MAX(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MAX(val, srcs[o][n]);
		}
		d[n] = ((uint16_t)val + shift);
	}
}

void _core_int16_max_unmarshall(int16_t *dst, const uint16_t *src, int count) {
  _core_uint16_conv((uint16_t *)dst, src, count);
}

void _core_int16_int32_maxloc_marshall(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	uint16_int32_t *d = dst;
	register unsigned shift = 0x8000UL;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 > src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a < srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		d[n].a = ((uint16_t)srcs[m][n].a + shift);
		d[n].b = ~(srcs[m][n].b);
	}

}

void _core_int16_int32_maxloc_unmarshall(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  _core_int16_int32_post_maxloc(dst, src, count);
}

void _core_int16_min_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	register unsigned shift = 0x8000UL;
	uint16_t *d=(uint16_t *)dst;
	for (; n < count; n++) {
		int16_t val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		d[n] = ~((uint16_t)val + shift);
	}
}

void _core_int16_min_unmarshall(int16_t *dst, const uint16_t *src, int count) {
  _core_uint16_conv_not((uint16_t *)dst, src, count);
}

void _core_int16_int32_minloc_marshall(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	uint16_int32_t *d = (uint16_int32_t *)dst;
	register unsigned shift = 0x8000UL;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 < src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a > srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		d[n].a = ~(srcs[m][n].a + shift);
		d[n].b = ~(srcs[m][n].b);
	}
}

void _core_int16_int32_minloc_unmarshall(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  _core_int16_int32_post_minloc(dst, src, count);
}

void _core_int16_sum_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_sum((int16_t *)dst, srcs, nsrc, count);
}

void _core_int16_sum_unmarshall(int16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int16_t));
}
