/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint32.cc
 * \brief Default C math routines for 32 bit unsigned integer operations.
 */

#include "math_bg_math.h"
#include "internal.h"


void _core_uint32_band(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_bor(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_bxor(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_land(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_lor(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_lxor(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_max(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_min(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_prod(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_sum(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_pack(uint32_t *dst, const uint32_t *src, int count) {
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

void _core_uint32_unpack(uint32_t *dst, const uint32_t *src, int count) {
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

void _core_uint32_conv(uint32_t *dst, const uint32_t *src, int count) {
#define OP(a) ((a)-(0x80000000UL))

#define TYPE int32_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_not(uint32_t *dst, const uint32_t *src, int count) {
#define OP(a) (~(a))

#define TYPE uint32_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_conv_not(uint32_t *dst, const uint32_t *src, int count) {
#define OP(a) (~((a)-(0x80000000UL)))

#define TYPE int32_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_pre_all(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _core_uint32_post_all(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _core_uint32_pre_min(uint32_t *dst, const uint32_t *src, int count) {
  _core_uint32_not(dst, src, count);
}

void _core_uint32_post_min(uint32_t *dst, const uint32_t *src, int count) {
  _core_uint32_not(dst, src, count);
}

void _core_uint32_band_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
  _core_uint32_band(dst, srcs, nsrc, count);
}

void _core_uint32_band_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _core_uint32_bor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
  _core_uint32_bor(dst, srcs, nsrc, count);
}

void _core_uint32_bor_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _core_uint32_bxor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
  _core_uint32_bxor(dst, srcs, nsrc, count);
}

void _core_uint32_bxor_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _core_uint32_land_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
  _core_uint32_land(dst, srcs, nsrc, count);
}

void _core_uint32_land_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _core_uint32_lor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
  _core_uint32_lor(dst, srcs, nsrc, count);
}

void _core_uint32_lor_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _core_uint32_lxor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
  _core_uint32_lxor(dst, srcs, nsrc, count);
}

void _core_uint32_lxor_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _core_uint32_max_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
  _core_uint32_max(dst, srcs, nsrc, count);
}

void _core_uint32_max_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _core_uint32_min_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	for (; n < count; n++) {
		uint32_t val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		dst[n] = ~(val);
	}
}

void _core_uint32_min_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
  _core_uint32_not(dst, src, count);
}

void _core_uint32_sum_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
  _core_uint32_sum(dst, srcs, nsrc, count);
}

void _core_uint32_sum_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}
