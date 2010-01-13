/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/bgp/collective_network/uint64_ot.cc
 * \brief Optimized math routines for unsigned 64 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "xmi_bg_math.h"
#include "util/common.h"
//#include "ppc450d/internal_o.h"

static void _xmi_core_uint64_not_o(uint64_t *dst, const uint64_t *src, int count) {
  _xmi_core_uint32_not_o((uint32_t *)dst, (const uint32_t *)src, count<<1);
}

#ifdef NOT_USED
static void _xmi_core_uint64_conv_o(uint64_t *dst, const uint64_t *src, int count) {
#define OP(a) ((a)-(0x8000000000000000ULL))

#define TYPE uint64_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

static void _xmi_core_uint64_conv_not_o(uint64_t *dst, const uint64_t *src, int count) {
#define OP(a) (~((a)-(0x8000000000000000ULL)))

#define TYPE uint64_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}
#endif /* NOT_USED */

void _xmi_core_uint64_pre_min_o(uint64_t *dst, const uint64_t *src, int count) {
  _xmi_core_uint64_not_o(dst, src, count);
}

void _xmi_core_uint64_post_min_o(uint64_t *dst, const uint64_t *src, int count) {
  _xmi_core_uint64_not_o(dst, src, count);
}
