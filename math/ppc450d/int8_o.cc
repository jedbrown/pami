/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/int8_o.cc
 * \brief Optimized math routines for signed 8 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "dcmf_bg_math.h"
#include "Util.h"
#include "ppc450d/internal_o.h"

void _core_int8_max2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {

  int8_t *dp = (int8_t *)dst;
  const int8_t *s0 = (const int8_t *)srcs[0];
  const int8_t *s1 = (const int8_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[s0]);"

      "extsb   5,5;"
      "lbz     6,1(%[s0]);"

      "extsb   6,6;"
      "lbz     7,2(%[s0]);"

      "extsb   7,7;"
      "lbz     8,3(%[s0]);"

      "extsb   8,8;"
      "lbz     9,0(%[s1]);"

      "extsb   9,9;"
      "lbz     10,1(%[s1]);"

      "extsb   10,10;"
      "lbz     11,2(%[s1]);"

      "extsb   11,11;"
      "lbz     12,3(%[s1]);"

      "extsb   12,12;"


      "cmpw    5,9;"
      "bge     0f;"
      "mr      5,9;"
"0:    stb     5,0(%[dp]);"

      "cmpw    6,10;"
      "bge     1f;"
      "mr      6,10;"
"1:    stb     6,1(%[dp]);"

      "cmpw    7,11;"
      "bge     2f;"
      "mr      7,11;"
"2:    stb     7,2(%[dp]);"

      "cmpw    8,12;"
      "bge     3f;"
      "mr      8,12;"
"3:    stb     8,3(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    dp += 4;
    s0 += 4;
    s1 += 4;
  }

  register int8_t s0_r;
  register int8_t s1_r;
  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    if (s0_r > s1_r) *(dp) = s0_r;
    else *(dp) = s1_r;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_int8_min2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {

  int8_t *dp = (int8_t *)dst;
  const int8_t *s0 = (const int8_t *)srcs[0];
  const int8_t *s1 = (const int8_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[s0]);"

      "extsb   5,5;"
      "lbz     6,1(%[s0]);"

      "extsb   6,6;"
      "lbz     7,2(%[s0]);"

      "extsb   7,7;"
      "lbz     8,3(%[s0]);"

      "extsb   8,8;"
      "lbz     9,0(%[s1]);"

      "extsb   9,9;"
      "lbz     10,1(%[s1]);"

      "extsb   10,10;"
      "lbz     11,2(%[s1]);"

      "extsb   11,11;"
      "lbz     12,3(%[s1]);"

      "extsb   12,12;"

      "cmpw    9,5;"
      "bge     0f;"
      "mr      5,9;"
"0:    stb     5,0(%[dp]);"

      "cmpw    10,6;"
      "bge     1f;"
      "mr      6,10;"
"1:    stb     6,1(%[dp]);"

      "cmpw    11,7;"
      "bge     2f;"
      "mr      7,11;"
"2:    stb     7,2(%[dp]);"

      "cmpw    12,8;"
      "bge     3f;"
      "mr      8,12;"
"3:    stb     8,3(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    dp += 4;
    s0 += 4;
    s1 += 4;
  }

  register int s0_r;
  register int s1_r;
  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    if (s1_r > s0_r) *(dp) = s0_r;
    else *(dp) = s1_r;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_int8_min_conv2(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {

  uint8_t *dp = (uint8_t *)dst;
  const int8_t *s0 = (const int8_t *)srcs[0];
  const int8_t *s1 = (const int8_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[s0]);"

      "extsb   5,5;"
      "lbz     6,1(%[s0]);"

      "extsb   6,6;"
      "lbz     7,2(%[s0]);"

      "extsb   7,7;"
      "lbz     8,3(%[s0]);"

      "extsb   8,8;"
      "lbz     9,0(%[s1]);"

      "extsb   9,9;"
      "lbz     10,1(%[s1]);"

      "extsb   10,10;"
      "lbz     11,2(%[s1]);"

      "extsb   11,11;"
      "lbz     12,3(%[s1]);"

      "extsb   12,12;"

      "cmpw    9,5;"
      "bge     0f;"
      "mr      5,9;"
"0:    addi    5,5,0x80;"
      "stb     5,0(%[dp]);"

      "cmpw    10,6;"
      "bge     1f;"
      "mr      6,10;"
"1:    addi    6,6,0x80;"
      "stb     6,1(%[dp]);"

      "cmpw    11,7;"
      "bge     2f;"
      "mr      7,11;"
"2:    addi    7,7,0x80;"
      "stb     7,2(%[dp]);"

      "cmpw    12,8;"
      "bge     3f;"
      "mr      8,12;"
"3:    addi    8,8,0x80;"
      "stb     8,3(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    dp += 4;
    s0 += 4;
    s1 += 4;
  }

  register int s0_r;
  register int s1_r;
  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    if (s1_r > s0_r) *(dp) = s0_r + 0x80;
    else *(dp) = s1_r + 0x80;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_int8_prod2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {

  int8_t *dp = (int8_t *)dst;
  const int8_t *s0 = (const int8_t *)srcs[0];
  const int8_t *s1 = (const int8_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (

      "lbz     5,0(%[s0]);"

      "extsb   5,5;"
      "lbz     6,1(%[s0]);"

      "extsb   6,6;"
      "lbz     7,2(%[s0]);"

      "extsb   7,7;"
      "lbz     8,3(%[s0]);"

      "extsb   8,8;"
      "lbz     9,0(%[s1]);"

      "extsb   9,9;"
      "lbz     10,1(%[s1]);"

      "extsb   10,10;"
      "lbz     11,2(%[s1]);"

      "extsb   11,11;"
      "lbz     12,3(%[s1]);"

      "extsb   12,12;"


      "mullw   5,5,9;"

      "mullw   6,6,10;"
      "stb     5,0(%[dp]);"

      "mullw   7,7,11;"
      "stb     6,1(%[dp]);"

      "mullw   8,8,12;"
      "stb     7,2(%[dp]);"

      "stb     8,3(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    dp += 4;
    s0 += 4;
    s1 += 4;
  }

  register int8_t s0_r;
  register int8_t s1_r;


  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    *dp = s0_r * s1_r;

    dp++;
    s0++;
    s1++;
  }

  return;
}

void _core_int8_prod_to_tree2(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {

  uint8_t *dp = (uint8_t *)dst;
  const int8_t *s0 = (const int8_t *)srcs[0];
  const int8_t *s1 = (const int8_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"
      "extsb   8,5;"
      "srawi   7,5,8;"
      "extsb   7,7;"
      "srawi   6,5,16;"
      "extsb   6,6;"
      "srawi   5,5,24;"

      "lwz     9,0(%[s1]);"
      "extsb   12,9;"
      "srawi   11,9,8;"
      "extsb   11,11;"
      "srawi   10,9,16;"
      "extsb   10,10;"
      "srawi   9,9,24;"

      "mullw   5,5,9;"
      "mullw   6,6,10;"
      "mullw   7,7,11;"
      "mullw   8,8,12;"

      "addi    5,5,0x80;"
      "addi    6,6,0x80;"
      "addi    7,7,0x80;"
      "addi    8,8,0x80;"

      "stb     5,0(%[dp]);"
      "stb     6,1(%[dp]);"
      "stb     7,2(%[dp]);"
      "stb     8,3(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    dp += 4;
    s0 += 4;
    s1 += 4;
  }

  register int8_t s0_r;
  register int8_t s1_r;

  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    *dp = (s0_r * s1_r) + 0x80;

    dp++;
    s0++;
    s1++;
  }

  return;
}

void _core_int8_sum2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {

  const int8_t *s0 = (const int8_t *)srcs[0];
  const int8_t *s1 = (const int8_t *)srcs[1];
  int8_t *dp = (int8_t *)dst;

  size_t n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[s0]);"

      "extsb   5,5;"
      "lbz     6,1(%[s0]);"

      "extsb   6,6;"
      "lbz     7,2(%[s0]);"

      "extsb   7,7;"
      "lbz     8,3(%[s0]);"

      "extsb   8,8;"
      "lbz     9,0(%[s1]);"

      "extsb   9,9;"
      "lbz     10,1(%[s1]);"

      "extsb   10,10;"
      "lbz     11,2(%[s1]);"

      "extsb   11,11;"
      "lbz     12,3(%[s1]);"

      "extsb   12,12;"

      "add     5,5,9;"

      "add     6,6,10;"
      "stb     5,0(%[dp]);"

      "add     7,7,11;"
      "stb     6,1(%[dp]);"

      "add     8,8,12;"
      "stb     7,2(%[dp]);"

      "stb     8,3(%[dp]);"

      : [s0] "+b" (s0),
        [s1] "+b" (s1),
        [dp] "+b" (dp)
      : // no inputs
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    s0 += 4;
    s1 += 4;
    dp += 4;
  }

  register int s0_r;
  register int s1_r;

  n = count & 0x07;
  while ( n-- ) {
    s0_r = *(s0);
    s1_r = *(s1);
    *(dp+0) = s0_r + s1_r;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_int8_conv_o(uint8_t *dst, const int8_t *src, int count) {

  uint8_t *dp = dst;
  const int8_t *sp = src;
  int n = count >> 3;

  while ( n-- ) {
    asm volatile (

      "lbz     5,0(%[sp]);"

      "extsb   5,5;"
      "lbz     6,1(%[sp]);"

      "extsb   6,6;"
      "lbz     7,2(%[sp]);"

      "extsb   7,7;"
      "lbz     8,3(%[sp]);"

      "extsb   8,8;"

      "addi   5,5,0x80;"

      "addi   6,6,0x80;"
      "stb    5,0(%[dp]);"

      "addi   7,7,0x80;"
      "stb    6,1(%[dp]);"

      "addi   8,8,0x80;"
      "stb    7,2(%[dp]);"

      "stb    8,3(%[dp]);"


      "lbz     5,4(%[sp]);"

      "extsb   5,5;"
      "lbz     6,5(%[sp]);"

      "extsb   6,6;"
      "lbz     7,6(%[sp]);"

      "extsb   7,7;"
      "lbz     8,7(%[sp]);"

      "extsb   8,8;"

      "addi   5,5,0x80;"

      "addi   6,6,0x80;"
      "stb    5,4(%[dp]);"

      "addi   7,7,0x80;"
      "stb    6,5(%[dp]);"

      "addi   8,8,0x80;"
      "stb    7,6(%[dp]);"

      "stb    8,7(%[dp]);"

      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
    );

    sp += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    asm volatile (
      "lbz   12,0(%[sp]);"
      "extsb 12,12;"
      "addi  12,12,0x80;"
      "stb   12,0(%[dp]);"

      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp)
      : "memory", "12"
    );

    sp++;
    dp++;
  }

  return;
}

