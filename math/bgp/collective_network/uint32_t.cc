/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint32.cc
 * \brief Default C math routines for 32 bit unsigned integer operations.
 */

#include "xmi_bg_math.h"
#include "util/common.h"
//#include "internal.h"

static void _xmi_core_uint32_not(uint32_t *dst, const uint32_t *src, int count) {
#define OP(a) (~(a))

#define TYPE uint32_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

void _xmi_core_uint32_pre_all(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _xmi_core_uint32_post_all(uint32_t *dst, const uint32_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint32_t));
}

void _xmi_core_uint32_pre_min(uint32_t *dst, const uint32_t *src, int count) {
  _xmi_core_uint32_not(dst, src, count);
}

void _xmi_core_uint32_post_min(uint32_t *dst, const uint32_t *src, int count) {
  _xmi_core_uint32_not(dst, src, count);
}
