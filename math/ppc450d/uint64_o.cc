/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/uint64_o.cc
 * \brief Optimized math routines for unsigned 64 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "dcmf_bg_math.h"
#include "Util.h"
#include "ppc450d/internal_o.h"

void _core_uint64_band2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint32_band2((uint32_t *)dst, (const uint32_t **)srcs, nsrc, count << 1);
}

void _core_uint64_bor2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint32_bor2((uint32_t *)dst, (const uint32_t **)srcs, nsrc, count << 1);
}

void _core_uint64_bxor2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint32_bxor2((uint32_t *)dst, (const uint32_t **)srcs, nsrc, count << 1);
}

void _core_uint64_land2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {

  uint64_t *dp = (uint64_t *)dst;
  const uint64_t *s0 = (const uint64_t *)srcs[0];
  const uint64_t *s1 = (const uint64_t *)srcs[1];

  int n = count >> 1;
  const uint32_t zero = 0;
  const uint32_t one  = 1;
  while ( n-- ) {
    asm volatile (

	"lwz    5,0(%[s0]);"

	"lwz    6,4(%[s0]);"

	"lwz    7,0(%[s1]);"
	"or.    5,5,6;"

	"lwz    8,4(%[s1]);"
	"crmove 31,2;"

	"stw    %[zero],0(%[dp]);"
	"or.    7,7,8;"

	"lwz    9,8(%[s0]);"
	"crnor  31,31,2;"

	"lwz    10,12(%[s0]);"
	"mfcr   5;"

	"lwz    11,8(%[s1]);"
	"andi.  5,5,1;"

	"stw    5,4(%[dp]);"
	"or.    9,9,10;"

	"crmove 31,2;"
	"lwz    12,12(%[s1]);"

	"or.    11,11,12;"
	"stw    %[zero],8(%[dp]);"

	"crnor  31,31,2;"

	"mfcr   9;"

	"andi.  9,9,1;"

	"stw    9,12(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
	[dp] "b" (dp),
	[zero] "r" (zero),
	[one]  "r" (one)
      : "memory", "5", "6", "7", "8", "9", "10", "11", "12"
    );

    s0 += 2;
    s1 += 2;
    dp += 2;
  }


  if (count & 0x01) {
    (*dp) = (*s0) && (*s1);
  }

  return;
}

void _core_uint64_lor2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {

  uint64_t *dp = (uint64_t *)dst;
  const uint64_t *s0 = (const uint64_t *)srcs[0];
  const uint64_t *s1 = (const uint64_t *)srcs[1];

  int n = count >> 1;
  const uint32_t zero = 0;
  const uint32_t one  = 1;
  while ( n-- ) {
    asm volatile (

	"lwz    5,0(%[s0]);"

	"lwz    6,4(%[s0]);"

	"lwz    7,0(%[s1]);"
	"or.    5,5,6;"

	"lwz    8,4(%[s1]);"
	"crmove 31,2;"

	"stw    %[zero],0(%[dp]);"
	"or.    7,7,8;"

	"lwz    9,8(%[s0]);"
	"crnand 31,31,2;"

	"lwz    10,12(%[s0]);"
	"mfcr   5;"

	"lwz    11,8(%[s1]);"
	"andi.  5,5,1;"

	"stw    5,4(%[dp]);"
	"or.    9,9,10;"

	"crmove 31,2;"
	"lwz    12,12(%[s1]);"

	"or.    11,11,12;"
	"stw    %[zero],8(%[dp]);"

	"crnand 31,31,2;"

	"mfcr   9;"

	"andi.  9,9,1;"

	"stw    9,12(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
	[dp] "b" (dp),
	[zero] "r" (zero),
	[one]  "r" (one)
      : "memory", "5", "6", "7", "8", "9", "10", "11", "12"
    );

    s0 += 2;
    s1 += 2;
    dp += 2;
  }


  if (count & 0x01) {
    (*dp) = (*s0) || (*s1);
  }

  return;
}

