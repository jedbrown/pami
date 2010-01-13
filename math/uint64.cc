/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint64.cc
 * \brief Default C math routines for 64 bit unsigned integer operations.
 */

#include "math_coremath.h"
#include "internal.h"

void _xmi_core_uint64_band(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_bor(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_bxor(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_land(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_lor(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_lxor(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_max(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_min(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_prod(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_sum(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE uint64_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}
