/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/uint16_o.cc
 * \brief Optimized math routines for unsigned 16 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "dcmf_bg_math.h"
#include "Util.h"
#include "ppc450d/internal_o.h"


void _core_uint16_band2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  const uint16_t *s[2] = {srcs[0],srcs[1]};
  uint16_t *dp = (uint16_t *)dst;

  if (count & 0x01) {
    (*dp) = (*s[0]) & (*s[1]);

    s[0]++;
    s[1]++;
    dp++;
  }
  int num = count >> 1;
  if (num) {
    _core_uint32_band2((uint32_t *)dp, (const uint32_t **)s, 2, num);
  }
  return;
}

void _core_uint16_bor2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  const uint16_t *s[2] = {srcs[0],srcs[1]};
  uint16_t *dp = (uint16_t *)dst;

  if (count & 0x01) {
    (*dp) = (*s[0]) | (*s[1]);

    s[0]++;
    s[1]++;
    dp++;
  }
  int num = count >> 1;
  if (num) {
    _core_uint32_bor2((uint32_t *)dp, (const uint32_t **)s, 2, num);
  }
  return;
}

void _core_uint16_bxor2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  const uint16_t *s[2] = {srcs[0],srcs[1]};
  uint16_t *dp = (uint16_t *)dst;

  if (count & 0x01) {
    (*dp) = (*s[0]) ^ (*s[1]);

    s[0]++;
    s[1]++;
    dp++;
  }
  int num = count >> 1;
  if (num) {
    _core_uint32_bxor2((uint32_t *)dp, (const uint32_t **)s, 2, num);
  }
  return;
}

void _core_uint16_land2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  uint16_t *dp = (uint16_t *)dst;
  const uint16_t *s0 = (const uint16_t *)srcs[0];
  const uint16_t *s1 = (const uint16_t *)srcs[1];

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
	"lhz     5,0(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,0(%[s1]);"

	"cmpwi   1,6,0;"

	"crnor   24,2,6;"
	"lhz     5,2(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,2(%[s1]);"

	"cmpwi   1,6,0;"

	"crnor   25,2,6;"
	"lhz     5,4(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,4(%[s1]);"

	"cmpwi   1,6,0;"

	"crnor   26,2,6;"
	"lhz     5,6(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,6(%[s1]);"

	"cmpwi   1,6,0;"

	"crnor   27,2,6;"
	"lhz     5,8(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,8(%[s1]);"

	"cmpwi   1,6,0;"

	"crnor   28,2,6;"
	"lhz     5,10(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,10(%[s1]);"

	"cmpwi   1,6,0;"

	"crnor   29,2,6;"
	"lhz     5,12(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,12(%[s1]);"

	"cmpwi   1,6,0;"

	"crnor   30,2,6;"
	"lhz     5,14(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,14(%[s1]);"

	"cmpwi   1,6,0;"

	"crnor   31,2,6;"


	"mfcr    5;"


	"rlwinm  6,5,25,31,31;"

	"sth     6,0(%[dp]);"
	"rlwinm  7,5,26,31,31;"

	"sth     7,2(%[dp]);"
	"rlwinm  6,5,27,31,31;"

	"sth     6,4(%[dp]);"
	"rlwinm  7,5,28,31,31;"

	"sth     7,6(%[dp]);"
	"rlwinm  6,5,29,31,31;"

	"sth     6,8(%[dp]);"
	"rlwinm  7,5,30,31,31;"

	"sth     7,10(%[dp]);"
	"rlwinm  6,5,31,31,31;"

	"sth     6,12(%[dp]);"
	"andi.   7,5,0x01;"

	"sth     7,14(%[dp]);"




      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
	[dp] "b" (dp)
      : "memory", "5", "6", "7"
    );

    s0 += 8;
    s1 += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    *dp = (*s0) && (*s1);
    dp++;
    s0++;
    s1++;
  }

  return;
}

