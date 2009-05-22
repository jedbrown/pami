/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int8.cc
 * \brief Default C math routines for 8 bit signed integer operations.
 */

#include "math_bg_math.h"
#include "internal.h"


void _core_int8_band(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_bor(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_bxor(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_land(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_lor(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_lxor(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_max(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_min(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_prod(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_sum(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_pack(uint8_t *dst, const int8_t *src, int count) {
  _core_uint8_pack   (dst, (const uint8_t *)src, count);
}

void _core_int8_unpack(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_unpack   ((uint8_t *)dst, src, count);
}

void _core_int8_conv(uint8_t *dst, const int8_t *src, int count) {
#define OP(a) ((a)+(0x80))

#define TYPE uint8_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_not(int8_t *dst, const int8_t *src, int count) {
#define OP(a) (~(a))

#define TYPE int8_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_conv_not(uint8_t *dst, const int8_t *src, int count) {
#define OP(a) (~((a)+(0x80)))

#define TYPE uint8_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_pre_all(uint8_t *dst, const int8_t *src, int count) {
  _core_int8_conv(dst, src, count);
}

void _core_int8_post_all(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_conv((uint8_t *)dst, src, count);
}

void _core_int8_pre_min(uint8_t *dst, const int8_t *src, int count) {
  _core_int8_conv_not(dst, src, count);
}

void _core_int8_post_min(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_conv_not((uint8_t *)dst, src, count);
}

void _core_int8_band_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_band((int8_t *)dst, srcs, nsrc, count);
}

void _core_int8_band_unmarshall(int8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_int8_bor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_bor((int8_t *)dst, srcs, nsrc, count);
}

void _core_int8_bor_unmarshall(int8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_int8_bxor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_bxor((int8_t *)dst, srcs, nsrc, count);
}

void _core_int8_bxor_unmarshall(int8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_int8_land_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_land((int8_t *)dst, srcs, nsrc, count);
}

void _core_int8_land_unmarshall(int8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_int8_lor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_lor((int8_t *)dst, srcs, nsrc, count);
}

void _core_int8_lor_unmarshall(int8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_int8_lxor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_lxor((int8_t *)dst, srcs, nsrc, count);
}

void _core_int8_lxor_unmarshall(int8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_int8_max_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	uint8_t *d = (uint8_t *)dst;
	register unsigned shift = 0x80UL;
	for (; n < count; n++) {
		int8_t val = MAX(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MAX(val, srcs[o][n]);
		}
		d[n] = ((uint8_t)val + shift);
	}
}

void _core_int8_max_unmarshall(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_conv((uint8_t *)dst, src, count);
}

void _core_int8_min_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	register unsigned shift = 0x80UL;
	uint8_t *d=(uint8_t *)dst;
	for (; n < count; n++) {
		int8_t val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		d[n] = ~((uint8_t)val + shift);
	}
}

void _core_int8_min_unmarshall(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_conv_not((uint8_t *)dst, src, count);
}

void _core_int8_sum_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_sum((int8_t *)dst, srcs, nsrc, count);
}

void _core_int8_sum_unmarshall(int8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}
