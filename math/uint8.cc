/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint8.cc
 * \brief Default C math routines for 8 bit unsigned integer operations.
 */

#include "math_coremath.h"
#include "internal.h"

void _xmi_core_uint8_band(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint8_bor(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint8_bxor(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint8_land(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint8_lor(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint8_lxor(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint8_max(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint8_min(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint8_prod(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint8_sum(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE uint8_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}
