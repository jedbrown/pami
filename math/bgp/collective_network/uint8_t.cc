/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/uint8.cc
 * \brief Default C math routines for 8 bit unsigned integer operations.
 */

#include "dcmf_bg_math.h"
#include "Util.h"
#include "internal.h"

static void _core_uint8_not(uint8_t *dst, const uint8_t *src, int count) {
#define OP(a) (~(a))

#define TYPE uint8_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_uint8_pre_all(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_post_all(uint8_t *dst, const uint8_t *src, int count) {
  Core_memcpy((void *)dst, (const void *)src, (size_t)count);
}

void _core_uint8_pre_min(uint8_t *dst, const uint8_t *src, int count) {
  _core_uint8_not(dst, src, count);
}

void _core_uint8_post_min(uint8_t *dst, const uint8_t *src, int count) {
  _core_uint8_not(dst, src, count);
}
