/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file math/bgp/collective_network/fp32_t.cc
 * \brief Default C math routines for 32 bit floating point operations.
 */

#include "xmi_bg_math.h"
#include "CNUtil.h"
#include "util/common.h"
//#include "internal.h"
#include "math/FloatUtil.h"

void _xmi_core_fp32_pre_max(float *dst, const float *src, int count) {
  uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
  register int n=0;
  register unsigned d0, d1, d2, d3;
  register unsigned s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MAX_FLOAT_TOTREE(s0);
      d1 = MAX_FLOAT_TOTREE(s1);
      d2 = MAX_FLOAT_TOTREE(s2);
      d3 = MAX_FLOAT_TOTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MAX_FLOAT_TOTREE(s0);
      d[n] = d0;
    }
}

void _xmi_core_fp32_post_max(float *dst, const float *src, int count) {
  uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
  register int n=0;
  register unsigned d0, d1, d2, d3;
  register unsigned s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MAX_FLOAT_FRTREE(s0);
      d1 = MAX_FLOAT_FRTREE(s1);
      d2 = MAX_FLOAT_FRTREE(s2);
      d3 = MAX_FLOAT_FRTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MAX_FLOAT_FRTREE(s0);
      d[n] = d0;
    }
}

void _xmi_core_fp32_int32_pre_maxloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
    }
}

void _xmi_core_fp32_int32_post_maxloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _xmi_core_fp32_fp32_pre_maxloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_TOTREE(s[n].a);
      d[n].b = MIN_FLOAT_TOTREE(s[n].b);
    }
}

void _xmi_core_fp32_fp32_post_maxloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_FLOAT_FRTREE(s[n].a);
      d[n].b = MIN_FLOAT_FRTREE(s[n].b);
    }
}

void _xmi_core_fp32_pre_min(float *dst, const float *src, int count) {
  uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
  register int n=0;
  register unsigned d0, d1, d2, d3;
  register unsigned s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MIN_FLOAT_TOTREE(s0);
      d1 = MIN_FLOAT_TOTREE(s1);
      d2 = MIN_FLOAT_TOTREE(s2);
      d3 = MIN_FLOAT_TOTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MIN_FLOAT_TOTREE(s0);
      d[n] = d0;
    }
}

void _xmi_core_fp32_post_min(float *dst, const float *src, int count) {
  uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
  register int n=0;
  register unsigned d0, d1, d2, d3;
  register unsigned s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MIN_FLOAT_FRTREE(s0);
      d1 = MIN_FLOAT_FRTREE(s1);
      d2 = MIN_FLOAT_FRTREE(s2);
      d3 = MIN_FLOAT_FRTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MIN_FLOAT_FRTREE(s0);
      d[n] = d0;
    }
}

void _xmi_core_fp32_int32_pre_minloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
    }
}

void _xmi_core_fp32_int32_post_minloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
  register int n = 0;
  uint32_int32_t *d=(uint32_int32_t*)dst, *s=(uint32_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _xmi_core_fp32_fp32_pre_minloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_TOTREE(s[n].a);
      d[n].b = MIN_FLOAT_TOTREE(s[n].b);
    }
}

void _xmi_core_fp32_fp32_post_minloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
  register int n = 0;
  uint32_uint32_t *d=(uint32_uint32_t*)dst, *s=(uint32_uint32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_FLOAT_FRTREE(s[n].a);
      d[n].b = MIN_FLOAT_FRTREE(s[n].b);
    }
}
