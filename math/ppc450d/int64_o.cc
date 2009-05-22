/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/int64_o.cc
 * \brief Optimized math routines for signed 64 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "dcmf_bg_math.h"
#include "Util.h"
#include "ppc450d/internal_o.h"

void _core_int64_sum2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {

  int64_t *dp = (int64_t *) dst;
  const int64_t *s0 = (const int64_t *) srcs[0];
  const int64_t *s1 = (const int64_t *) srcs[1];

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

void _core_int64_sum_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_sum2((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_band2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_bor2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_bxor2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_land2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_lor2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_lxor2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_max2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE int64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_min2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE int64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_prod2(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE int64_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_conv_o(uint64_t *dst, const int64_t *src, int count) {
#define OP(a) ((a)+(0x8000000000000000ULL))

#define TYPE uint64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_not_o(int64_t *dst, const int64_t *src, int count) {
#define OP(a) (~(a))

#define TYPE int64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_conv_not_o(uint64_t *dst, const int64_t *src, int count) {
#define OP(a) (~((a)+(0x8000000000000000ULL)))

#define TYPE uint64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_pre_all_o(uint64_t *dst, const int64_t *src, int count) {
  _core_int64_conv_o(dst, src, count);
}

void _core_int64_post_all_o(int64_t *dst, const uint64_t *src, int count) {
  _core_uint64_conv_o((uint64_t *)dst, src, count);
}

void _core_int64_pre_min_o(uint64_t *dst, const int64_t *src, int count) {
  _core_int64_conv_not_o(dst, src, count);
}

void _core_int64_post_min_o(int64_t *dst, const uint64_t *src, int count) {
  _core_uint64_conv_not_o((uint64_t *)dst, src, count);
}

void _core_int64_band_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_band2((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_bor_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_bor2((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_bxor_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_bxor2((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_land_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_land2((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_lor_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_lor2((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_lxor_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  _core_int64_lxor2((int64_t *)dst, srcs, nsrc, count);
}

void _core_int64_max_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  const int64_t *s0 = srcs[0], *s1 = srcs[1];
  register int n=0;
  register int64_t rbuffer0, rbuffer1, rbuffer2, rbuffer3;
  register int64_t buf00, buf01, buf02, buf03;
  register int64_t buf10, buf11, buf12, buf13;
  register uint64_t shift = 0x8000000000000000ULL;
  for(; n<count-3; n+=4)
    {
      buf00 = s0[n+0]; buf10 = s1[n+0];
      buf01 = s0[n+1]; buf11 = s1[n+1];
      buf02 = s0[n+2]; buf12 = s1[n+2];
      buf03 = s0[n+3]; buf13 = s1[n+3];

      rbuffer0 = MAX(buf00,buf10)+shift;
      rbuffer1 = MAX(buf01,buf11)+shift;
      rbuffer2 = MAX(buf02,buf12)+shift;
      rbuffer3 = MAX(buf03,buf13)+shift;

      dst[n+0] = rbuffer0;
      dst[n+1] = rbuffer1;
      dst[n+2] = rbuffer2;
      dst[n+3] = rbuffer3;
    }
  for(; n<count; n++)
    {
      dst[n] = MAX(s0[n],s1[n])+shift;
    }
}

void _core_int64_max_unmarshall_o(int64_t *dst, const uint64_t *src, int count) {
  _core_uint64_conv_o((uint64_t *)dst, src, count);
}

void _core_int64_min_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
  const int64_t *s0 = srcs[0], *s1 = srcs[1];
  register int n=0;
  register int64_t rbuffer0, rbuffer1, rbuffer2, rbuffer3;
  register int64_t buf00, buf01, buf02, buf03;
  register int64_t buf10, buf11, buf12, buf13;
  register uint64_t shift = 0x8000000000000000ULL;
  for(; n<count-3; n+=4)
    {
      buf00 = s0[n+0]; buf10 = s1[n+0];
      buf01 = s0[n+1]; buf11 = s1[n+1];
      buf02 = s0[n+2]; buf12 = s1[n+2];
      buf03 = s0[n+3]; buf13 = s1[n+3];

      rbuffer0 = ~(MIN(buf00,buf10)+shift);
      rbuffer1 = ~(MIN(buf01,buf11)+shift);
      rbuffer2 = ~(MIN(buf02,buf12)+shift);
      rbuffer3 = ~(MIN(buf03,buf13)+shift);

      dst[n+0] = rbuffer0;
      dst[n+1] = rbuffer1;
      dst[n+2] = rbuffer2;
      dst[n+3] = rbuffer3;
    }
  for(; n<count; n++)
    {
      dst[n] = ~(MIN(s0[n],s1[n])+shift);
    }
}

void _core_int64_min_unmarshall_o(int64_t *dst, const uint64_t *src, int count) {
  _core_uint64_conv_not_o((uint64_t *)dst, src, count);
}
