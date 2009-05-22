/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint16.cc
 * \brief Default C math routines for 16 bit unsigned integer operations.
 */

#include "math_bg_math.h"
#include "internal.h"


void _core_uint16_band(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_bor(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_bxor(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_land(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_lor(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_lxor(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_max(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_min(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_prod(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_sum(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_pack(uint16_t *dst, const uint16_t *src, int count) {
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

void _core_uint16_unpack(uint16_t *dst, const uint16_t *src, int count) {
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

void _core_uint16_conv(uint16_t *dst, const uint16_t *src, int count) {
#define OP(a) ((a)-(0x8000))

#define TYPE int16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_not(uint16_t *dst, const uint16_t *src, int count) {
#define OP(a) (~(a))

#define TYPE uint16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_conv_not(uint16_t *dst, const uint16_t *src, int count) {
#define OP(a) (~((a)-(0x8000)))

#define TYPE int16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_pre_all(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_post_all(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_pre_min(uint16_t *dst, const uint16_t *src, int count) {
  _core_uint16_not(dst, src, count);
}

void _core_uint16_post_min(uint16_t *dst, const uint16_t *src, int count) {
  _core_uint16_not(dst, src, count);
}

void _core_uint16_band_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_band(dst, srcs, nsrc, count);
}

void _core_uint16_band_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_bor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_bor(dst, srcs, nsrc, count);
}

void _core_uint16_bor_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_bxor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_bxor(dst, srcs, nsrc, count);
}

void _core_uint16_bxor_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_land_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_land(dst, srcs, nsrc, count);
}

void _core_uint16_land_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_lor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_lor(dst, srcs, nsrc, count);
}

void _core_uint16_lor_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_lxor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_lxor(dst, srcs, nsrc, count);
}

void _core_uint16_lxor_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_max_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_max(dst, srcs, nsrc, count);
}

void _core_uint16_max_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_min_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	for (; n < count; n++) {
		uint16_t val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		dst[n] = ~(val);
	}
}

void _core_uint16_min_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
  _core_uint16_not(dst, src, count);
}

void _core_uint16_sum_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_sum(dst, srcs, nsrc, count);
}

void _core_uint16_sum_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}
