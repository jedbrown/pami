/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/int32_o.cc
 * \brief Optimized math routines for signed 32 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "dcmf_bg_math.h"
#include "Util.h"
#include "ppc450d/internal_o.h"

void _core_int32_max2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {

  int32_t *dp = (int32_t *)dst;
  const int32_t *s0 = (const int32_t *)srcs[0];
  const int32_t *s1 = (const int32_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"

      "lwz     9,0(%[s1]);"

      "lwz     6,4(%[s0]);"
      "cmpw    5,9;"

      "lwz     10,4(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0:    stw     5,0(%[dp]);"

      "lwz     7,8(%[s0]);"

      "lwz     11,8(%[s1]);"
      "cmpw    6,10;"

      "lwz     8,12(%[s0]);"
      "bge     1f;"

      "mr      6,10;"

"1:    stw     6,4(%[dp]);"
      "cmpw    7,11;"

      "lwz     12,12(%[s1]);"
      "bge     2f;"

      "mr      7,11;"

"2:    stw     7,8(%[dp]);"

      "cmpw    8,12;"

      "bge     3f;"

      "mr      8,12;"

"3:    stw     8,12(%[dp]);"

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

  register int32_t s0_r;
  register int32_t s1_r;
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

void _core_int32_min2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {

  int32_t *dp = (int32_t *)dst;
  const int32_t *s0 = (const int32_t *)srcs[0];
  const int32_t *s1 = (const int32_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"
      "lwz     9,0(%[s1]);"

      "lwz     6,4(%[s0]);"
      "lwz     10,4(%[s1]);"

      "lwz     7,8(%[s0]);"
      "lwz     11,8(%[s1]);"

      "lwz     8,12(%[s0]);"
      "lwz     12,12(%[s1]);"

      "cmpw    9,5;"
      "bge     0f;"
      "mr      5,9;"
"0:    stw     5,0(%[dp]);"

      "cmpw    10,6;"
      "bge     1f;"
      "mr      6,10;"
"1:    stw     6,4(%[dp]);"

      "cmpw    11,7;"
      "bge     2f;"
      "mr      7,11;"
"2:    stw     7,8(%[dp]);"

      "cmpw    12,8;"
      "bge     3f;"
      "mr      8,12;"
"3:    stw     8,12(%[dp]);"

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

  register int32_t s0_r;
  register int32_t s1_r;
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

void _core_int32_min_conv2(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {

  uint32_t *dp = (uint32_t *)dst;
  const int32_t *s0 = (const int32_t *)srcs[0];
  const int32_t *s1 = (const int32_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"

      "lwz     9,0(%[s1]);"

      "lwz     6,4(%[s0]);"
      "cmpw    9,5;"

      "lwz     10,4(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0:    addis   5,5,0x8000;"
      "lwz     7,8(%[s0]);"

      "stw     5,0(%[dp]);"
      "cmpw    10,6;"

      "lwz     11,8(%[s1]);"
      "bge     1f;"

      "mr      6,10;"

"1:    addis   6,6,0x8000;"
      "lwz     8,12(%[s0]);"

      "stw     6,4(%[dp]);"
      "cmpw    11,7;"

      "lwz     12,12(%[s1]);"
      "bge     2f;"

      "mr      7,11;"

"2:    addis   7,7,0x8000;"

      "stw     7,8(%[dp]);"
      "cmpw    12,8;"

      "bge     3f;"

      "mr      8,12;"

"3:    addis   8,8,0x8000;"

      "stw     8,12(%[dp]);"

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

  register int32_t s0_r;
  register int32_t s1_r;
  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    if (s1_r > s0_r) *(dp) = s0_r + 0x80000000;
    else *(dp) = s1_r + 0x80000000;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_int32_prod2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {

	const int32_t *s0 = srcs[0];
	const int32_t *s1 = srcs[1];
	int num;
	int remainder = count;
	if (count <= 2048) {
		num = count >> 2;
		remainder = count - (num << 2);
		register int r0=0;
		register int r1=0;
		register int r2=0;
		register int r3=0;
		register int r4=0;
		register int r5=0;
		register int r6=0;
		register int r7=0;

		while (num--) {
			asm volatile(
				"lwz   %[r0],0(%[s0]);"
				"lwz   %[r4],0(%[s1]);"
				"lwz   %[r1],4(%[s0]);"
				"lwz   %[r5],4(%[s1]);"

				"mullw %[r0],%[r0],%[r4];"
				"stw   %[r0],0(%[dp]);"

				"lwz   %[r2],8(%[s0]);"
				"lwz   %[r6],8(%[s1]);"
				"mullw %[r1],%[r1],%[r5];"
				"stw   %[r1],4(%[dp]);"

				"lwz   %[r3],12(%[s0]);"
				"lwz   %[r7],12(%[s1]);"
				"mullw %[r2],%[r2],%[r6];"
				"stw   %[r2],8(%[dp]);"

				"mullw %[r3],%[r3],%[r7];"
				"stw   %[r3],12(%[dp]);"

				"addi  %[s0],%[s0],16;"
				"addi  %[s1],%[s1],16;"
				"addi  %[dp],%[dp],16;"

				: [s0] "+b" (s0),
				  [s1] "+b" (s1),
				  [dp] "+b" (dst),
				  [r0] "+r" (r0),
				  [r1] "+r" (r1),
				  [r2] "+r" (r2),
				  [r3] "+r" (r3),
				  [r4] "+r" (r4),
				  [r5] "+r" (r5),
				  [r6] "+r" (r6),
				  [r7] "+r" (r7)
				:
				: "memory");
		}
		for (num = 0; num < remainder; ++num) {
			dst[num] = s0[num] * s1[num];
		}
		return;
	}

	int32_t *s2 = dst;
	num = (count - 16) >> 3;
	remainder = (count & 0x07) + 16;
#define OP2(a,b)	asm volatile ("mullw %0, %0, %1" : "=r"(a) : "r"(b))
#include "ppc450d/_optim_uint32_dual_src.x.h"
#undef OP2
	for (num = 0; num < remainder; ++num) {
		s2[num] = s0[num] * s1[num];
	}

	//fprintf (stderr, "<< Core_uint32_prod()\n");
	return;
}


void _core_int32_prod_to_tree2(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {

  uint32_t *dp = (uint32_t *)dst;
  const int32_t *s0 = (const int32_t *)srcs[0];
  const int32_t *s1 = (const int32_t *)srcs[1];

  register int32_t u asm("r5") = count >> 3;

  asm volatile (
"0:      lswi   6,%[s0],28;"
        "lswi  15,%[s1],32;"
	"lwz   14,28(%[s0]);"

	"mullw 15,15,6;"
	"mullw 16,16,7;"
	"mullw 17,17,8;"
	"mullw 18,18,9;"
	"mullw 19,19,10;"
	"mullw 20,20,11;"
	"mullw 21,21,12;"
	"mullw 22,22,14;"

        "addis 15,15,0x8000;"
        "addis 16,16,0x8000;"
        "addis 17,17,0x8000;"
        "addis 18,18,0x8000;"
        "addis 19,19,0x8000;"
        "addis 20,20,0x8000;"
        "addis 21,21,0x8000;"
        "addis 22,22,0x8000;"

	"stswi 15,%[dp],32;"
	"addi  %[s0],%[s0],32;"
	"addi  %[s1],%[s1],32;"
	"addi  %[dp],%[dp],32;"
	"addi 5,5,-1;"
	"cmpwi 5,0;"
	"bne   0b;"
      : [s0] "+b" (s0),
        [s1] "+b" (s1),
        [dp] "+b" (dp)
      : "r" (u)
      : "memory",
        "6",  "7",  "8",  "9",  "10", "11", "12", "14",
        "15", "16", "17", "18", "19", "20", "21", "22"
        );

  register int32_t s0_r;
  register int32_t s1_r;

  u = count & 0x07;
  while ( u-- ) {
    s0_r = *s0;
    s1_r = *s1;
    *dp = (s0_r * s1_r) + 0x80000000;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_int32_sum2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {

	const int32_t *s0 = srcs[0];
	const int32_t *s1 = srcs[1];
	int num;
	int remainder = count;
	if (count <= 2048) {
		num = count >> 2;
		remainder = count - (num << 2);
		register int r0=0;
		register int r1=0;
		register int r2=0;
		register int r3=0;
		register int r4=0;
		register int r5=0;
		register int r6=0;
		register int r7=0;

		while (num--) {
			asm volatile(
				"lwz   %[r0],0(%[s0]);"
				"lwz   %[r4],0(%[s1]);"
				"lwz   %[r1],4(%[s0]);"
				"lwz   %[r5],4(%[s1]);"

				"add   %[r0],%[r0],%[r4];"
				"stw   %[r0],0(%[dp]);"

				"lwz   %[r2],8(%[s0]);"
				"lwz   %[r6],8(%[s1]);"
				"add   %[r1],%[r1],%[r5];"
				"stw   %[r1],4(%[dp]);"

				"lwz   %[r3],12(%[s0]);"
				"lwz   %[r7],12(%[s1]);"
				"add   %[r2],%[r2],%[r6];"
				"stw   %[r2],8(%[dp]);"

				"add   %[r3],%[r3],%[r7];"
				"stw   %[r3],12(%[dp]);"

				"addi  %[s0],%[s0],16;"
				"addi  %[s1],%[s1],16;"
				"addi  %[dp],%[dp],16;"

				: [s0] "+b" (s0),
				  [s1] "+b" (s1),
				  [dp] "+b" (dst),
				  [r0] "+r" (r0),
				  [r1] "+r" (r1),
				  [r2] "+r" (r2),
				  [r3] "+r" (r3),
				  [r4] "+r" (r4),
				  [r5] "+r" (r5),
				  [r6] "+r" (r6),
				  [r7] "+r" (r7)
				:
				: "memory");
		}
		for (num = 0; num < remainder; ++num) {
			dst[num] = s0[num] + s1[num];
		}
		return;
	}

	int32_t *s2 = dst;
	num = (count - 16) >> 3;
	remainder = (count & 0x07) + 16;
#define OP2(a,b)	asm volatile ("add %0, %0, %1" : "=r"(a) : "r"(b))
#include "ppc450d/_optim_uint32_dual_src.x.h"
#undef OP2
	for (num = 0; num < remainder; ++num) {
		s2[num] = s0[num] + s1[num];
	}

	//fprintf (stderr, "<< Core_uint32_sum()\n");
	return;
}

void _core_int32_band4(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
const int32_t *src0 = srcs[0];
const int32_t *src1 = srcs[1];
const int32_t *src2 = srcs[2];
const int32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a&b&c&d)
#define TYPE		int32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_int32_bor4(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
const int32_t *src0 = srcs[0];
const int32_t *src1 = srcs[1];
const int32_t *src2 = srcs[2];
const int32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a|b|c|d)
#define TYPE		int32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_int32_bxor4(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
const int32_t *src0 = srcs[0];
const int32_t *src1 = srcs[1];
const int32_t *src2 = srcs[2];
const int32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a^b^c^d)
#define TYPE		int32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_int32_max4(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
const int32_t *src0 = srcs[0];
const int32_t *src1 = srcs[1];
const int32_t *src2 = srcs[2];
const int32_t *src3 = srcs[3];
#define OP(a,b,c,d)	MAX(MAX(a,b),MAX(c,d))
#define TYPE		int32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_int32_min4(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
const int32_t *src0 = srcs[0];
const int32_t *src1 = srcs[1];
const int32_t *src2 = srcs[2];
const int32_t *src3 = srcs[3];
#define OP(a,b,c,d)	MIN(MIN(a,b),MIN(c,d))
#define TYPE		int32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_int32_prod4(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
const int32_t *src0 = srcs[0];
const int32_t *src1 = srcs[1];
const int32_t *src2 = srcs[2];
const int32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a*b*c*d)
#define TYPE		int32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_int32_sum4(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
const int32_t *src0 = srcs[0];
const int32_t *src1 = srcs[1];
const int32_t *src2 = srcs[2];
const int32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a+b+c+d)
#define TYPE		int32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_int32_conv_o(uint32_t *dst, const int32_t *src, int count) {

  uint32_t *dp = dst;
  const int32_t *sp = src;

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
      "lwz    5,0(%[sp]);"

      "lwz    6,4(%[sp]);"
      "addis  5,5,0x8000;"

      "stw    5,0(%[dp]);"

      "lwz    7,8(%[sp]);"
      "addis  6,6,0x8000;"

      "stw    6,4(%[dp]);"

      "lwz    8,12(%[sp]);"
      "addis  7,7,0x8000;"

      "stw    7,8(%[dp]);"

      "lwz    9,16(%[sp]);"
      "addis  8,8,0x8000;"

      "stw    8,12(%[dp]);"

      "lwz   10,20(%[sp]);"
      "addis  9,9,0x8000;"

      "stw    9,16(%[dp]);"

      "lwz   11,24(%[sp]);"
      "addis 10,10,0x8000;"

      "stw   10,20(%[dp]);"

      "lwz   12,28(%[sp]);"
      "addis 11,11,0x8000;"

      "stw   11,24(%[dp]);"
      "addis 12,12,0x8000;"

      "stw   12,28(%[dp]);"

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
           "lwz   12,0(%[sp]);"
	   "addis  12,12,0x8000;"
	   "stw   12,0(%[dp]);"

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

void _core_int32_max_marshall2(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {

  uint32_t *dp = (uint32_t *)dst;
  const int32_t *s0 = (const int32_t *)srcs[0];
  const int32_t *s1 = (const int32_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"
      "lwz     6,4(%[s0]);"
      "lwz     7,8(%[s0]);"
      "lwz     8,12(%[s0]);"

      "lwz     9,0(%[s1]);"
      "lwz     10,4(%[s1]);"
      "lwz     11,8(%[s1]);"
      "lwz     12,12(%[s1]);"

      "cmpw    5,9;"
      "bge     0f;"
      "mr      5,9;"
"0:    addis   5,5,0x8000;"
      "stw     5,0(%[dp]);"

      "cmpw    6,10;"
      "bge     1f;"
      "mr      6,10;"
"1:    addis   6,6,0x8000;"
      "stw     6,4(%[dp]);"

      "cmpw    7,11;"
      "bge     2f;"
      "mr      7,11;"
"2:    addis   7,7,0x8000;"
      "stw     7,8(%[dp]);"

      "cmpw    8,12;"
      "bge     3f;"
      "mr      8,12;"
"3:    addis   8,8,0x8000;"
      "stw     8,12(%[dp]);"

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

  register int32_t s0_r;
  register int32_t s1_r;
  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    if (s0_r > s1_r) *(dp) = s0_r + 0x80000000;
    else *(dp) = s1_r + 0x80000000;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_int32_pre_all_o(uint32_t *dst, const int32_t *src, int count) {
	_core_int32_conv_o(dst, src, count);
}

void _core_int32_post_all_o(int32_t *dst, const uint32_t *src, int count) {
	_core_uint32_conv_o((uint32_t *)dst, src, count);
}

void _core_int32_post_min_o(int32_t *dst, const uint32_t *src, int count) {
	_core_uint32_conv_not_o((uint32_t *)dst, src, count);
}

void _core_int32_min_unmarshall_o(int32_t *dst, const uint32_t *src, int count) {
	_core_uint32_conv_not_o((uint32_t *)dst, src, count);
}

void _core_int32_sum_marshall2(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_sum2((int32_t *)dst, srcs, nsrc, count);
}

void _core_int32_band2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int32_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_bor2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int32_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_bxor2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int32_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_land2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int32_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_lor2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int32_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_lxor2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int32_t
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_not_o(int32_t *dst, const int32_t *src, int count) {
#define OP(a) (~(a))

#define TYPE int32_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_conv_not_o(uint32_t *dst, const int32_t *src, int count) {
#define OP(a) (~((a)+(0x80000000UL)))

#define TYPE uint32_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int32_int32_maxloc2(int32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
  const int32_int32_t *s0 = srcs[0], *s1 = srcs[1];
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

void _core_int32_int32_minloc2(int32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
  const int32_int32_t *s0 = srcs[0], *s1 = srcs[1];
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

void _core_int32_pre_min_o(uint32_t *dst, const int32_t *src, int count) {
  _core_int32_conv_not_o(dst, src, count);
}

void _core_int32_int32_pre_maxloc_o(uint32_int32_t *dst, const int32_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x80000000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = src[n].a + shift;
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int32_int32_post_maxloc_o(int32_int32_t *dst, const uint32_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x80000000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = src[n].a - shift;
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}

void _core_int32_int32_pre_minloc_o(uint32_int32_t *dst, const int32_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x80000000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(src[n].a + shift);
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int32_int32_post_minloc_o(int32_int32_t *dst, const uint32_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x80000000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(src[n].a - shift);
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}

void _core_int32_band_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_band2(dst, srcs, nsrc, count);
}

void _core_int32_bor_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_bor2(dst, srcs, nsrc, count);
}

void _core_int32_bxor_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_bxor2(dst, srcs, nsrc, count);
}

void _core_int32_land_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_land2(dst, srcs, nsrc, count);
}

void _core_int32_lor_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_lor2(dst, srcs, nsrc, count);
}

void _core_int32_lxor_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
  _core_int32_lxor2(dst, srcs, nsrc, count);
}

void _core_int32_max_unmarshall_o(int32_t *dst, const uint32_t *src, int count) {
  _core_uint32_conv_o((uint32_t *)dst, src, count);
}

void _core_int32_int32_maxloc_marshall2(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
  const int32_int32_t *s0 = srcs[0], *s1 = srcs[1];
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

void _core_int32_int32_maxloc_unmarshall_o(int32_int32_t *dst, const uint32_int32_t *src, int count) {
  _core_int32_int32_post_maxloc_o(dst, src, count);
}

void _core_int32_min_marshall2(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
  const int32_t *s0 = srcs[0], *s1 = srcs[1];
  register int n=0;
  register int32_t rbuffer0, rbuffer1, rbuffer2, rbuffer3;
  register int32_t buf00, buf01, buf02, buf03;
  register int32_t buf10, buf11, buf12, buf13;
  register uint32_t shift = 0x80000000UL;
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

void _core_int32_int32_minloc_marshall2(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
  const int32_int32_t *s0 = srcs[0], *s1 = srcs[1];
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(MIN(s0[n].a,s1[n].a) + shift);
      if(s0[n].a < s1[n].a)
        dst[n].b = s0[n].b;
      else if(s0[n].a > s1[n].a)
        dst[n].b = s1[n].b;
      else
        dst[n].b = MIN(s0[n].b, s1[n].b);

      dst[n].b = ~dst[n].b;
    }
}

void _core_int32_int32_minloc_unmarshall_o(int32_int32_t *dst, const uint32_int32_t *src, int count) {
  _core_int32_int32_post_minloc_o(dst, src, count);
}
