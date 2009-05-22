/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file math/fp64.cc
 * \brief Default C math routines for 64 bit floating point operations.
 */

#include "math_bg_math.h"
#include "FloatUtil.h"
#include "internal.h"

// should be getting from tree/Device.h
#define TREE_PKT_SIZE	256

extern "C" uint64_t FU_MAX_DOUBLE_TOTREE(double f) {
	union { double f; uint64_t u; } x;
	x.f = f;
	return MAX_DOUBLE_TOTREE(x.u);
}
extern "C" uint64_t FU_MAX_DOUBLE_FRTREE(double f) {
	union { double f; uint64_t u; } x;
	x.f = f;
	return MAX_DOUBLE_FRTREE(x.u);
}
extern "C" uint64_t FU_MIN_DOUBLE_TOTREE(double f) {
	union { double f; uint64_t u; } x;
	x.f = f;
	return MIN_DOUBLE_TOTREE(x.u);
}
extern "C" uint64_t FU_MIN_DOUBLE_FRTREE(double f) {
	union { double f; uint64_t u; } x;
	x.f = f;
	return MIN_DOUBLE_FRTREE(x.u);
}

//extern "C" int _g_num_active_nodes;
int _g_num_active_nodes;

void _core_fp64_max(double *dst, const double **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp64_min(double *dst, const double **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp64_prod(double *dst, const double **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp64_sum(double *dst, const double **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp64_int32_maxloc(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
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

void _core_fp64_fp64_maxloc(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
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

void _core_fp64_int32_minloc(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
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

void _core_fp64_fp64_minloc(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
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

void _core_fp64_pre_all(double *dst, const double *src, int count) {
  MATH_abort();
}

void _core_fp64_post_all(double *dst, const double *src, int count) {
  MATH_abort();
}

void _core_fp64_pre_sum(double *dst, const double *src, int count) {
	int c = 0;
	double *s = (double *)src;
	struct b { char b[TREE_PKT_SIZE]; } *d = (struct b *)dst;
	for (c = 0; c < count; c++) {
		double2uint(s, PKTWORDS, (uint64_t *)d);
		++s;
		++d;
	}
}

void _core_fp64_post_sum(double *dst, const double *src, int count) {
	int c = 0;
	struct b { char b[TREE_PKT_SIZE]; } *s = (struct b *)src;
	double *d = dst;
	for (c = 0; c < count; c++) {
		uint2double(_g_num_active_nodes, d, PKTWORDS, (uint64_t *)s);
		++s;
		++d;
	}
}

void _core_fp64_pre_max(double *dst, const double *src, int count) {
  uint64_t *d=(uint64_t*)dst, *s=(uint64_t*)src;
  register int n=0;
  register unsigned long long d0, d1, d2, d3;
  register unsigned long long s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MAX_DOUBLE_TOTREE(s0);
      d1 = MAX_DOUBLE_TOTREE(s1);
      d2 = MAX_DOUBLE_TOTREE(s2);
      d3 = MAX_DOUBLE_TOTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MAX_DOUBLE_TOTREE(s0);
      d[n] = d0;
    }
}

void _core_fp64_post_max(double *dst, const double *src, int count) {
  uint64_t *d=(uint64_t*)dst, *s=(uint64_t*)src;
  register int n=0;
  register unsigned long long d0, d1, d2, d3;
  register unsigned long long s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MAX_DOUBLE_FRTREE(s0);
      d1 = MAX_DOUBLE_FRTREE(s1);
      d2 = MAX_DOUBLE_FRTREE(s2);
      d3 = MAX_DOUBLE_FRTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MAX_DOUBLE_FRTREE(s0);
      d[n] = d0;
    }
}

void _core_fp64_int32_pre_maxloc(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s=(uint64_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_DOUBLE_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
      d[n].z = 0;
    }
}

void _core_fp64_int32_post_maxloc(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s=(uint64_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_DOUBLE_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _core_fp64_fp64_pre_maxloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s=(uint64_uint64_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_DOUBLE_TOTREE(s[n].a);
      d[n].b = MIN_DOUBLE_TOTREE(s[n].b);
    }
}

void _core_fp64_fp64_post_maxloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s=(uint64_uint64_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_DOUBLE_FRTREE(s[n].a);
      d[n].b = MIN_DOUBLE_FRTREE(s[n].b);
    }
}

void _core_fp64_pre_min(double *dst, const double *src, int count) {
  uint64_t *d=(uint64_t*)dst, *s=(uint64_t*)src;
  register int n=0;
  register unsigned long long d0, d1, d2, d3;
  register unsigned long long s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MIN_DOUBLE_TOTREE(s0);
      d1 = MIN_DOUBLE_TOTREE(s1);
      d2 = MIN_DOUBLE_TOTREE(s2);
      d3 = MIN_DOUBLE_TOTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MIN_DOUBLE_TOTREE(s0);
      d[n] = d0;
    }
}

void _core_fp64_post_min(double *dst, const double *src, int count) {
  uint64_t *d=(uint64_t*)dst, *s=(uint64_t*)src;
  register int n=0;
  register unsigned long long d0, d1, d2, d3;
  register unsigned long long s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MIN_DOUBLE_FRTREE(s0);
      d1 = MIN_DOUBLE_FRTREE(s1);
      d2 = MIN_DOUBLE_FRTREE(s2);
      d3 = MIN_DOUBLE_FRTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MIN_DOUBLE_FRTREE(s0);
      d[n] = d0;
    }
}

void _core_fp64_int32_pre_minloc(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s=(uint64_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_DOUBLE_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
      d[n].z = 0;
    }
}

void _core_fp64_int32_post_minloc(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s=(uint64_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_DOUBLE_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _core_fp64_fp64_pre_minloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s=(uint64_uint64_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_DOUBLE_TOTREE(s[n].a);
      d[n].b = MIN_DOUBLE_TOTREE(s[n].b);
    }
}

void _core_fp64_fp64_post_minloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s=(uint64_uint64_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_DOUBLE_FRTREE(s[n].a);
      d[n].b = MIN_DOUBLE_FRTREE(s[n].b);
    }
}

void _core_fp64_max_marshall(double *dst, const double **srcs, int nsrc, int count) {
	register int n = 0, o;
	uint64_t *d = (uint64_t *)dst;
	union { double f; uint64_t u; } x;
	for (; n < count; n++) {
		double val = MAX(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MAX(val, srcs[o][n]);
		}
		x.f = val;
		d[n] = MAX_DOUBLE_TOTREE(x.u);
	}
}

void _core_fp64_max_unmarshall(double *dst, const double *src, int count) {
  _core_fp64_post_max(dst, src, count);
}

void _core_fp64_int32_maxloc_marshall(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	union { double f; uint64_t u; } x;
	uint64_int32_t *d = (uint64_int32_t*)dst;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 > src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a < srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		x.f = srcs[m][n].a;
		d[n].a = MAX_DOUBLE_TOTREE(x.u);
		d[n].b = ~srcs[m][n].b;
	}
}

void _core_fp64_int32_maxloc_unmarshall(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  _core_fp64_int32_post_maxloc(dst, src, count);
}

void _core_fp64_fp64_maxloc_marshall(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	union { double f; uint64_t u; } x;
	uint64_uint64_t *d = (uint64_uint64_t*)dst;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 > src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a < srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		x.f = srcs[m][n].a;
		d[n].a = MAX_DOUBLE_TOTREE(x.u);
		x.f = srcs[m][n].b;
		d[n].b = MIN_DOUBLE_TOTREE(x.u);
	}
}

void _core_fp64_fp64_maxloc_unmarshall(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  _core_fp64_fp64_post_maxloc(dst, src, count);
}

void _core_fp64_min_marshall(double *dst, const double **srcs, int nsrc, int count) {
	register int n = 0, o;
	uint64_t *d = (uint64_t *)dst;
	union { double f; uint64_t u; } x;
	for (; n < count; n++) {
		double val = MIN(srcs[0][n], srcs[1][n]);
		for (o = 2; o < nsrc; ++o) {
			val = MIN(val, srcs[o][n]);
		}
		x.f = val;
		d[n] = MIN_DOUBLE_TOTREE(x.u);
	}
}

void _core_fp64_min_unmarshall(double *dst, const double *src, int count) {
  _core_fp64_post_min(dst, src, count);
}

void _core_fp64_int32_minloc_marshall(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	union { double f; uint64_t u; } x;
	uint64_int32_t *d = (uint64_int32_t*)dst;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 < src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a > srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		x.f = srcs[m][n].a;
		d[n].a = MIN_DOUBLE_TOTREE(x.u);
		d[n].b = ~srcs[m][n].b;
	}
}

void _core_fp64_int32_minloc_unmarshall(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  _core_fp64_int32_post_minloc(dst, src, count);
}

void _core_fp64_fp64_minloc_marshall(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
	register int n = 0, m, o;
	union { double f; uint64_t u; } x;
	uint64_uint64_t *d = (uint64_uint64_t*)dst;
	for (n = 0; n < count; n++) {
		m = 0;  // assume src0 < src1
		for (o = 1; o < nsrc; ++o) {
			if (srcs[m][n].a > srcs[o][n].a ||
			    (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
				m = o;
			}
		}
		x.f = srcs[m][n].a;
		d[n].a = MIN_DOUBLE_TOTREE(x.u);
		x.f = srcs[m][n].b;
		d[n].b = MIN_DOUBLE_TOTREE(x.u);
	}
}

void _core_fp64_fp64_minloc_unmarshall(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  _core_fp64_fp64_post_minloc(dst, src, count);
}
