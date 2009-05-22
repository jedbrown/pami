/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/uint8_o.cc
 * \brief Optimized math routines for unsigned 8 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "dcmf_bg_math.h"
#include "Util.h"
#include "ppc450d/internal_o.h"

void _core_uint8_band2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {

  const uint8_t *s[2] = {srcs[0],srcs[1]};
  uint8_t *dp = (uint8_t *)dst;

  int n = count & 0x03;
  while (n--) {
    (*dp) = (*s[0]) & (*s[1]);
    s[0]++;
    s[1]++;
    dp++;
  }

  _core_uint32_band2((uint32_t *)dp, (const uint32_t **)s, nsrc, (count >> 2));

  return;
}

void _core_uint8_bor2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {

  const uint8_t *s[2] = {srcs[0],srcs[1]};
  uint8_t *dp = (uint8_t *)dst;

  int n = count & 0x03;
  while (n--) {
    (*dp) = (*s[0]) | (*s[1]);
    s[0]++;
    s[1]++;
    dp++;
  }

  _core_uint32_bor2((uint32_t *)dp, (const uint32_t **)s, nsrc, (count >> 2));

  return;
}

void _core_uint8_bxor2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {

  const uint8_t *s[2] = {srcs[0],srcs[1]};
  uint8_t *dp = (uint8_t *)dst;

  int n = count & 0x03;
  while (n--) {
    (*dp) = (*s[0]) ^ (*s[1]);
    s[0]++;
    s[1]++;
    dp++;
  }

  _core_uint32_bxor2((uint32_t *)dp, (const uint32_t **)s, nsrc, (count >> 2));

  return;
}

void _core_uint8_land2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {

  uint8_t *dp = (uint8_t *)dst;
  const uint8_t *s0 = (const uint8_t *)srcs[0];
  const uint8_t *s1 = (const uint8_t *)srcs[1];

  register uint32_t zero = 0;
  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
	"mtcr    %[z];"

	"lwz     5,0(%[s0]);"
	"lwz     6,0(%[s1]);"


	"andi.   7,5,0x00ff;"
	"crmove  31,2;"

	"andi.   7,6,0x00ff;"
	"crnor   31,31,2;"


	"andi.   7,5,0xff00;"
	"crmove  23,2;"

	"andi.   7,6,0xff00;"
	"crnor   23,23,2;"


	"andis.  7,5,0x00ff;"
	"crmove  15,2;"

	"andis.  7,6,0x00ff;"
	"crnor   15,15,2;"


	"andis.  7,5,0xff00;"
	"crmove  7,2;"

	"andis.  7,6,0xff00;"
	"crnor   7,7,2;"


	"crclr   0;"
	"crclr   1;"
	"crclr   2;"

	"mfcr    7;"
	"stw     7,0(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
	[dp] "b" (dp),
	[z] "r" (zero)
      : "memory", "5", "6", "7"
    );

    s0 += 4;
    s1 += 4;
    dp += 4;
  }

  n = count & 0x03;
  while ( n-- ) {
    *dp = (*s0) && (*s1);
    dp++;
    s0++;
    s1++;
  }

  return;
}

void _core_uint8_lor2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {

  uint8_t *dp = (uint8_t *)dst;
  const uint8_t *s0 = (const uint8_t *)srcs[0];
  const uint8_t *s1 = (const uint8_t *)srcs[1];

  register uint32_t zero = 0;
  int n = count >> 2;
  while ( n-- ) {
    asm volatile (

	"lwz     5,0(%[s0]);"
	"lwz     6,0(%[s1]);"
	"mtcr    %[z];"
	"or      5,5,6;"


	"andi.   7,5,0x00ff;"
	"crnot   31,2;"

	"andi.   7,5,0xff00;"
	"crnot   23,2;"

	"andis.  7,5,0x00ff;"
	"crnot   15,2;"

	"andis.  7,5,0xff00;"
	"crnot   7,2;"


	"crclr   0;"
	"crclr   1;"
	"crclr   2;"

	"mfcr    7;"
	"stw     7,0(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
	[dp] "b" (dp),
	[z] "r" (zero)
      : "memory", "5", "6", "7"
    );

    s0 += 4;
    s1 += 4;
    dp += 4;
  }

  n = count & 0x03;
  while ( n-- ) {
    (*dp) = (*s0) || (*s1);

    dp++;
    s0++;
    s1++;
  }

  return;
}

