/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file math/fp64.cc
 * \brief Default C math routines for 64 bit floating point operations.
 */

#include "math_coremath.h"
#include "FloatUtil.h"
#include "internal.h"

void _pami_core_fp64_max(double *dst, const double **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_fp64_min(double *dst, const double **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_fp64_prod(double *dst, const double **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_fp64_sum(double *dst, const double **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_fp64_int32_maxloc(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
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

void _pami_core_fp64_fp64_maxloc(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
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

void _pami_core_fp64_int32_minloc(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
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

void _pami_core_fp64_fp64_minloc(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
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
