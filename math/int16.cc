/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int16.cc
 * \brief Default C math routines for 16 bit signed integer operations.
 */

#include "math_coremath.h"
#include "internal.h"


void _xmi_core_int16_band(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_bor(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_bxor(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_land(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_lor(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_lxor(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_max(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_min(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_prod(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_sum(int16_t *dst, const int16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE int16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int16_int32_maxloc(int16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
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

void _xmi_core_int16_int32_minloc(int16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
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
