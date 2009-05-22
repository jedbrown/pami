/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file math/fp32.cc
 * \brief Default C math routines for 32 bit floating point operations.
 */

#include "math_bg_math.h"
#include "FloatUtil.h"
#include "internal.h"

extern "C" uint32_t FU_MAX_FLOAT_TOTREE(float f) {
	union { float f; uint32_t u; } x;
	x.f = f;
	return MAX_FLOAT_TOTREE(x.u);
}
extern "C" uint32_t FU_MAX_FLOAT_FRTREE(float f) {
	union { float f; uint32_t u; } x;
	x.f = f;
	return MAX_FLOAT_FRTREE(x.u);
}
extern "C" uint32_t FU_MIN_FLOAT_TOTREE(float f) {
	union { float f; uint32_t u; } x;
	x.f = f;
	return MIN_FLOAT_TOTREE(x.u);
}
extern "C" uint32_t FU_MIN_FLOAT_FRTREE(float f) {
	union { float f; uint32_t u; } x;
	x.f = f;
	return MIN_FLOAT_FRTREE(x.u);
}

void _core_fp32_max(float *dst, const float **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE float
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp32_min(float *dst, const float **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE float
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp32_prod(float *dst, const float **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE float
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp32_sum(float *dst, const float **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE float
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp32_int32_maxloc(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 > src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a < srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		dst[n].a = srcs[m][n].a;
		dst[n].b = srcs[m][n].b;
	}
}

void _core_fp32_fp32_maxloc(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 > src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a < srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		dst[n].a = srcs[m][n].a;
		dst[n].b = srcs[m][n].b;
	}
}

void _core_fp32_int32_minloc(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 < src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a > srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		dst[n].a = srcs[m][n].a;
		dst[n].b = srcs[m][n].b;
	}
}

void _core_fp32_fp32_minloc(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 < src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a > srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		dst[n].a = srcs[m][n].a;
		dst[n].b = srcs[m][n].b;
	}
}


void _core_fp32_pre_max(float *dst, const float *src, int count) {
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

void _core_fp32_post_max(float *dst, const float *src, int count) {
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

void _core_fp32_int32_pre_maxloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
    }
}

void _core_fp32_int32_post_maxloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _core_fp32_fp32_pre_maxloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_TOTREE(s[n].a);
      d[n].b = MIN_FLOAT_TOTREE(s[n].b);
    }
}

void _core_fp32_fp32_post_maxloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_FRTREE(s[n].a);
      d[n].b = MIN_FLOAT_FRTREE(s[n].b);
    }
}

void _core_fp32_pre_min(float *dst, const float *src, int count) {
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

void _core_fp32_post_min(float *dst, const float *src, int count) {
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

void _core_fp32_int32_pre_minloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
    }
}

void _core_fp32_int32_post_minloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _core_fp32_fp32_pre_minloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_TOTREE(s[n].a);
      d[n].b = MIN_FLOAT_TOTREE(s[n].b);
    }
}

void _core_fp32_fp32_post_minloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_FRTREE(s[n].a);
      d[n].b = MIN_FLOAT_FRTREE(s[n].b);
    }
}

void _core_fp32_max_marshall(float *dst, const float **srcs, int nsrc, int count) {
	register int n = 0, o;
	uint32_t *d = (uint32_t *)dst;
	union { float f; uint32_t u; } u;
	for (; n < count; n++) {
		float val = MAX(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MAX(val, srcs[o][n]);
		}
		u.f = val;
		d[n] = MAX_FLOAT_TOTREE(u.u);
	}
}

void _core_fp32_max_unmarshall(float *dst, const float *src, int count) {
  _core_fp32_post_max(dst, src, count);
}

void _core_fp32_int32_maxloc_marshall(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	uint32_uint32_t *d = (uint32_uint32_t *)dst;
	union { float f; uint32_t u; } u;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 > src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a < srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		u.f = srcs[m][n].a;
		d[n].a = MAX_FLOAT_TOTREE(u.u);
		d[n].b = ~srcs[m][n].b;
	}
}

void _core_fp32_int32_maxloc_unmarshall(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  _core_fp32_int32_post_maxloc(dst, src, count);
}

void _core_fp32_fp32_maxloc_marshall(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	uint32_uint32_t *d = (uint32_uint32_t *)dst;
	union { float f; uint32_t u; } u;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 > src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a < srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		u.f = srcs[m][n].a;
		d[n].a = MAX_FLOAT_TOTREE(u.u);
		u.f = srcs[m][n].b;
		d[n].b = MIN_FLOAT_TOTREE(u.u);
	}
}

void _core_fp32_fp32_maxloc_unmarshall(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  _core_fp32_fp32_post_maxloc(dst, src, count);
}

void _core_fp32_min_marshall(float *dst, const float **srcs, int nsrc, int count) {
	register int n = 0, o;
	uint32_t *d=(uint32_t *)dst;
	union { float f; uint32_t u; } u;
	for (; n < count; n++) {
		float val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		u.f = val;
		d[n] = MIN_FLOAT_TOTREE(u.u);
	}
}

void _core_fp32_min_unmarshall(float *dst, const float *src, int count) {
  _core_fp32_post_min(dst, src, count);
}

void _core_fp32_int32_minloc_marshall(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	uint32_uint32_t *d = (uint32_uint32_t *)dst;
	union { float f; uint32_t u; } u;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 < src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a > srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		u.f = srcs[m][n].a;
		d[n].a = MIN_FLOAT_TOTREE(u.u);
		d[n].b = ~srcs[m][n].b;
	}
}

void _core_fp32_int32_minloc_unmarshall(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  _core_fp32_int32_post_minloc(dst, src, count);
}

void _core_fp32_fp32_minloc_marshall(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	uint32_uint32_t *d = (uint32_uint32_t *)dst;
	union { float f; uint32_t u; } u;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 < src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a > srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		u.f = srcs[m][n].a;
		d[n].a = MIN_FLOAT_TOTREE(u.u);
		u.f = srcs[m][n].b;
		d[n].b = MIN_FLOAT_TOTREE(u.u);
	}
}

void _core_fp32_fp32_minloc_unmarshall(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  _core_fp32_fp32_post_minloc(dst, src, count);
}
