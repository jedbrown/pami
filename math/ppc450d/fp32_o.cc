/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/fp32_o.cc
 * \brief Optimized math routines for 32 bit floating point operations on
 *        the ppc 450 dual fpu architecture.
 *
 * \note Optimized 2-way sum float turned off.
 */
#include "dcmf_bg_math.h"
#include "FloatUtil.h"
#include "Util.h"
#include "ppc450d/internal_o.h"

static inline void _sum_aligned2(float *dst, const float **srcs, int nsrc, int count) {

  uint8_t *dp = (uint8_t *)dst;
  uint8_t *s0 = (uint8_t *)srcs[0];
  uint8_t *s1 = (uint8_t *)srcs[1];

  s0 -= 8;
  s1 -= 8;
  dp -= 8;

  register int tmp = 8;

  int n = count >> 4;
  while ( n-- ) {

    asm volatile(

      "lfpsux  1,%[s0],%[offset];"
      "lfpsux  2,%[s0],%[offset];"
      "lfpsux  3,%[s0],%[offset];"
      "lfpsux  4,%[s0],%[offset];"
      "lfpsux  5,%[s1],%[offset];"
      "lfpsux  6,%[s1],%[offset];"
      "lfpsux  7,%[s1],%[offset];"
      "lfpsux  8,%[s1],%[offset];"
      "fpadd   1,1,5;"
      "stfpsux 1,%[dp],%[offset];"
      "fpadd   2,2,6;"
      "stfpsux 2,%[dp],%[offset];"
      "fpadd   3,3,7;"
      "stfpsux 3,%[dp],%[offset];"
      "fpadd   4,4,8;"
      "stfpsux 4,%[dp],%[offset];"

      "lfpsux  1,%[s0],%[offset];"
      "lfpsux  2,%[s0],%[offset];"
      "lfpsux  3,%[s0],%[offset];"
      "lfpsux  4,%[s0],%[offset];"
      "lfpsux  5,%[s1],%[offset];"
      "lfpsux  6,%[s1],%[offset];"
      "lfpsux  7,%[s1],%[offset];"
      "lfpsux  8,%[s1],%[offset];"
      "fpadd   1,1,5;"
      "stfpsux 1,%[dp],%[offset];"
      "fpadd   2,2,6;"
      "stfpsux 2,%[dp],%[offset];"
      "fpadd   3,3,7;"
      "stfpsux 3,%[dp],%[offset];"
      "fpadd   4,4,8;"
      "stfpsux 4,%[dp],%[offset];"

      : [s0] "+b" (s0),
        [s1] "+b" (s1),
        [dp] "+b" (dp)
      : [offset] "r" (tmp)
      : "memory", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7", "fr8"
    );
  }

  n = count & 0x0f;
  while ( n-- ) {
    asm volatile(

      "lfpsux  1,%[s0],%[offset];"
      "lfpsux  2,%[s1],%[offset];"
      "fpadd   1,1,2;"
      "stfpsux 1,%[dp],%[offset];"

      : [s0] "+b" (s0),
        [s1] "+b" (s1),
        [dp] "+b" (dp)
      : [offset] "r" (tmp)
      : "memory", "fr1", "fr2"
    );
  }
}

#if 0
void _core_fp32_sum2(float *dst, const float **srcs, int nsrc, int count) {

	if (( ((uint32_t) dst) | ((uint32_t) srcs[0]) | ((uint32_t) srcs[1])) & 0x03) {
		// a buffer is not 8-byte aligned.
		const float *s0 = srcs[0], *s1 = srcs[1];
		register int n=0;
		register float rbuffer0, rbuffer1, rbuffer2, rbuffer3;
		register float buf00, buf01, buf02, buf03;
		register float buf10, buf11, buf12, buf13;
		for(; n<count-3; n+=4) {
			buf00 = s0[n+0]; buf10 = s1[n+0];
			buf01 = s0[n+1]; buf11 = s1[n+1];
			buf02 = s0[n+2]; buf12 = s1[n+2];
			buf03 = s0[n+3]; buf13 = s1[n+3];

			rbuffer0 = buf00+buf10;
			rbuffer1 = buf01+buf11;
			rbuffer2 = buf02+buf12;
			rbuffer3 = buf03+buf13;

			dst[n+0] = rbuffer0;
			dst[n+1] = rbuffer1;
			dst[n+2] = rbuffer2;
			dst[n+3] = rbuffer3;
		}
		for(; n<count; n++) {
			dst[n] = s0[n]+s1[n];
		}
	} else {
		_sum_aligned2(dst, srcs[0], srcs[1], count);
	}

	return;
}
#else
//#warning optimized 2-way sum float turned off