void _core_int8_conv_not_o(uint8_t *dst, const int8_t *src, int count) {

  uint8_t *dp = dst;
  const int8_t *sp = src;
  int n = count >> 3;

  while ( n-- ) {
    asm volatile (

      "lbz     5,0(%[sp]);"

      "extsb   5,5;"

      "addi    5,5,0x80;"

      "not     5,5;"
      "lbz     6,1(%[sp]);"

      "stb     5,0(%[dp]);"
      "extsb   6,6;"

      "addi    6,6,0x80;"

      "not     6,6;"
      "lbz     7,2(%[sp]);"

      "stb     6,1(%[dp]);"
      "extsb   7,7;"

      "addi    7,7,0x80;"

      "not     7,7;"
      "lbz     8,3(%[sp]);"

      "stb     7,2(%[dp]);"
      "extsb   8,8;"

      "addi    8,8,0x80;"

      "not     8,8;"
      "lbz     5,4(%[sp]);"

      "stb     8,3(%[dp]);"
      "extsb   5,5;"

      "addi    5,5,0x80;"

      "not     5,5;"
      "lbz     6,5(%[sp]);"

      "stb     5,4(%[dp]);"
      "extsb   6,6;"

      "addi    6,6,0x80;"

      "not     6,6;"
      "lbz     7,6(%[sp]);"

      "stb     6,5(%[dp]);"
      "extsb   7,7;"

      "addi    7,7,0x80;"

      "not     7,7;"
      "lbz     8,7(%[sp]);"

      "stb     7,6(%[dp]);"
      "extsb   8,8;"

      "addi    8,8,0x80;"

      "not     8,8;"

      "stb     8,7(%[dp]);"


      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
    );

    sp += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    asm volatile (
      "lbz   12,0(%[sp]);"
      "extsb 12,12;"
      "addi  12,12,0x80;"
      "not   12,12;"
      "stb   12,0(%[dp]);"

      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp)
      : "memory", "12"
    );

    sp++;
    dp++;
  }

  return;
}

