/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/fp128_o.cc
 * \brief Optimized math routines for 128 bit floating point operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "math_coremath.h"
#include "FloatUtil.h"
#include "Util.h"
#include "ppc450d/internal_o.h"

void _xmi_core_fp128_max2(long double *dst, const long double **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE long double
#include "../_dual_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_fp128_min2(long double *dst, const long double **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE long double
#include "../_dual_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_fp128_prod2(long double *dst, const long double **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE long double
#include "../_dual_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_fp128_sum2(long double *dst, const long double **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE long double
#include "../_dual_src.x.h"
#undef TYPE
#undef OP
}
