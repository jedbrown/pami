/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int64.cc
 * \brief Default C math routines for 32 bit signed integer operations.
 */

#include "dcmf_bg_math.h"
#include "Util.h"
#include "internal.h"

static void _core_int64_conv(uint64_t *dst, const int64_t *src, int count) {
#define OP(a) ((a)+(0x8000000000000000ULL))

#define TYPE uint64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

static void _core_int64_conv_not(uint64_t *dst, const int64_t *src, int count) {
#define OP(a) (~((a)+(0x8000000000000000ULL)))

#define TYPE uint64_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

static void _core_int64_unconv(int64_t *dst, const uint64_t *src, int count) {
#define OP(a) ((a)-(0x8000000000000000ULL))

#define TYPE int64_t 
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

static void _core_int64_unconv_not(int64_t *dst, const uint64_t *src, int count) {
#define OP(a) (~((a)-(0x8000000000000000ULL)))

#define TYPE int64_t 
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int64_pre_all(uint64_t *dst, const int64_t *src, int count) {
  _core_int64_conv(dst, src, count);
}

void _core_int64_post_all(int64_t *dst, const uint64_t *src, int count) {
  _core_int64_unconv(dst, src, count);
}

void _core_int64_pre_min(uint64_t *dst, const int64_t *src, int count) {
  _core_int64_conv_not(dst, src, count);
}

void _core_int64_post_min(int64_t *dst, const uint64_t *src, int count) {
  _core_int64_unconv_not(dst, src, count);
}