void _core_int8_pack_o(uint8_t *dst, const int8_t *src, int count) {
  _core_uint8_pack_o(dst, (const uint8_t *)src, count);
}

void _core_int8_unpack_o(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_unpack_o((uint8_t *)dst, src, count);
}

void _core_int8_pre_all_o(uint8_t *dst, const int8_t *src, int count) {
  _core_int8_conv_o(dst, src, count);
}

void _core_int8_post_all_o(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_conv_o((uint8_t *)dst, src, count);
}

void _core_int8_pre_min_o(uint8_t *dst, const int8_t *src, int count) {
  _core_int8_conv_not_o(dst, src, count);
}

void _core_int8_post_min_o(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_conv_not_o((uint8_t *)dst, src, count);
}

void _core_int8_max_unmarshall_o(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_conv_o((uint8_t *)dst, src, count);
}

void _core_int8_min_unmarshall_o(int8_t *dst, const uint8_t *src, int count) {
  _core_uint8_conv_not_o((uint8_t *)dst, src, count);
}

void _core_int8_sum_marshall2(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_sum2((int8_t *)dst, srcs, nsrc, count);
}

void _core_int8_band2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int8_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_bor2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int8_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_bxor2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int8_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_land2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int8_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_lor2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int8_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_lxor2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int8_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_not_o(int8_t *dst, const int8_t *src, int count) {
#define OP(a) (~(a))

#define TYPE int8_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_band_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_band2(dst, srcs, nsrc, count);
}

void _core_int8_bor_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_bor2(dst, srcs, nsrc, count);
}

void _core_int8_bxor_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_bxor2(dst, srcs, nsrc, count);
}

void _core_int8_land_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_land2(dst, srcs, nsrc, count);
}

void _core_int8_lor_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_lor2(dst, srcs, nsrc, count);
}

void _core_int8_lxor_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
  _core_int8_lxor2(dst, srcs, nsrc, count);
}

void _core_int8_max_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
  const int8_t *s0 = srcs[0], *s1 = srcs[1];
  register int n=0;
  register int8_t rbuffer0, rbuffer1, rbuffer2, rbuffer3;
  register int8_t buf00, buf01, buf02, buf03;
  register int8_t buf10, buf11, buf12, buf13;
  register uint8_t shift = 0x80;
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

void _core_int8_min_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
  const int8_t *s0 = srcs[0], *s1 = srcs[1];
  register int n=0;
  register int8_t rbuffer0, rbuffer1, rbuffer2, rbuffer3;
  register int8_t buf00, buf01, buf02, buf03;
  register int8_t buf10, buf11, buf12, buf13;
  register uint8_t shift = 0x80;
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
