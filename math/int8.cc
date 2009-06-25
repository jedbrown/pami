/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int8.cc
 * \brief Default C math routines for 8 bit signed integer operations.
 */

#include "math_coremath.h"
#include "internal.h"

void _core_int8_band(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_bor(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_bxor(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_land(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_lor(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_lxor(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_max(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_min(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_prod(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_int8_sum(int8_t *dst, const int8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE int8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}