void _core_uint16_lor2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  const uint16_t *s0 = (const uint16_t *)srcs[0];
  const uint16_t *s1 = (const uint16_t *)srcs[1];
  uint16_t *dp = (uint16_t *)dst;

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
	"lhz    6,0(%[s0]);"

	"lhz    7,0(%[s1]);"

	"lhz    8,2(%[s0]);"
	"or.    5,6,7;"

	"crnot  24,2;"
	"lhz    9,2(%[s1]);"

	"or.    5,8,9;"
	"lhz    6,4(%[s0]);"

	"crnot  25,2;"
	"lhz    7,4(%[s1]);"

	"or.    5,6,7;"
	"lhz    8,6(%[s0]);"

	"crnot  26,2;"
	"lhz    9,6(%[s1]);"

	"or.    5,8,9;"
	"lhz    6,8(%[s0]);"

	"crnot  27,2;"
	"lhz    7,8(%[s1]);"

	"or.    5,6,7;"
	"lhz    8,10(%[s0]);"

	"crnot  28,2;"
	"lhz    9,10(%[s1]);"

	"or.    5,8,9;"
	"lhz    6,12(%[s0]);"

	"crnot  29,2;"
	"lhz    7,12(%[s1]);"

	"or.    5,6,7;"
	"lhz    8,14(%[s0]);"

	"crnot  30,2;"
	"lhz    9,14(%[s1]);"

	"or.    5,8,9;"

	"crnot  31,2;"


	"mfcr   5;"

	"rlwinm 6,5,25,31,31;"

	"sth    6,0(%[dp]);"
	"rlwinm 7,5,26,31,31;"

	"sth    7,2(%[dp]);"
	"rlwinm 8,5,27,31,31;"

	"sth    8,4(%[dp]);"
	"rlwinm 9,5,28,31,31;"

	"sth    9,6(%[dp]);"
	"rlwinm 6,5,29,31,31;"

	"sth    6,8(%[dp]);"
	"rlwinm 7,5,30,31,31;"

	"sth    7,10(%[dp]);"
	"rlwinm 8,5,31,31,31;"

	"sth    8,12(%[dp]);"
	"rlwinm 9,5,0,31,31;"

	"sth    9,14(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory", "5",  "6",  "7",  "8",  "9"
        );
    s0 += 8;
    s1 += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    (*dp) = (*s0) || (*s1);

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_uint16_lxor2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  uint16_t *dp = (uint16_t *)dst;
  const uint16_t *s0 = (const uint16_t *)srcs[0];
  const uint16_t *s1 = (const uint16_t *)srcs[1];

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
	"lhz     5,0(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,0(%[s1]);"

	"cmpwi   1,6,0;"

	"crxor   24,2,6;"
	"lhz     5,2(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,2(%[s1]);"

	"cmpwi   1,6,0;"

	"crxor   25,2,6;"
	"lhz     5,4(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,4(%[s1]);"

	"cmpwi   1,6,0;"

	"crxor   26,2,6;"
	"lhz     5,6(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,6(%[s1]);"

	"cmpwi   1,6,0;"

	"crxor   27,2,6;"
	"lhz     5,8(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,8(%[s1]);"

	"cmpwi   1,6,0;"

	"crxor   28,2,6;"
	"lhz     5,10(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,10(%[s1]);"

	"cmpwi   1,6,0;"

	"crxor   29,2,6;"
	"lhz     5,12(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,12(%[s1]);"

	"cmpwi   1,6,0;"

	"crxor   30,2,6;"
	"lhz     5,14(%[s0]);"

	"cmpwi   0,5,0;"
	"lhz     6,14(%[s1]);"

	"cmpwi   1,6,0;"

	"crxor   31,2,6;"


	"mfcr    5;"



	"rlwinm  6,5,25,31,31;"
	"sth     6,0(%[dp]);"

	"rlwinm  7,5,26,31,31;"

	"sth     7,2(%[dp]);"
	"rlwinm  6,5,27,31,31;"

	"sth     6,4(%[dp]);"
	"rlwinm  7,5,28,31,31;"

	"sth     7,6(%[dp]);"
	"rlwinm  6,5,29,31,31;"

	"sth     6,8(%[dp]);"
	"rlwinm  7,5,30,31,31;"

	"sth     7,10(%[dp]);"
	"rlwinm  6,5,31,31,31;"

	"sth     6,12(%[dp]);"
	"andi.   7,5,0x01;"

	"sth     7,14(%[dp]);"




      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
	[dp] "b" (dp)
      : "memory", "5", "6", "7"
    );

    s0 += 8;
    s1 += 8;
    dp += 8;
  }

  n = count & 0x07;
  register uint16_t s0_r;
  register uint16_t s1_r;
  while ( n-- ) {

    s0_r = *s0;
    s1_r = *s1;

    *dp = (s0_r && !s1_r) || (!s0_r && s1_r);

    dp++;
    s0++;
    s1++;
  }

  return;
}