void _core_uint64_lxor2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {

  uint64_t *dp = (uint64_t *)dst;
  const uint64_t *s0 = (const uint64_t *)srcs[0];
  const uint64_t *s1 = (const uint64_t *)srcs[1];

  int n = count >> 1;
  const uint32_t zero = 0;
  const uint32_t one  = 1;
  while ( n-- ) {
    asm volatile (

	"lwz    5,0(%[s0]);"

	"lwz    6,4(%[s0]);"

	"lwz    7,0(%[s1]);"
	"or.    5,5,6;"

	"lwz    8,4(%[s1]);"
	"crmove 31,2;"

	"stw    %[zero],0(%[dp]);"
	"or.    7,7,8;"

	"lwz    9,8(%[s0]);"
	"crxor  31,31,2;"

	"lwz    10,12(%[s0]);"
	"mfcr   5;"

	"lwz    11,8(%[s1]);"
	"andi.  5,5,1;"

	"stw    5,4(%[dp]);"
	"or.    9,9,10;"

	"crmove 31,2;"
	"lwz    12,12(%[s1]);"

	"or.    11,11,12;"
	"stw    %[zero],8(%[dp]);"

	"crxor  31,31,2;"

	"mfcr   9;"

	"andi.  9,9,1;"

	"stw    9,12(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
	[dp] "b" (dp),
	[zero] "r" (zero),
	[one]  "r" (one)
      : "memory", "5", "6", "7", "8", "9", "10", "11", "12"
    );

    s0 += 2;
    s1 += 2;
    dp += 2;
  }


  if (count & 0x01) {
    (*dp) = ((*s0) && !(*s1)) || (!(*s0) && (*s1));
  }

  return;
}

void _core_uint64_sum2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {

  uint64_t *dp = (uint64_t *) dst;
  const uint64_t *s0 = (const uint64_t *) srcs[0];
  const uint64_t *s1 = (const uint64_t *) srcs[1];

  int q64 = count >> 1;
  while (q64--)
    {
      asm volatile
        (
          "lwz     5,4(%[s0]);"

	  "lwz     6,4(%[s1]);"

	  "lwz     7,0(%[s0]);"
	  "addc    5,5,6;"

	  "lwz     8,0(%[s1]);"

	  "stw     5,4(%[dp]);"
	  "adde    7,7,8;"

	  "stw     7,0(%[dp]);"

	  "lwz     9,12(%[s0]);"

	  "lwz    10,12(%[s1]);"

	  "lwz    11,8(%[s0]);"
	  "addc    9,9,10;"

	  "lwz    12,8(%[s1]);"

	  "stw     9,12(%[dp]);"
	  "adde   11,11,12;"

	  "stw    11,8(%[dp]);"

	  : // no outputs
	  : [s0] "b" (s0),
	    [s1] "b" (s1),
	    [dp] "b" (dp)
	  : "memory", "5", "6", "7", "8", "9", "10", "11", "12"
        );
      dp += 2;
      s0 += 2;
      s1 += 2;
    }

  if (count & 0x01)
    {
      (*dp) = (*s0) + (*s1);
    }

  return;
}

void _core_uint64_not_o(uint64_t *dst, const uint64_t *src, int count) {
  _core_uint32_not_o((uint32_t *)dst, (const uint32_t *)src, count<<1);
}

void _core_uint64_pre_min_o(uint64_t *dst, const uint64_t *src, int count) {
  _core_uint64_not_o(dst, src, count);
}

void _core_uint64_post_min_o(uint64_t *dst, const uint64_t *src, int count) {
  _core_uint64_not_o(dst, src, count);
}

void _core_uint64_min_unmarshall_o(uint64_t *dst, const uint64_t *src, int count) {
  _core_uint64_not_o(dst, src, count);
}

void _core_uint64_band_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_band2(dst, srcs, nsrc, count);
}

void _core_uint64_bor_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_bor2(dst, srcs, nsrc, count);
}

void _core_uint64_bxor_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_bxor2(dst, srcs, nsrc, count);
}

void _core_uint64_land_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_land2(dst, srcs, nsrc, count);
}

void _core_uint64_lor_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_lor2(dst, srcs, nsrc, count);
}

void _core_uint64_lxor_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_lxor2(dst, srcs, nsrc, count);
}

void _core_uint64_sum_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
  _core_uint64_sum2(dst, srcs, nsrc, count);
}

void _core_uint64_max2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE uint64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_min2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE uint64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_prod2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE uint64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_conv_o(uint64_t *dst, const uint64_t *src, int count) {
#define OP(a) ((a)-(0x8000000000000000ULL))

#define TYPE uint64_t 
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint64_conv_not_o(uint64_t *dst, const uint64_t *src, int count) {
#define OP(a) (~((a)-(0x8000000000000000ULL)))

#define TYPE uint64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}
