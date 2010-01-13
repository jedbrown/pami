/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file math/fp128.cc
 * \brief Default C math routines for 128 bit floating point operations.
 */

#include "math_coremath.h"
#include "internal.h"

void _xmi_core_fp128_max(long double *dst, const long double **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE long double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_fp128_min(long double *dst, const long double **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE long double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_fp128_prod(long double *dst, const long double **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE long double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_fp128_sum(long double *dst, const long double **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE long double
#include "_N_src.x.h"
#undef TYPE
#undef OP
}