void _core_uint16_max2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  uint16_t *dp = (uint16_t *)dst;
  const uint16_t *s0 = (const uint16_t *)srcs[0];
  const uint16_t *s1 = (const uint16_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lhz     5,0(%[s0]);"

      "lhz     9,0(%[s1]);"

      "lhz     6,2(%[s0]);"
      "cmpw    5,9;"

      "lhz     10,2(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0:    sth     5,0(%[dp]);"

      "lhz     7,4(%[s0]);"

      "lhz     8,6(%[s0]);"
      "cmpw    6,10;"

      "lhz     11,4(%[s1]);"
      "bge     1f;"

      "mr      6,10;"

"1:    sth     6,2(%[dp]);"
      "cmpw    7,11;"

      "lhz     12,6(%[s1]);"
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

  register uint16_t s0_r;
  register uint16_t s1_r;
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

void _core_uint16_min2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  uint16_t *dp = (uint16_t *)dst;
  const uint16_t *s0 = (const uint16_t *)srcs[0];
  const uint16_t *s1 = (const uint16_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lhz     5,0(%[s0]);"

      "lhz     9,0(%[s1]);"

      "lhz     6,2(%[s0]);"
      "cmpw    9,5;"

      "lhz     10,2(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0: "
      "sth     5,0(%[dp]);"

      "lhz     7,4(%[s0]);"

      "lhz     11,4(%[s1]);"
      "cmpw    10,6;"

      "lhz     8,6(%[s0]);"
      "bge     1f;"

      "mr      6,10;"
"1: "

      "sth     6,2(%[dp]);"
      "cmpw    11,7;"

      "lhz     12,6(%[s1]);"
      "bge     2f;"

      "mr      7,11;"

"2: "
      "sth     7,4(%[dp]);"
      "cmpw    12,8;"

      "bge     3f;"

      "mr      8,12;"

"3: "
      "sth     8,6(%[dp]);"

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

  register uint16_t s0_r;
  register uint16_t s1_r;
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

void _core_uint16_prod2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  uint16_t *dp = (uint16_t *)dst;
  const uint16_t *s0 = (const uint16_t *)srcs[0];
  const uint16_t *s1 = (const uint16_t *)srcs[1];

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"

      "lwz     6,0(%[s1]);"

      "lwz     9,4(%[s0]);"
      "mulhhw  7,5,6;"

      "mullhw  8,5,6;"
      "sth     7,0(%[dp]);"

      "lwz    10,4(%[s1]);"

      "sth     8,2(%[dp]);"
      "mulhhw 11,9,10;"

      "mullhw 12,9,10;"
      "sth    11,4(%[dp]);"

      "sth    12,6(%[dp]);"


      "lwz     5,8(%[s0]);"

      "lwz     6,8(%[s1]);"

      "lwz     9,12(%[s0]);"
      "mulhhw  7,5,6;"

      "mullhw  8,5,6;"
      "sth     7,8(%[dp]);"

      "lwz    10,12(%[s1]);"

      "sth     8,10(%[dp]);"
      "mulhhw 11,9,10;"

      "mullhw 12,9,10;"
      "sth    11,12(%[dp]);"

      "sth    12,14(%[dp]);"


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

  register int s0_r;
  register int s1_r;

  n = count & 0x07;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    *(dp+0) = s0_r * s1_r;

    dp++;
    s0++;
    s1++;
  }

  return;
}

void _core_uint16_sum2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {

  const uint16_t *s0 = (const uint16_t *)srcs[0];
  const uint16_t *s1 = (const uint16_t *)srcs[1];
  uint16_t *dp = (uint16_t *)dst;

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lhz     5,0(%[s0]);"

      "lhz     9,0(%[s1]);"

      "lhz     6,2(%[s0]);"

      "lhz     10,2(%[s1]);"

      "lhz     7,4(%[s0]);"
      "add     12,5,9;"

      "lhz     11,4(%[s1]);"

      "sth     12,0(%[dp]);"

      "add     5,6,10;"
      "lhz     8,6(%[s0]);"

      "sth     5,2(%[dp]);"
      "add     6,7,11;"

      "lhz     12,6(%[s1]);"

      "sth     6,4(%[dp]);"
      "add     7,8,12;"

      "sth     7,6(%[dp]);"
      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    s0 += 4;
    s1 += 4;
    dp += 4;
  }

  register uint16_t s0_r;
  register uint16_t s1_r;

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

/**
 * \brief Optimized ppc450d (Blue Gene/P) boolean pack.
 * \warning Causing a segfault when the src and dst buffers are freed ?
 */
