/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint64.cc
 * \brief Default C math routines for 64 bit unsigned integer operations.
 */

#include "math_bg_math.h"
#include "internal.h"


void _core_uint64_band(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_bor(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_bxor(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_land(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_lor(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_lxor(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_max(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_min(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_prod(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_sum(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_pack(uint64_t *dst, const uint64_t *src, int count) {
  uint8_t *d = (uint8_t *)dst;
  int n = count >> 3;
  int i = 0;
  int j = 0;
  while ( n-- ) {
    d[i] = 0;
    if (src[j++]) d[i] |= 0x080;
    if (src[j++]) d[i] |= 0x040;
    if (src[j++]) d[i] |= 0x020;
    if (src[j++]) d[i] |= 0x010;
    if (src[j++]) d[i] |= 0x008;
    if (src[j++]) d[i] |= 0x004;
    if (src[j++]) d[i] |= 0x002;
    if (src[j++]) d[i] |= 0x001;
    i++;
  }

  n = count & 0x07;
  d[i] = 0;
  if ( n-- && src[j++]) d[i] |= 0x080;
  if ( n-- && src[j++]) d[i] |= 0x040;
  if ( n-- && src[j++]) d[i] |= 0x020;
  if ( n-- && src[j++]) d[i] |= 0x010;
  if ( n-- && src[j++]) d[i] |= 0x008;
  if ( n-- && src[j++]) d[i] |= 0x004;
  if ( n-- && src[j++]) d[i] |= 0x002;
}

void _core_uint64_unpack(uint64_t *dst, const uint64_t *src, int count) {
  uint8_t *s = (uint8_t *)src;
  int n = count >> 3;
  int i = 0;
  int j = 0;
  while ( n-- ) {
    dst[j++] = s[i] & 0x080;
    dst[j++] = s[i] & 0x040;
    dst[j++] = s[i] & 0x020;
    dst[j++] = s[i] & 0x010;
    dst[j++] = s[i] & 0x008;
    dst[j++] = s[i] & 0x004;
    dst[j++] = s[i] & 0x002;
    dst[j++] = s[i] & 0x001;
    i++;
  }

  n = count & 0x07;
  if ( n-- ) dst[j++] = s[i] & 0x080;
  if ( n-- ) dst[j++] = s[i] & 0x040;
  if ( n-- ) dst[j++] = s[i] & 0x020;
  if ( n-- ) dst[j++] = s[i] & 0x010;
  if ( n-- ) dst[j++] = s[i] & 0x008;
  if ( n-- ) dst[j++] = s[i] & 0x004;
  if ( n-- ) dst[j++] = s[i] & 0x002;
}

void _core_uint64_conv(uint64_t *dst, const uint64_t *src, int count) {
#define OP(a) ((a)-(0x8000000000000000ULL))

#define TYPE int64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_not(uint64_t *dst, const uint64_t *src, int count) {
#define OP(a) (~(a))

#define TYPE uint64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_conv_not(uint64_t *dst, const uint64_t *src, int count) {
#define OP(a) (~((a)-(0x8000000000000000ULL)))

#define TYPE int64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_pre_all(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _core_uint64_post_all(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _core_uint64_pre_min(uint64_t *dst, const uint64_t *src, int count) {
  _core_uint64_not(dst, src, count);
}

void _core_uint64_post_min(uint64_t *dst, const uint64_t *src, int count) {
  _core_uint64_not(dst, src, count);
}

void _core_uint64_band_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_band(dst, srcs, nsrc, count);
}

void _core_uint64_band_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _core_uint64_bor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_bor(dst, srcs, nsrc, count);
}

void _core_uint64_bor_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _core_uint64_bxor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_bxor(dst, srcs, nsrc, count);
}

void _core_uint64_bxor_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _core_uint64_land_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_land(dst, srcs, nsrc, count);
}

void _core_uint64_land_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _core_uint64_lor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_lor(dst, srcs, nsrc, count);
}

void _core_uint64_lor_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _core_uint64_lxor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_lxor(dst, srcs, nsrc, count);
}

void _core_uint64_lxor_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _core_uint64_max_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_max(dst, srcs, nsrc, count);
}

void _core_uint64_max_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _core_uint64_min_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	for (; n < count; n++) {
		uint64_t val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		dst[n] = ~(val);
	}
}

void _core_uint64_min_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
  _core_uint64_not(dst, src, count);
}

void _core_uint64_sum_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_sum(dst, srcs, nsrc, count);
}

void _core_uint64_sum_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}
