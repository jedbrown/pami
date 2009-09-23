/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint16.cc
 * \brief Default C math routines for 16 bit unsigned integer operations.
 */

#include "dcmf_bg_math.h"
#include "Util.h"
#include "internal.h"

static void _core_uint16_not(uint16_t *dst, const uint16_t *src, int count) {
#define OP(a) (~(a))

#define TYPE uint16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint16_pre_all(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_post_all(uint16_t *dst, const uint16_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count * sizeof(uint16_t));
}

void _core_uint16_pre_min(uint16_t *dst, const uint16_t *src, int count) {
  _core_uint16_not(dst, src, count);
}

void _core_uint16_post_min(uint16_t *dst, const uint16_t *src, int count) {
  _core_uint16_not(dst, src, count);
}