void _core_uint16_pack_o(uint16_t *dst, const uint16_t *src, int count) {

  uint8_t *p = (uint8_t *)dst;
  const uint16_t *s = src;

  int n = count >> 3;
  while ( n-- ) {

    asm volatile (
	"lwz    5,0(%[s]);"
	"lwz    6,4(%[s]);"

	"andis. 7,5,0xffff;"
	"crnand 24,2,2;"

	"andi.  7,5,0xffff;"
	"crnand 25,2,2;"

	"lwz    5,8(%[s]);"

	"andis. 7,6,0xffff;"
	"crnand 26,2,2;"

	"andi.  7,6,0xffff;"
	"crnand 27,2,2;"

	"lwz    6,12(%[s]);"

	"andis. 7,5,0xffff;"
	"crnand 28,2,2;"

	"andi.  7,5,0xffff;"
	"crnand 29,2,2;"

	"andis. 7,6,0xffff;"
	"crnand 30,2,2;"

	"andi.  7,6,0xffff;"
	"crnand 31,2,2;"

	"mfcr   5;"
	"stb    5,0(%[p]);"

      : // no outputs
      : [s] "b" (s),
        [p] "b" (p)
      : "memory", "5", "6", "7"
    );

    s += 8;
    p++;
  }

#if 0
  int n = count >> 3;
  while ( n-- ) {
    (*p) = 0;
    asm volatile (
        "lwz    5,0(%[s]);"
        "lwz    6,4(%[s]);"
        "lwz    7,8(%[s]);"
        "lwz    8,12(%[s]);"

"pack0h: andis.  9,5,0xffff;"
        "beq    pack1h;"
        "ori    %[p],%[p],0x080;"

"pack1h: andi.  9,5,0xffff;"
        "beq    pack2h;"
        "ori    %[p],%[p],0x040;"

"pack2h: andis.  9,6,0xffff;"
        "beq    pack3h;"
        "ori    %[p],%[p],0x020;"

"pack3h: andi.  9,6,0xffff;"
        "beq    pack4h;"
        "ori    %[p],%[p],0x010;"

"pack4h: andis.  9,7,0xffff;"
        "beq    pack5h;"
        "ori    %[p],%[p],0x008;"

"pack5h: andi.  9,7,0xffff;"
        "beq    pack6h;"
        "ori    %[p],%[p],0x004;"

"pack6h: andis.  9,8,0xffff;"
        "beq    pack7h;"
        "ori    %[p],%[p],0x002;"

"pack7h: andi.  9,8,0xffff;"
        "beq    done_h;"
        "ori    %[p],%[p],0x001;"

"done_h: nop;"

      : [p] "+r" (*p)
      : [s] "b"  (s)
      : "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"
    );

    s += 8;
    p++;
  }
#endif /* 0 */

  n = count & 0x07;
  (*p) = 0;

  if (n--) {
    if (*(s++)) { *(p) |= 0x080; }

    if (n--) {
      if (*(s++)) { *(p) |= 0x040; }

      if (n--) {
        if (*(s++)) { *(p) |= 0x020; }

        if (n--) {
          if (*(s++)) { *(p) |= 0x010; }

          if (n--) {
            if (*(s++)) { *(p) |= 0x008; }

            if (n--) {
              if (*(s++)) { *(p) |= 0x004; }

              if (n--) {
                if (*(s++)) { *(p) |= 0x002; }
              }
	    }
	  }
	}
      }
    }
  }
}

/**
 * \brief Optimized ppc450d (Blue Gene/P) boolean unpack.
 * \warning Causing a segfault when the src and dst buffers are freed ?
 */
void _core_uint16_unpack_o(uint16_t *dst, const uint16_t *src, int count) {

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

#if 0
  uint16_t *d = dst;
  const uint8_t *p = (const uint8_t *)src;

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
        "lbz    7,0(%[p]);"

	"andi.  5,7,0x0080;"
	"sth    5,0(%[d]);"

	"andi.  6,7,0x0040;"
	"sth    6,2(%[d]);"

	"andi.  5,7,0x0020;"
	"sth    5,4(%[d]);"

	"andi.  6,7,0x0010;"
	"sth    6,6(%[d]);"

	"andi.  5,7,0x0008;"
	"sth    5,8(%[d]);"

	"andi.  6,7,0x0004;"
	"sth    6,10(%[d]);"

	"andi.  5,7,0x0002;"
	"sth    5,12(%[d]);"

	"andi.  6,7,0x0001;"
	"sth    6,14(%[d]);"

      : // no outputs
      : [p] "b" (p),
        [d] "b" (d)
      : "memory", "5", "6", "7"
    );

    d += 8;
    p++;
  }

  n = count & 0x07;
  if ( n-- ) {
    (*d) = (*p) & 0x080;
    d++;

    if ( n-- ) {
      (*d) = (*p) & 0x040;
      d++;

      if ( n-- ) {
        (*d) = (*p) & 0x020;
        d++;

        if ( n-- ) {
          (*d) = (*p) & 0x010;
          d++;

          if ( n-- ) {
            (*d) = (*p) & 0x008;
            d++;

            if ( n-- ) {
              (*d) = (*p) & 0x004;
              d++;

              if ( n-- ) {
                (*d) = (*p) & 0x002;
                d++;
	      }
	    }
	  }
	}
      }
    }
  }