void _core_fp32_sum2(float *dst, const float **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE float
#include "../_dual_src.x.h"
#undef TYPE
#undef OP
}
#endif

void _core_fp32_max2(float *dst, const float **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE float
#include "../_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp32_min2(float *dst, const float **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE float
#include "../_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp32_prod2(float *dst, const float **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE float
#include "../_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp32_int32_maxloc2(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
  const fp32_int32_t *s0 = srcs[0], *s1 = srcs[1];
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

void _core_fp32_fp32_maxloc2(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
  const fp32_fp32_t *s0 = srcs[0], *s1 = srcs[1];
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

void _core_fp32_int32_minloc2(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
  const fp32_int32_t *s0 = srcs[0], *s1 = srcs[1];
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

void _core_fp32_fp32_minloc2(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
  const fp32_fp32_t *s0 = srcs[0], *s1 = srcs[1];
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


void _core_fp32_pre_max_o(float *dst, const float *src, int count) {
  uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
  register int n=0;
  register unsigned d0, d1, d2, d3;
  register unsigned s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MAX_FLOAT_TOTREE(s0);
      d1 = MAX_FLOAT_TOTREE(s1);
      d2 = MAX_FLOAT_TOTREE(s2);
      d3 = MAX_FLOAT_TOTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MAX_FLOAT_TOTREE(s0);
      d[n] = d0;
    }
}

void _core_fp32_post_max_o(float *dst, const float *src, int count) {
  uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
  register int n=0;
  register unsigned d0, d1, d2, d3;
  register unsigned s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MAX_FLOAT_FRTREE(s0);
      d1 = MAX_FLOAT_FRTREE(s1);
      d2 = MAX_FLOAT_FRTREE(s2);
      d3 = MAX_FLOAT_FRTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MAX_FLOAT_FRTREE(s0);
      d[n] = d0;
    }
}

void _core_fp32_int32_pre_maxloc_o(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
    }
}

void _core_fp32_int32_post_maxloc_o(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _core_fp32_fp32_pre_maxloc_o(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_TOTREE(s[n].a);
      d[n].b = MIN_FLOAT_TOTREE(s[n].b);
    }
}

void _core_fp32_fp32_post_maxloc_o(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_FRTREE(s[n].a);
      d[n].b = MIN_FLOAT_FRTREE(s[n].b);
    }
}

void _core_fp32_pre_min_o(float *dst, const float *src, int count) {
  uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
  register int n=0;
  register unsigned d0, d1, d2, d3;
  register unsigned s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MIN_FLOAT_TOTREE(s0);
      d1 = MIN_FLOAT_TOTREE(s1);
      d2 = MIN_FLOAT_TOTREE(s2);
      d3 = MIN_FLOAT_TOTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MIN_FLOAT_TOTREE(s0);
      d[n] = d0;
    }
}

void _core_fp32_post_min_o(float *dst, const float *src, int count) {
  uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
  register int n=0;
  register unsigned d0, d1, d2, d3;
  register unsigned s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MIN_FLOAT_FRTREE(s0);
      d1 = MIN_FLOAT_FRTREE(s1);
      d2 = MIN_FLOAT_FRTREE(s2);
      d3 = MIN_FLOAT_FRTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MIN_FLOAT_FRTREE(s0);
      d[n] = d0;
    }
}

void _core_fp32_int32_pre_minloc_o(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
    }
}

void _core_fp32_int32_post_minloc_o(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _core_fp32_fp32_pre_minloc_o(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_TOTREE(s[n].a);
      d[n].b = MIN_FLOAT_TOTREE(s[n].b);
    }
}