void _core_uint8_lxor2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {

  uint8_t *dp = (uint8_t *)dst;
  const uint8_t *s0 = (const uint8_t *)srcs[0];
  const uint8_t *s1 = (const uint8_t *)srcs[1];

  register uint32_t zero = 0;
  int n = count >> 2;
  while ( n-- ) {
    asm volatile (

	"mtcr    %[z];"

	"lwz     5,0(%[s0]);"
	"lwz     6,0(%[s1]);"


	"andi.   7,5,0x00ff;"
	"crmove  31,2;"

	"andi.   7,6,0x00ff;"
	"crxor   31,31,2;"


	"andi.   7,5,0xff00;"
	"crmove  23,2;"

	"andi.   7,6,0xff00;"
	"crxor   23,23,2;"


	"andis.  7,5,0x00ff;"
	"crmove  15,2;"

	"andis.  7,6,0x00ff;"
	"crxor   15,15,2;"


	"andis.  7,5,0xff00;"
	"crmove  7,2;"

	"andis.  7,6,0xff00;"
	"crxor   7,7,2;"


	"crclr   0;"
	"crclr   1;"
	"crclr   2;"

	"mfcr    7;"
	"stw     7,0(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
	[dp] "b" (dp),
	[z] "r" (zero)
      : "memory", "5", "6", "7"
    );

    s0 += 4;
    s1 += 4;
    dp += 4;
  }

  n = count & 0x03;
  register uint8_t s0_r;
  register uint8_t s1_r;
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

void _core_uint8_max2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  uint8_t *dp = (uint8_t *)dst;
  const uint8_t *s0 = (const uint8_t *)srcs[0];
  const uint8_t *s1 = (const uint8_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[s0]);"

      "lbz     9,0(%[s1]);"

      "lbz     6,1(%[s0]);"
      "cmpw    5,9;"

      "lbz     10,1(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0:    stb     5,0(%[dp]);"



      "lbz     7,2(%[s0]);"
      "cmpw    6,10;"

      "lbz     11,2(%[s1]);"
      "bge     1f;"

      "mr      6,10;"

"1:    stb     6,1(%[dp]);"



      "lbz     8,3(%[s0]);"
      "cmpw    7,11;"

      "lbz     12,3(%[s1]);"
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

  register uint8_t s0_r;
  register uint8_t s1_r;
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

void _core_uint8_min2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  uint8_t *dp = (uint8_t *)dst;
  const uint8_t *s0 = (const uint8_t *)srcs[0];
  const uint8_t *s1 = (const uint8_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[s0]);"
      "lbz     9,0(%[s1]);"
      "lbz     6,1(%[s0]);"
      "cmpw    9,5;"
      "lbz     10,1(%[s1]);"
      "bge     0f;"
      "mr      5,9;"
"0:    stb     5,0(%[dp]);"

      "lbz     7,2(%[s0]);"
      "cmpw    10,6;"
      "lbz     11,2(%[s1]);"
      "bge     1f;"
      "mr      6,10;"
"1:    stb     6,1(%[dp]);"

      "lbz     8,3(%[s0]);"
      "cmpw    11,7;"
      "lbz     12,3(%[s1]);"
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

  register uint8_t s0_r;
  register uint8_t s1_r;
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

void _core_uint8_prod2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {

  uint8_t *dp = (uint8_t *)dst;
  const uint8_t *s0 = (const uint8_t *)srcs[0];
  const uint8_t *s1 = (const uint8_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[s0]);"

      "lbz     6,0(%[s1]);"

      "mullw   5,5,6;"
      "lbz     7,1(%[s0]);"

      "lbz     8,1(%[s1]);"

      "stb     5,0(%[dp]);"
      "mullw   7,7,8;"

      "stb     7,1(%[dp]);"

      "lbz     5,2(%[s0]);"

      "lbz     6,2(%[s1]);"

      "mullw   5,5,6;"
      "lbz     7,3(%[s0]);"

      "lbz     8,3(%[s1]);"

      "stb     5,2(%[dp]);"
      "mullw   7,7,8;"

      "stb     7,3(%[dp]);"


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

  n = count & 0x03;
  while ( n-- ) {
    (*dp) = (*s0) * (*s1);

    dp++;
    s0++;
    s1++;
  }

  return;
}

void _core_uint8_sum2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {

  const uint8_t *s0 = (const uint8_t *)srcs[0];
  const uint8_t *s1 = (const uint8_t *)srcs[1];
  uint8_t *dp = (uint8_t *)dst;

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[s0]);"

      "lbz     6,0(%[s1]);"

      "add     5,5,6;"
      "lbz     7,1(%[s0]);"

      "stb     5,0(%[dp]);"

      "lbz     8,1(%[s1]);"

      "add     7,7,8;"
      "lbz     9,2(%[s0]);"

      "stb     7,1(%[dp]);"

      "lbz     10,2(%[s1]);"

      "add     9,9,10;"
      "lbz     11,3(%[s0]);"

      "lbz     12,3(%[s1]);"

      "stb     9,2(%[dp]);"
      "add     11,11,12;"

      "stb     11,3(%[dp]);"

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

  n = count & 0x07;
  while ( n-- ) {
    (*dp) = (*s0) + (*s1);

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _core_uint8_pack_o(uint8_t *dst, const uint8_t *src, int count) {

  uint8_t *p = (uint8_t *)dst;
  const uint8_t *s = (const uint8_t *)src;

  int n = count >> 3;
  while ( n-- ) {

    asm volatile (
	"lwz    5,0(%[s]);"
	"lwz    6,4(%[s]);"

	"andis. 7,5,0xff00;"
	"crnand 24,2,2;"

	"andis. 7,5,0x00ff;"
	"crnand 25,2,2;"

	"andi.  7,5,0xff00;"
	"crnand 26,2,2;"

	"andi.  7,5,0x00ff;"
	"crnand 27,2,2;"

	"andis. 7,6,0xff00;"
	"crnand 28,2,2;"

	"andis. 7,6,0x00ff;"
	"crnand 29,2,2;"

	"andi.  7,6,0xff00;"
	"crnand 30,2,2;"

	"andi.  7,6,0x00ff;"
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

void _core_uint8_unpack_o(uint8_t *dst, const uint8_t *src, int count) {

  uint8_t *d = (uint8_t *)dst;
  const uint8_t *p = (const uint8_t *)src;

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
        "lbz    7,0(%[p]);"

	"andi.  5,7,0x0080;"
	"stb    5,0(%[d]);"

	"andi.  6,7,0x0040;"
	"stb    6,1(%[d]);"

	"andi.  5,7,0x0020;"
	"stb    5,2(%[d]);"

	"andi.  6,7,0x0010;"
	"stb    6,3(%[d]);"

	"andi.  5,7,0x0008;"
	"stb    5,4(%[d]);"

	"andi.  6,7,0x0004;"
	"stb    6,5(%[d]);"

	"andi.  5,7,0x0002;"
	"stb    5,6(%[d]);"

	"andi.  6,7,0x0001;"
	"stb    6,7(%[d]);"

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
}

void _core_uint8_conv_o(uint8_t *dst, const uint8_t *src, int count) {

  uint8_t *dp = dst;
  const uint8_t *sp = src;
  int n = count >> 3;

  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[sp]);"

      "extsb   5,5;"
      "lbz     6,1(%[sp]);"

      "subi    5,5,0x80;"

      "stb     5,0(%[dp]);"
      "extsb   6,6;"

      "subi    6,6,0x80;"
      "lbz     7,2(%[sp]);"

      "stb     6,1(%[dp]);"
      "extsb   7,7;"

      "subi    7,7,0x80;"
      "lbz     8,3(%[sp]);"

      "stb     7,2(%[dp]);"
      "extsb   8,8;"

      "subi    8,8,0x80;"
      "lbz     5,4(%[sp]);"

      "stb     8,3(%[dp]);"
      "extsb   5,5;"

      "subi    5,5,0x80;"
      "lbz     6,5(%[sp]);"

      "stb     5,4(%[dp]);"
      "extsb   6,6;"

      "subi    6,6,0x80;"
      "lbz     7,6(%[sp]);"

      "stb     6,5(%[dp]);"
      "extsb   7,7;"

      "subi    7,7,0x80;"
      "lbz     8,7(%[sp]);"

      "stb     7,6(%[dp]);"
      "extsb   8,8;"

      "subi    8,8,0x80;"

      "stb     8,7(%[dp]);"

      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8"
    );

    sp += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    asm volatile (
      "lbz   12,0(%[sp]);"
      "extsb 12,12;"
      "subi  12,12,0x80;"
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

void _core_uint8_not_o(uint8_t *dst, const uint8_t *src, int count) {

  uint8_t *dp = (uint8_t *)dst;
  uint8_t *sp = (uint8_t *)src;

  int n = count & 0x03;
  while ( n-- ) {
    (*dp) = ~(*sp);
    sp++;
    dp++;
  }

  _core_uint32_not_o((uint32_t *)dp, (const uint32_t *)sp, count>>2);
}

void _core_uint8_conv_not_o(uint8_t *dst, const uint8_t *src, int count) {

  uint8_t *dp = dst;
  const uint8_t *sp = src;
  int n = count >> 3;

  while ( n-- ) {
    asm volatile (
      "lbz     5,0(%[sp]);"

      "extsb   5,5;"

      "subi    5,5,0x80;"

      "not     5,5;"
      "lbz     6,1(%[sp]);"

      "stb     5,0(%[dp]);"
      "extsb   6,6;"

      "subi    6,6,0x80;"

      "not     6,6;"
      "lbz     7,2(%[sp]);"

      "stb     6,1(%[dp]);"
      "extsb   7,7;"

      "subi    7,7,0x80;"

      "not     7,7;"
      "lbz     8,3(%[sp]);"

      "stb     7,2(%[dp]);"
      "extsb   8,8;"

      "subi    8,8,0x80;"

      "not     8,8;"
      "lbz     5,4(%[sp]);"

      "stb     8,3(%[dp]);"
      "extsb   5,5;"

      "subi    5,5,0x80;"

      "not     5,5;"
      "lbz     6,5(%[sp]);"

      "stb     5,4(%[dp]);"
      "extsb   6,6;"

      "subi    6,6,0x80;"

      "not     6,6;"
      "lbz     7,6(%[sp]);"

      "stb     6,5(%[dp]);"
      "extsb   7,7;"

      "subi    7,7,0x80;"

      "not     7,7;"
      "lbz     8,7(%[sp]);"

      "stb     7,6(%[dp]);"
      "extsb   8,8;"

      "subi    8,8,0x80;"

      "not     8,8;"

      "stb     8,7(%[dp]);"

      : // no outputs
      : [sp] "b" (sp),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8"
    );

    sp += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    asm volatile (
      "lbz   12,0(%[sp]);"
      "extsb 12,12;"
      "subi  12,12,0x80;"
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

void _core_uint8_pre_min_o(uint8_t *dst, const uint8_t *src, int count) {
  _core_uint8_not_o(dst, src, count);
}

void _core_uint8_post_min_o(uint8_t *dst, const uint8_t *src, int count) {
  _core_uint8_not_o(dst, src, count);
}

void _core_uint8_min_unmarshall_o(uint8_t *dst, const uint8_t *src, int count) {
  _core_uint8_not_o(dst, src, count);
}

void _core_uint8_band_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_band2(dst, srcs, nsrc, count);
}

void _core_uint8_bor_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_bor2(dst, srcs, nsrc, count);
}

void _core_uint8_bxor_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_bxor2(dst, srcs, nsrc, count);
}

void _core_uint8_land_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_land2(dst, srcs, nsrc, count);
}

void _core_uint8_lor_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_lor2(dst, srcs, nsrc, count);
}

void _core_uint8_lxor_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_lxor2(dst, srcs, nsrc, count);
}

void _core_uint8_max_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_max2(dst, srcs, nsrc, count);
}

void _core_uint8_sum_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
  _core_uint8_sum2(dst, srcs, nsrc, count);
}
