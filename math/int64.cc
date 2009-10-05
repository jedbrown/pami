/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int64.cc
 * \brief Default C math routines for 32 bit signed integer operations.
 */

#include "math_coremath.h"
#include "internal.h"


void _xmi_core_int64_band(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_bor(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_bxor(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_land(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_lor(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_lxor(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_max(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_min(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_prod(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_sum(int64_t *dst, const int64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE int64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}
