/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint32.cc
 * \brief Default C math routines for 32 bit unsigned integer operations.
 */

#include "math_coremath.h"
#include "internal.h"

void _core_uint32_band(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_bor(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_bxor(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_land(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_lor(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_lxor(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_max(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_min(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_prod(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint32_sum(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE uint32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}