void _core_fp32_fp32_post_minloc_o(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_FRTREE(s[n].a);
      d[n].b = MIN_FLOAT_FRTREE(s[n].b);
    }
}

void _core_fp32_max_marshall2(float *dst, const float **srcs, int nsrc, int count) {
  register int n=0;
  uint32_t *d=(uint32_t*)dst;
  for(; n<count; n++)
    {
      volatile float *a    = (float *) &srcs[0][n];
      volatile float *b    = (float *) &srcs[1][n];
      volatile float  val  = MAX((*a), (*b));
      volatile float *dval = &val;
      volatile unsigned *temp = (unsigned *)dval;
      d[n] = MAX_FLOAT_TOTREE(*temp);
    }
}

void _core_fp32_int32_maxloc_marshall2(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s0=(uint32_int32_t*)srcs[0], *s1=(uint32_int32_t*)srcs[1];
  for (n = 0; n < count; n++)
    {
      register unsigned tmp = MAX(s0[n].a,s1[n].a);
      d[n].a = MAX_FLOAT_TOTREE (tmp);
      if(s0[n].a > s1[n].a)
        d[n].b = s0[n].b;
      else if(s0[n].a < s1[n].a)
        d[n].b = s1[n].b;
      else
        d[n].b = MIN(s0[n].b, s1[n].b);
      d[n].b = ~d[n].b;
    }
}

void _core_fp32_fp32_maxloc_marshall2(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s0=(uint32_uint32_t*)srcs[0], *s1=(uint32_uint32_t*)srcs[1];
  for (n = 0; n < count; n++)
    {
      register unsigned tmp0 = MAX(s0[n].a,s1[n].a);
      register unsigned tmp1 = MIN(s0[n].b,s1[n].b);
      d[n].a = MAX_FLOAT_TOTREE (tmp0);
      if(s0[n].a > s1[n].a)
        d[n].b = s0[n].b;
      else if(s0[n].a < s1[n].a)
        d[n].b = s1[n].b;
      else
        d[n].b = MIN(s0[n].b, s1[n].b);

      tmp1 = (unsigned)d[n].b;
      d[n].b = MIN_FLOAT_TOTREE (tmp1);
    }
}

void _core_fp32_min_marshall2(float *dst, const float **srcs, int nsrc, int count) {
  register int n=0;
  uint32_t *d=(uint32_t*)dst;
  for(; n<count; n++)
    {
      volatile float *a    = (float *) &srcs[0][n];
      volatile float *b    = (float *) &srcs[1][n];
      volatile float  val  = MIN((*a), (*b));
      volatile float *dval = &val;
      volatile unsigned *temp = (unsigned *)dval;
      d[n] = MIN_FLOAT_TOTREE(*temp);
    }
}

void _core_fp32_int32_minloc_marshall2(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s0=(uint32_int32_t*)srcs[0], *s1=(uint32_int32_t*)srcs[1];
  for (n = 0; n < count; n++)
    {
      register unsigned tmp = MIN(s0[n].a,s1[n].a);
      d[n].a = MIN_FLOAT_TOTREE (tmp);
      if(s0[n].a < s1[n].a)
        d[n].b = s0[n].b;
      else if(s0[n].a > s1[n].a)
        d[n].b = s1[n].b;
      else
        d[n].b = MIN(s0[n].b, s1[n].b);
      d[n].b = ~d[n].b;
    }
}

void _core_fp32_fp32_minloc_marshall2(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s0=(uint32_uint32_t*)srcs[0], *s1=(uint32_uint32_t*)srcs[1];
  for (n = 0; n < count; n++)
    {
      register unsigned tmp0 = MIN(s0[n].a,s1[n].a);
      register unsigned tmp1 = MIN(s0[n].b,s1[n].b);
      d[n].a = MIN_FLOAT_TOTREE (tmp0);
      if(s0[n].a < s1[n].a)
        d[n].b = s0[n].b;
      else if(s0[n].a > s1[n].a)
        d[n].b = s1[n].b;
      else
        d[n].b = MIN(s0[n].b, s1[n].b);

      tmp1 = (unsigned)d[n].b;
      d[n].b = MIN_FLOAT_TOTREE (tmp1);
    }
}
