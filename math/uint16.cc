/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint16.cc
 * \brief Default C math routines for 16 bit unsigned integer operations.
 */

#include "math_coremath.h"
#include "internal.h"


void _xmi_core_uint16_band(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint16_bor(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint16_bxor(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint16_land(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint16_lor(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint16_lxor(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint16_max(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint16_min(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint16_prod(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint16_sum(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE uint16_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}
