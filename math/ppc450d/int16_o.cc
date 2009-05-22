/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/int16_o.cc
 * \brief Optimized math routines for signed 16 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "dcmf_bg_math.h"
#include "Util.h"
#include "ppc450d/internal_o.h"


void _core_int16_max2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {

  int16_t *dp = (int16_t *)dst;
  const int16_t *s0 = (const int16_t *)srcs[0];
  const int16_t *s1 = (const int16_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lha     5,0(%[s0]);"

      "lha     9,0(%[s1]);"

      "lha     6,2(%[s0]);"
      "cmpw    5,9;"

      "lha     10,2(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0:    sth     5,0(%[dp]);"


      "lha     7,4(%[s0]);"
      "cmpw    6,10;"

      "lha     11,4(%[s1]);"
      "bge     1f;"

      "mr      6,10;"

"1:    sth     6,2(%[dp]);"

      "lha     8,6(%[s0]);"
      "cmpw    7,11;"

      "lha     12,6(%[s1]);"
      "bge     2f;"

      "mr      7,11;"

"2:    sth     7,4(%[dp]);"

      "cmpw    8,12;"

      "bge     3f;"

      "mr      8,12;"

"3:    sth     8,6(%[dp]);"

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

  register int16_t s0_r;
  register int16_t s1_r;
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

void _core_int16_min2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {

  int16_t *dp = (int16_t *)dst;
  const int16_t *s0 = (const int16_t *)srcs[0];
  const int16_t *s1 = (const int16_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lha     5,0(%[s0]);"

      "lha     9,0(%[s1]);"

      "lha     6,2(%[s0]);"
      "cmpw    9,5;"

      "lha     10,2(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0:    sth     5,0(%[dp]);"

      "lha     7,4(%[s0]);"
      "cmpw    10,6;"

      "lha     11,4(%[s1]);"
      "bge     1f;"

      "mr      6,10;"

"1:    sth     6,2(%[dp]);"

      "lha     8,6(%[s0]);"
      "cmpw    11,7;"

      "lha     12,6(%[s1]);"
      "bge     2f;"

      "mr      7,11;"

"2:    sth     7,4(%[dp]);"

      "cmpw    12,8;"

      "bge     3f;"

      "mr      8,12;"

"3:    sth     8,6(%[dp]);"

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

void _core_int16_min_conv2(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {

  uint16_t *dp = (uint16_t *)dst;
  const int16_t *s0 = (const int16_t *)srcs[0];
  const int16_t *s1 = (const int16_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "addis  14,0,0;"
      "lha     5,0(%[s0]);"

      "ori    14,14,0x8000;"
      "lha     9,0(%[s1]);"

      "lha     6,2(%[s0]);"
      "cmpw    9,5;"

      "lha     10,2(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0:    add   5,5,14;"

      "sth     5,0(%[dp]);"
      "cmpw    10,6;"


      "lha     7,4(%[s0]);"
      "bge     1f;"

      "mr      6,10;"

"1:    add   6,6,14;"
      "lha     11,4(%[s1]);"

      "sth     6,2(%[dp]);"
      "cmpw    11,7;"

      "lha     8,6(%[s0]);"
      "bge     2f;"

      "mr      7,11;"

"2:    add   7,7,14;"
      "lha     12,6(%[s1]);"

      "sth     7,4(%[dp]);"
      "cmpw    12,8;"

      "bge     3f;"

      "mr      8,12;"

"3:    add   8,8,14;"

      "sth     8,6(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12", "14"
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
    if (s1_r > s0_r) *(dp) = s0_r + 0x08000;
    else *(dp) = s1_r + 0x08000;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_int16_prod2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {

  int16_t *dp = (int16_t *)dst;
  const int16_t *s0 = (const int16_t *)srcs[0];
  const int16_t *s1 = (const int16_t *)srcs[1];

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"

      "lwz     6,0(%[s1]);"

      "mulhhw  7,5,6;"
      "lwz     8,4(%[s0]);"

      "mullhw  6,5,6;"
      "sth     7,0(%[dp]);"

      "lwz     9,4(%[s1]);"

      "sth     6,2(%[dp]);"
      "mulhhw  10,8,9;"

      "mullhw  9,8,9;"
      "sth     10,4(%[dp]);"

      "sth     9,6(%[dp]);"



      "lwz     5,8(%[s0]);"

      "lwz     6,8(%[s1]);"

      "mulhhw  7,5,6;"
      "lwz     8,12(%[s0]);"

      "mullhw  6,5,6;"
      "sth     7,8(%[dp]);"

      "lwz     9,12(%[s1]);"

      "sth     6,10(%[dp]);"
      "mulhhw  10,8,9;"

      "mullhw  9,8,9;"
      "sth     10,12(%[dp]);"

      "sth     9,14(%[dp]);"






//       "lwz     5,0(%[s0]);"
//       "lwz     6,0(%[s1]);"
//       "mulhhw  7,5,6;"
//       "mullhw  6,5,6;"
//       "sth     7,0(%[dp]);"
//       "sth     6,2(%[dp]);"
//
//       "lwz     8,4(%[s0]);"
//       "lwz     9,4(%[s1]);"
//       "mulhhw  10,8,9;"
//       "mullhw  9,8,9;"
//       "sth     10,4(%[dp]);"
//       "sth     9,6(%[dp]);"
//
//       "lwz     5,8(%[s0]);"
//       "lwz     6,8(%[s1]);"
//       "mulhhw  7,5,6;"
//       "mullhw  6,5,6;"
//       "sth     7,8(%[dp]);"
//       "sth     6,10(%[dp]);"
//
//       "lwz     8,12(%[s0]);"
//       "lwz     9,12(%[s1]);"
//       "mulhhw  10,8,9;"
//       "mullhw  9,8,9;"
//       "sth     10,12(%[dp]);"
//       "sth     9,14(%[dp]);"


      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    dp += 8;
    s0 += 8;
    s1 += 8;
  }

  register int16_t s0_r;
  register int16_t s1_r;


  n = count & 0x07;
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

void _core_int16_prod_conv2(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {

  uint16_t *dp = (uint16_t *)dst;
  const int16_t *s0 = (const int16_t *)srcs[0];
  const int16_t *s1 = (const int16_t *)srcs[1];

  register uint32_t add = 0x8000;
  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"
      "mr      7,%[add];"

      "lwz     6,0(%[s1]);"
      "mr      8,%[add];"

      "machhwu 7,5,6;"

      "maclhwu 8,5,6;"
      "sth     7,0(%[dp]);"

      "sth     8,2(%[dp]);"


      "lwz     5,4(%[s0]);"
      "mr      7,%[add];"

      "lwz     6,4(%[s1]);"
      "mr      8,%[add];"

      "machhwu 7,5,6;"

      "maclhwu 8,5,6;"
      "sth     7,4(%[dp]);"

      "sth     8,6(%[dp]);"


      "lwz     5,8(%[s0]);"
      "mr      7,%[add];"

      "lwz     6,8(%[s1]);"
      "mr      8,%[add];"

      "machhwu 7,5,6;"

      "maclhwu 8,5,6;"
      "sth     7,8(%[dp]);"

      "sth     8,10(%[dp]);"


      "lwz     5,12(%[s0]);"
      "mr      7,%[add];"

      "lwz     6,12(%[s1]);"
      "mr      8,%[add];"

      "machhwu 7,5,6;"

      "maclhwu 8,5,6;"
      "sth     7,12(%[dp]);"

      "sth     8,14(%[dp]);"



    /*
      "addis  9,0,0;"
      "ori    9,9,0x8000;"

      "lwz     5,0(%[s0]);"
      "lwz     6,0(%[s1]);"
      "mulhhw  7,5,6;"
      "mullhw  8,5,6;"
      "add     7,7,9;"
      "add     8,8,9;"
      "sth     7,0(%[dp]);"
      "sth     8,2(%[dp]);"

      "lwz     5,4(%[s0]);"
      "lwz     6,4(%[s1]);"
      "mulhhw  7,5,6;"
      "mullhw  8,5,6;"
      "add     7,7,9;"
      "add     8,8,9;"
      "sth     7,4(%[dp]);"
      "sth     8,6(%[dp]);"

      "lwz     5,8(%[s0]);"
      "lwz     6,8(%[s1]);"
      "mulhhw  7,5,6;"
      "mullhw  8,5,6;"
      "add     7,7,9;"
      "add     8,8,9;"
      "sth     7,8(%[dp]);"
      "sth     8,10(%[dp]);"

      "lwz     5,12(%[s0]);"
      "lwz     6,12(%[s1]);"
      "mulhhw  7,5,6;"
      "mullhw  8,5,6;"
      "add     7,7,9;"
      "add     8,8,9;"
      "sth     7,12(%[dp]);"
      "sth     8,14(%[dp]);"*/

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp),
	[add] "r" (add)
      : "memory",
        "5",  "6",  "7",  "8",  "9"
        );
    dp += 8;
    s0 += 8;
    s1 += 8;
  }

  register int16_t s0_r;
  register int16_t s1_r;


  n = count & 0x07;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    *dp = (s0_r * s1_r) + 0x08000;

    dp++;
    s0++;
    s1++;
  }

  return;
}

void _core_int16_sum2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {

  const int16_t *s0 = (const int16_t *)srcs[0];
  const int16_t *s1 = (const int16_t *)srcs[1];
  int16_t *dp = (int16_t *)dst;

  size_t n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lha     5,0(%[s0]);"

      "lha     9,0(%[s1]);"

      "add     5,5,9;"
      "lha     6,2(%[s0]);"

      "lha     10,2(%[s1]);"

      "sth     5,0(%[dp]);"
      "add     6,6,10;"

      "lha     7,4(%[s0]);"

      "lha     11,4(%[s1]);"

      "sth     6,2(%[dp]);"
      "add     7,7,11;"

      "lha     8,6(%[s0]);"

      "lha     12,6(%[s1]);"

      "sth     7,4(%[dp]);"
      "add     8,8,12;"

      "sth     8,6(%[dp]);"

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

  register int s0_r asm("r5");
  register int s1_r asm("r6");

  n = count & 0x03;
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


void _core_int16_conv_o(uint16_t *dst, const int16_t *src, int count) {

  uint16_t *dp = dst;
  const int16_t *sp = src;
  int n = count >> 3;

  register uint32_t add = 0x8000;
  while ( n-- ) {
    asm volatile (
      "lha    5,0(%[sp]);"

      "add    5,5,%[add];"
      "lha    6,2(%[sp]);"

      "add    6,6,%[add];"
      "lha    7,4(%[sp]);"

      "add    7,7,%[add];"
      "lha    8,6(%[sp]);"

      "add    8,8,%[add];"
      "lha    9,8(%[sp]);"

      "add    9,9,%[add];"
      "lha   10,10(%[sp]);"

      "add   10,10,%[add];"
      "lha   11,12(%[sp]);"

      "add   11,11,%[add];"
      "lha   12,14(%[sp]);"

      "add   12,12,%[add];"
      "sth    5,0(%[dp]);"

      "sth    6,2(%[dp]);"

      "sth    7,4(%[dp]);"

      "sth    8,6(%[dp]);"

      "sth    9,8(%[dp]);"

      "sth   10,10(%[dp]);"

      "sth   11,12(%[dp]);"

      "sth   12,14(%[dp]);"

      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp),
	[add] "r" (add)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
    );

    sp += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    asm volatile (
      "lha   12,0(%[sp]);"
      "add   12,12,%[add];"
      "sth   12,0(%[dp]);"

      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp),
	[add] "r" (add)
      : "memory", "12"
    );

    sp++;
    dp++;
  }

  return;
}

void _core_int16_conv_not_o(uint16_t *dst, const int16_t *src, int count) {

  uint16_t *dp = dst;
  const int16_t *sp = src;
  int n = count >> 3;

  register uint32_t add = 0x8000;
  while ( n-- ) {
    asm volatile (
      "lha    5,0(%[sp]);"

      "add    5,5,%[add];"
      "lha    6,2(%[sp]);"

      "not    5,5;"
      "lha    7,4(%[sp]);"

      "sth    5,0(%[dp]);"
      "add    6,6,%[add];"

      "not    6,6;"
      "lha    8,6(%[sp]);"

      "sth    6,2(%[dp]);"
      "add    7,7,%[add];"

      "not    7,7;"
      "lha    9,8(%[sp]);"

      "sth    7,4(%[dp]);"
      "add    8,8,%[add];"

      "not    8,8;"
      "lha   10,10(%[sp]);"

      "sth    8,6(%[dp]);"
      "add    9,9,%[add];"

      "not    9,9;"
      "lha   11,12(%[sp]);"

      "sth    9,8(%[dp]);"
      "add   10,10,%[add];"

      "not   10,10;"
      "lha   12,14(%[sp]);"

      "sth   10,10(%[dp]);"
      "add   11,11,%[add];"

      "not   11,11;"

      "sth   11,12(%[dp]);"
      "add   12,12,%[add];"

      "not   12,12;"

      "sth   12,14(%[dp]);"

      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp),
	[add] "r" (add)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
    );

    sp += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    asm volatile (
      "lha   12,0(%[sp]);"
      "add   12,12,%[add];"
      "not   12,12;"
      "sth   12,0(%[dp]);"

      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp),
	[add] "r" (add)
      : "memory", "12"
    );

    sp++;
    dp++;
  }

  return;
}

void _core_int16_max_marshall2(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {

  uint16_t *dp = (uint16_t *)dst;
  const int16_t *s0 = (const int16_t *)srcs[0];
  const int16_t *s1 = (const int16_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "addis  14,0,0;"
      "lha     5,0(%[s0]);"

      "ori    14,14,0x8000;"
      "lha     9,0(%[s1]);"

      "lha     6,2(%[s0]);"
      "cmpw    5,9;"

      "lha     10,2(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0:    add   5,5,14;"
      "lha     7,4(%[s0]);"

      "sth     5,0(%[dp]);"
      "cmpw    6,10;"

      "lha     11,4(%[s1]);"
      "bge     1f;"

      "mr      6,10;"

"1:    add   6,6,14;"
      "lha     8,6(%[s0]);"

      "sth     6,2(%[dp]);"
      "cmpw    7,11;"

      "lha     12,6(%[s1]);"
      "bge     2f;"

      "mr      7,11;"

"2:    add   7,7,14;"

      "sth     7,4(%[dp]);"
      "cmpw    8,12;"

      "bge     3f;"

      "mr      8,12;"

"3:    add   8,8,14;"

      "sth     8,6(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12", "14"
        );
    dp += 4;
    s0 += 4;
    s1 += 4;
  }

  register int16_t s0_r;
  register int16_t s1_r;
  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    if (s0_r > s1_r) *(dp) = s0_r + 0x08000;
    else *(dp) = s1_r + 0x08000;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_int16_pre_all_o(uint16_t *dst, const int16_t *src, int count) {
	_core_int16_conv_o(dst, src, count); 
}

void _core_int16_post_all_o(int16_t *dst, const uint16_t *src, int count) {
	_core_uint16_conv_o((uint16_t *)dst, src, count); 
}

void _core_int16_pre_min_o(uint16_t *dst, const int16_t *src, int count) {
	_core_int16_conv_not_o(dst, src, count); 
}

void _core_int16_post_min_o(int16_t *dst, const uint16_t *src, int count) {
	_core_uint16_conv_not_o((uint16_t *)dst, src, count); 
}

void _core_int16_max_unmarshall_o(int16_t *dst, const uint16_t *src, int count) {
	_core_uint16_conv_o((uint16_t *)dst, src, count);
}

void _core_int16_min_unmarshall_o(int16_t *dst, const uint16_t *src, int count) {
	_core_uint16_conv_not_o((uint16_t *)dst, src, count);
}

void _core_int16_sum_marshall2(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_sum2((int16_t *)dst, srcs, nsrc, count);
}

void _core_int16_band2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int16_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_bor2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int16_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_bxor2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int16_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_land2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int16_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_lor2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int16_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_lxor2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int16_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_not_o(int16_t *dst, const int16_t *src, int count) {
#define OP(a) (~(a))

#define TYPE int16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_int32_maxloc2(int16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
  const int16_int32_t *s0 = srcs[0], *s1 = srcs[1];
  register int n = 0;
  for (n = 0; n < count; n++)
    {
      if(s0[n].a > s1[n].a)
        dst[n].b = s0[n].b;
      else if(s0[n].a < s1[n].a)
        dst[n].b = s1[n].b;
      else
        dst[n].b = MIN(s0[n].b, s1[n].b);
      dst[n].a = MAX(s0[n].a,s1[n].a);
    }
}

void _core_int16_int32_minloc2(int16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
  const int16_int32_t *s0 = srcs[0], *s1 = srcs[1];
  register int n = 0;
  for (n = 0; n < count; n++)
    {
      if(s0[n].a < s1[n].a)
        dst[n].b = s0[n].b;
      else if(s0[n].a > s1[n].a)
        dst[n].b = s1[n].b;
      else
        dst[n].b = MIN(s0[n].b, s1[n].b);
      dst[n].a = MIN(s0[n].a,s1[n].a);
    }
}

void _core_int16_int32_pre_maxloc_o(uint16_int32_t *dst, const int16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = src[n].a + shift;
      dst[n].z = 0;
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int16_int32_post_maxloc_o(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = src[n].a - shift;
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}

void _core_int16_int32_pre_minloc_o(uint16_int32_t *dst, const int16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(src[n].a + shift);
      dst[n].z = 0;
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int16_int32_post_minloc_o(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(src[n].a - shift);
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}

void _core_int16_band_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_band2(dst, srcs, nsrc, count);
}

void _core_int16_bor_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_bor2(dst, srcs, nsrc, count);
}

void _core_int16_bxor_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_bxor2(dst, srcs, nsrc, count);
}

void _core_int16_land_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_land2(dst, srcs, nsrc, count);
}

