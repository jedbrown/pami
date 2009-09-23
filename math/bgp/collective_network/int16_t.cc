/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int16.cc
 * \brief Default C math routines for 16 bit signed integer operations.
 */

#include "dcmf_bg_math.h"
#include "Util.h"
#include "internal.h"

static void _core_int16_conv(uint16_t *dst, const int16_t *src, int count) {
#define OP(a) ((a)+(0x8000))

#define TYPE uint16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

static void _core_int16_conv_not(uint16_t *dst, const int16_t *src, int count) {
#define OP(a) (~((a)+(0x8000)))

#define TYPE uint16_t
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

static void _core_int16_unconv(int16_t *dst, const uint16_t *src, int count) {
#define OP(a) ((a)-(0x8000))

#define TYPE int16_t 
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

static void _core_int16_unconv_not(int16_t *dst, const uint16_t *src, int count) {
#define OP(a) (~((a)-(0x8000)))

#define TYPE int16_t 
#include "_single_src.x.h"
#undef TYPE
#undef OP
}

void _core_int16_pre_all(uint16_t *dst, const int16_t *src, int count) {
  _core_int16_conv(dst, src, count);
}

void _core_int16_post_all(int16_t *dst, const uint16_t *src, int count) {
  _core_int16_unconv(dst, src, count);
}

void _core_int16_pre_min(uint16_t *dst, const int16_t *src, int count) {
  _core_int16_conv_not(dst, src, count);
}

void _core_int16_post_min(int16_t *dst, const uint16_t *src, int count) {
  _core_int16_unconv_not(dst, src, count);
}

void _core_int16_int32_pre_maxloc(uint16_int32_t *dst, const int16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = src[n].a + shift;
      dst[n].z = 0;
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int16_int32_post_maxloc(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = src[n].a - shift;
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}

void _core_int16_int32_pre_minloc(uint16_int32_t *dst, const int16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(src[n].a + shift);
      dst[n].z = 0;
      dst[n].b = LOC_INT_TOTREE(src[n].b);
    }
}

void _core_int16_int32_post_minloc(int16_int32_t *dst, const uint16_int32_t *src, int count) {
  register int n = 0;
  register unsigned shift = 0x8000UL;
  for (n = 0; n < count; n++)
    {
      dst[n].a = ~(src[n].a - shift);
      dst[n].b = LOC_INT_FRTREE(src[n].b);
    }
}
