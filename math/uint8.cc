/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint8.cc
 * \brief Default C math routines for 8 bit unsigned integer operations.
 */

#include "math_bg_math.h"
#include "internal.h"


void _core_uint8_band(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_bor(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_bxor(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_land(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_lor(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_lxor(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_max(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_min(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_prod(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_sum(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}


/**
 * \brief Boolean bitwise pack of an input array
 * \todo Determine best C implementation
 */
void _core_uint8_pack(uint8_t *dst, const uint8_t *src, int count) {
  int n = count >> 3;
  int i = 0;
  int j = 0;
  while ( n-- ) {
    dst[i] = 0;
    if (src[j++]) dst[i] |= 0x080;
    if (src[j++]) dst[i] |= 0x040;
    if (src[j++]) dst[i] |= 0x020;
    if (src[j++]) dst[i] |= 0x010;
    if (src[j++]) dst[i] |= 0x008;
    if (src[j++]) dst[i] |= 0x004;
    if (src[j++]) dst[i] |= 0x002;
    if (src[j++]) dst[i] |= 0x001;
    i++;
  }

  n = count & 0x07;
  dst[i] = 0;
  if ( n-- && src[j++]) dst[i] |= 0x080;
  if ( n-- && src[j++]) dst[i] |= 0x040;
  if ( n-- && src[j++]) dst[i] |= 0x020;
  if ( n-- && src[j++]) dst[i] |= 0x010;
  if ( n-- && src[j++]) dst[i] |= 0x008;
  if ( n-- && src[j++]) dst[i] |= 0x004;
  if ( n-- && src[j++]) dst[i] |= 0x002;
#if 0
  register unsigned char result;

  unsigned n = count >> 3;
  while ( n-- )
  {
    result =  (src[0]!=0)<<7;
    result |= (src[1]!=0)<<6;
    result |= (src[2]!=0)<<5;
    result |= (src[3]!=0)<<4;
    result |= (src[4]!=0)<<3;
    result |= (src[5]!=0)<<2;
    result |= (src[6]!=0)<<1;
    result |= (src[7]!=0);

    dst[0] = result;

    dst += 1;
    src += 8;
  }

  if ((n = count & 0x07) > 0)
  {
    result = 0;
    unsigned i;
    for (i = 0; i < n; i++) result |= (src[i]!=0)<<(7-i);
    dst[0] = result;
  }
#endif
}

/**
 * \brief Boolean bitwise unpack of an input array
 * \todo Determine best C implementation
 */
void _core_uint8_unpack(uint8_t *dst, const uint8_t *src, int count) {
  int n = count >> 3;
  int i = 0;
  int j = 0;
  while ( n-- ) {
    dst[j++] = src[i] & 0x080;
    dst[j++] = src[i] & 0x040;
    dst[j++] = src[i] & 0x020;
    dst[j++] = src[i] & 0x010;
    dst[j++] = src[i] & 0x008;
    dst[j++] = src[i] & 0x004;
    dst[j++] = src[i] & 0x002;
    dst[j++] = src[i] & 0x001;
    i++;
  }

  n = count & 0x07;
  if ( n-- ) dst[j++] = src[i] & 0x080;
  if ( n-- ) dst[j++] = src[i] & 0x040;
  if ( n-- ) dst[j++] = src[i] & 0x020;
  if ( n-- ) dst[j++] = src[i] & 0x010;
  if ( n-- ) dst[j++] = src[i] & 0x008;
  if ( n-- ) dst[j++] = src[i] & 0x004;
  if ( n-- ) dst[j++] = src[i] & 0x002;
#if 0
  unsigned n = count >> 3;
  while ( n-- )
  {
    dst[0] =  (src[0]&0x080)>>7;
    dst[1] =  (src[0]&0x040)>>6;
    dst[2] =  (src[0]&0x020)>>5;
    dst[3] =  (src[0]&0x010)>>4;
    dst[4] =  (src[0]&0x008)>>3;
    dst[5] =  (src[0]&0x004)>>2;
    dst[6] =  (src[0]&0x002)>>1;
    dst[7] =  (src[0]&0x001);

    dst += 8;
    src += 1;
  }

  if ((n = count & 0x07) > 0)
  {
    unsigned i;
    for (i = 0; i < n; i++) dst[i] = ((src[0]&(1<<(7-i)))!=0);
  }
#endif
}

void _core_uint8_conv(uint8_t *dst, const uint8_t *src, int count) {
#define OP(a) ((a)-(0x80))

#define TYPE int8_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_not(uint8_t *dst, const uint8_t *src, int count) {
#define OP(a) (~(a))

#define TYPE uint8_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_conv_not(uint8_t *dst, const uint8_t *src, int count) {
#define OP(a) (~((a)-(0x80)))

#define TYPE int8_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_pre_all(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_post_all(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_pre_min(uint8_t *dst, const uint8_t *src, int count) {
  _core_uint8_not(dst, src, count);
}

void _core_uint8_post_min(uint8_t *dst, const uint8_t *src, int count) {
  _core_uint8_not(dst, src, count);
}

void _core_uint8_band_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_band(dst, srcs, nsrc, count);
}

void _core_uint8_band_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_bor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_bor(dst, srcs, nsrc, count);
}

void _core_uint8_bor_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_bxor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_bxor(dst, srcs, nsrc, count);
}

void _core_uint8_bxor_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_land_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_land(dst, srcs, nsrc, count);
}

void _core_uint8_land_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_lor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_lor(dst, srcs, nsrc, count);
}

void _core_uint8_lor_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_lxor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_lxor(dst, srcs, nsrc, count);
}

void _core_uint8_lxor_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_max_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_max(dst, srcs, nsrc, count);
}

void _core_uint8_max_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_min_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	register int n = 0, o;
	for (; n < count; n++) {
		uint8_t val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		dst[n] = ~(val);
	}
}

void _core_uint8_min_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
  _core_uint8_not(dst, src, count);
}

void _core_uint8_sum_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_sum(dst, srcs, nsrc, count);
}

void _core_uint8_sum_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}
