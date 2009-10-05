/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/int64_o.cc
 * \brief Optimized math routines for signed 64 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "xmi_bg_math.h"
#include "util/common.h"
//#include "ppc450d/internal_o.h"

static void _xmi_core_int64_conv_o(uint64_t *dst, const int64_t *src, int count) {
#define OP(a) ((a)+(0x8000000000000000ULL))

#define TYPE uint64_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

static void _xmi_core_int64_conv_not_o(uint64_t *dst, const int64_t *src, int count) {
#define OP(a) (~((a)+(0x8000000000000000ULL)))

#define TYPE uint64_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

static void _xmi_core_int64_unconv_o(int64_t *dst, const uint64_t *src, int count) {
#define OP(a) ((a)-(0x8000000000000000ULL))

#define TYPE uint64_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

static void _xmi_core_int64_unconv_not_o(int64_t *dst, const uint64_t *src, int count) {
#define OP(a) (~((a)-(0x8000000000000000ULL)))

#define TYPE uint64_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_int64_pre_all_o(uint64_t *dst, const int64_t *src, int count) {
  _xmi_core_int64_conv_o(dst, src, count);
}

void _xmi_core_int64_post_all_o(int64_t *dst, const uint64_t *src, int count) {
  _xmi_core_int64_unconv_o(dst, src, count);
}

void _xmi_core_int64_pre_min_o(uint64_t *dst, const int64_t *src, int count) {
  _xmi_core_int64_conv_not_o(dst, src, count);
}

void _xmi_core_int64_post_min_o(int64_t *dst, const uint64_t *src, int count) {
  _xmi_core_int64_unconv_not_o(dst, src, count);
}