#endif /* 0 */
}

void _core_uint16_conv_o(uint16_t *dst, const uint16_t *src, int count) {

  uint16_t *dp = dst;
  const uint16_t *sp = src;

  register uint32_t add = 0x8000;
  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
      "lha    5,0(%[sp]);"

      "sub    5,5,%[add];"
      "lha    6,2(%[sp]);"

      "sub    6,6,%[add];"
      "lha    7,4(%[sp]);"

      "sub    7,7,%[add];"
      "lha    8,6(%[sp]);"

      "sub    8,8,%[add];"
      "lha    9,8(%[sp]);"

      "sub    9,9,%[add];"
      "lha   10,10(%[sp]);"

      "sub   10,10,%[add];"
      "lha   11,12(%[sp]);"

      "lha   12,14(%[sp]);"
      "sub   11,11,%[add];"

      "sub   12,12,%[add];"
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
      "sub   12,12,%[add];"
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

void _core_uint16_not_o(uint16_t *dst, const uint16_t *src, int count) {

  uint16_t *dp = (uint16_t *)dst;
  uint16_t *sp = (uint16_t *)src;

  if ( count & 0x01 ) {
    (*dp) = ~(*sp);
    sp++;
    dp++;
  }

  _core_uint32_not_o((uint32_t *)dp, (const uint32_t *)sp, count>>1);
}

void _core_uint16_conv_not_o(uint16_t *dst, const uint16_t *src, int count) {

  uint16_t *dp = dst;
  const uint16_t *sp = src;

  register uint32_t add = 0x8000;
  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
      "lha    5,0(%[sp]);"

      "sub    5,5,%[add];"
      "lha    6,2(%[sp]);"

      "not    5,5;"
      "lha    7,4(%[sp]);"

      "sth    5,0(%[dp]);"
      "sub    6,6,%[add];"

      "not    6,6;"
      "lha    8,6(%[sp]);"

      "sth    6,2(%[dp]);"
      "sub    7,7,%[add];"

      "not    7,7;"
      "lha    9,8(%[sp]);"

      "sth    7,4(%[dp]);"
      "sub    8,8,%[add];"

      "not    8,8;"
      "lha   10,10(%[sp]);"

      "sth    8,6(%[dp]);"
      "sub    9,9,%[add];"

      "not    9,9;"
      "lha   11,12(%[sp]);"

      "sth    9,8(%[dp]);"
      "sub   10,10,%[add];"

      "not   10,10;"
      "lha   12,14(%[sp]);"

      "sth   10,10(%[dp]);"
      "sub   11,11,%[add];"

      "not   11,11;"

      "sth   11,12(%[dp]);"
      "sub   12,12,%[add];"

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
      "sub   12,12,%[add];"
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

void _core_uint16_pre_min_o(uint16_t *dst, const uint16_t *src, int count) {
  _core_uint16_not_o(dst, src, count);
}

void _core_uint16_post_min_o(uint16_t *dst, const uint16_t *src, int count) {
  _core_uint16_not_o(dst, src, count);
}

void _core_uint16_band_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_band2(dst, srcs, nsrc, count);
}

void _core_uint16_bor_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_bor2(dst, srcs, nsrc, count);
}

void _core_uint16_bxor_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_bxor2(dst, srcs, nsrc, count);
}

void _core_uint16_land_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_land2(dst, srcs, nsrc, count);
}

void _core_uint16_lor_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_lor2(dst, srcs, nsrc, count);
}

void _core_uint16_lxor_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_lxor2(dst, srcs, nsrc, count);
}

void _core_uint16_min_unmarshall_o(uint16_t *dst, const uint16_t *src, int count) {
  _core_uint16_not_o(dst, src, count);
}

void _core_uint16_sum_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
  _core_uint16_sum2(dst, srcs, nsrc, count);
}
