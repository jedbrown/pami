/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint64.cc
 * \brief Default C math routines for 64 bit unsigned integer operations.
 */

#include "xmi_bg_math.h"
#include "util/common.h"
//#include "internal.h"

static void _xmi_core_uint64_not(uint64_t *dst, const uint64_t *src, int count) {
#define OP(a) (~(a))

#define TYPE uint64_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint64_pre_all(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _xmi_core_uint64_post_all(uint64_t *dst, const uint64_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint64_t));
}

void _xmi_core_uint64_pre_min(uint64_t *dst, const uint64_t *src, int count) {
  _xmi_core_uint64_not(dst, src, count);
}

void _xmi_core_uint64_post_min(uint64_t *dst, const uint64_t *src, int count) {
  _xmi_core_uint64_not(dst, src, count);
}