void _core_int16_lor_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_lor2(dst, srcs, nsrc, count);
}

void _core_int16_lxor_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
  _core_int16_lxor2(dst, srcs, nsrc, count);
}

void _core_int16_int32_maxloc_marshall2(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
  const int16_int32_t *s0 = srcs[0], *s1 = srcs[1];
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = MAX(s0[n].a,s1[n].a) + shift;
      if(s0[n].a > s1[n].a)
        dst[n].b = s0[n].b;
      else if(s0[n].a < s1[n].a)
        dst[n].b = s1[n].b;
      else
        dst[n].b = MIN(s0[n].b, s1[n].b);
      dst[n].b = ~dst[n].b;
    }

}

void _core_int16_int32_maxloc_unmarshall_o(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  _core_int16_int32_post_maxloc_o(dst, src, count);
}

void _core_int16_min_marshall2(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
  const int16_t *s0 = srcs[0], *s1 = srcs[1];
  register int n=0;
  register int16_t rbuffer0, rbuffer1, rbuffer2, rbuffer3;
  register int16_t buf00, buf01, buf02, buf03;
  register int16_t buf10, buf11, buf12, buf13;
  register uint16_t shift = 0x8000;
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

void _core_int16_int32_minloc_marshall2(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
  const int16_int32_t *s0 = srcs[0], *s1 = srcs[1];
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(MIN(s0[n].a,s1[n].a) + shift);
      if(s0[n].a < s1[n].a)
        dst[n].b = s1[n].b;
      else if(s0[n].a > s1[n].a)
        dst[n].b = s1[n].b;
      else
        dst[n].b = MIN(s0[n].b, s1[n].b);

      dst[n].b = ~dst[n].b;
    }
}

void _core_int16_int32_minloc_unmarshall_o(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  _core_int16_int32_post_minloc_o(dst, src, count);
}
