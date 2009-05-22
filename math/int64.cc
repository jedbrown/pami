/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int64.cc
 * \brief Default C math routines for 32 bit signed integer operations.
 */

#include "math_bg_math.h"
#include "internal.h"


void _core_int64_band(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_bor(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_bxor(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_land(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_lor(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_lxor(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_max(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_min(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_prod(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_sum(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_conv(uint64_t *dst, const int64_t *src, int count) {
#define OP(a) ((a)+(0x8000000000000000ULL))

#define TYPE uint64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_not(int64_t *dst, const int64_t *src, int count) {
#define OP(a) (~(a))

#define TYPE int64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_conv_not(uint64_t *dst, const int64_t *src, int count) {
#define OP(a) (~((a)+(0x8000000000000000ULL)))

#define TYPE uint64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_pre_all(uint64_t *dst, const int64_t *src, int count) {
  _core_int64_conv(dst, src, count);
}

void _core_int64_post_all(int64_t *dst, const uint64_t *src, int count) {
  _core_uint64_conv((uint64_t *)dst, src, count);
}

void _core_int64_pre_min(uint64_t *dst, const int64_t *src, int count) {
  _core_int64_conv_not(dst, src, count);
}

void _core_int64_post_min(int64_t *dst, const uint64_t *src, int count) {
  _core_uint64_conv_not((uint64_t *)dst, src, count);
}

void _core_int64_band_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_band((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_band_unmarshall(int64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int64_t));
}

void _core_int64_bor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_bor((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_bor_unmarshall(int64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int64_t));
}

void _core_int64_bxor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_bxor((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_bxor_unmarshall(int64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int64_t));
}

void _core_int64_land_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_land((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_land_unmarshall(int64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int64_t));
}

void _core_int64_lor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_lor((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_lor_unmarshall(int64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int64_t));
}

void _core_int64_lxor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_lxor((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_lxor_unmarshall(int64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int64_t));
}

void _core_int64_max_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	uint64_t *d = (uint64_t *)dst;
	register uint64_t shift = 0x8000000000000000ULL;
	for (; n < count; n++) {
		int64_t val = MAX(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MAX(val, srcs[o][n]);
		}
		d[n] = ((uint64_t)val + shift);
	}
}

void _core_int64_max_unmarshall(int64_t *dst, const uint64_t *src, int count) {
  _core_uint64_conv((uint64_t *)dst, src, count);
}

void _core_int64_min_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	register uint64_t shift = 0x8000000000000000ULL;
	uint64_t *d = dst;
	for (; n < count; n++) {
		int64_t val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		d[n] = ~((uint64_t)val + shift);
	}
}

void _core_int64_min_unmarshall(int64_t *dst, const uint64_t *src, int count) {
  _core_uint64_conv_not((uint64_t *)dst, src, count);
}

void _core_int64_sum_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_sum((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_sum_unmarshall(int64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(int64_t));
}
