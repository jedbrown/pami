/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/ppc450d/fp64_o.cc
 * \brief Optimized math routines for 64 bit floating point operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "dcmf_bg_math.h"
#include "FloatUtil.h"
#include "Util.h"
#include "multisend/bgp/tree/DblUtils.h"
#include "ppc450d/internal_o.h"

void _core_fp64_sum2(double *dst, const double **srcs, int nsrc, int count) {
	if (count < 16) {
		// This seems necessary for good latency,
		// but its not as good as the unoptimized compile.
#define OP(a,b)	((a) + (b))
#define TYPE	double
#include "_dual_src.x.h"
#undef OP
#undef TYPE
		return;
	}
	int size128 = count >> 4;
	int remainder = count & 0xf;
	const double *f0 = srcs[0];
	const double *f1 = srcs[1];
	double *f2 = dst;
	if (size128 > 0) {
#define OP2(a,b)	(a) += (b)
#define OP3(a,b,c)	asm volatile ("fpadd %0, %1, %2" : "=f"(a) : "f"(b), "f"(c))
#include "ppc450d/_optim_fp64_dual_src.x.h"
#undef OP2
#undef OP3
	}
	int n;
	for (n = 0; n < remainder; ++n) {
		f2[n] = f0[n] + f1[n];
	}
	return;
}

void _core_fp64_max2(double *dst, const double **srcs, int nsrc, int count) {
	if (count < 16) {
		// This seems necessary for good latency,
		// but its not as good as the unoptimized compile.
#define OP(a,b) (((a)>(b))?(a):(b))
#define TYPE	double
#include "_dual_src.x.h"
#undef OP
#undef TYPE
		return;
	}
	int size128 = count >> 4;
	int remainder = count & 0xf;
	const double *f0 = srcs[0];
	const double *f1 = srcs[1];
	double *f2 = dst;
	if (size128 > 0) {
#define OP2(a,b)  a=(((a)>(b))?(a):(b)) 
#define OP3(a,b,c)	asm volatile ("fpsub %0, %1, %2" : "=f"(a) : "f"(b), "f"(c)); \
	                asm volatile ("fpsel %0, %1, %2, %3" : "=f"(a) : "f"(a), "f"(b), "f"(c))
#include "ppc450d/_optim_fp64_dual_src.x.h"
#undef OP2
#undef OP3
	}
	int n;
	for (n = 0; n < remainder; ++n) {
	  f2[n] = (((f0[n])>(f1[n]))?(f0[n]):(f1[n])); 
	}
	return;
}

void _core_fp64_min2(double *dst, const double **srcs, int nsrc, int count) {
	if (count < 16) {
		// This seems necessary for good latency,
		// but its not as good as the unoptimized compile.
#define OP(a,b) (((a)>(b))?(b):(a))
#define TYPE	double
#include "_dual_src.x.h"
#undef OP
#undef TYPE
		return;
	}
	int size128 = count >> 4;
	int remainder = count & 0xf;
	const double *f0 = srcs[0];
	const double *f1 = srcs[1];
	double *f2 = dst;
	if (size128 > 0) {
#define OP2(a,b)  a=(((a)>(b))?(b):(a)) 
#define OP3(a,b,c)	asm volatile ("fpsub %0, %1, %2" : "=f"(a) : "f"(b), "f"(c)); \
	                asm volatile ("fpsel %0, %1, %2, %3" : "=f"(a) : "f"(a), "f"(c), "f"(b))
#include "ppc450d/_optim_fp64_dual_src.x.h"
#undef OP2
#undef OP3
	}
	int n;
	for (n = 0; n < remainder; ++n) {
	  f2[n] = (((f0[n])>(f1[n]))?(f1[n]):(f0[n])); 
	}
	return;  
}


void _core_fp64_prod2(double *dst, const double **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE double
#include "_dual_src.x.h"
#undef TYPE
#undef OP
}

void _core_fp64_int32_maxloc2(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
  const fp64_int32_t *s0 = srcs[0], *s1 = srcs[1];
  register int n = 0;
  for (n = 0; n < count; n++)
    {
      if(s0[n].a > s1[n].a)
        dst[n].b = s0[n].b;
      else if(s0[n].a < s1[n].a)
        dst[n].b = s1[n].b;
      else
        dst[n].b = MIN(s0[n].b, s1[n].b);
      dst[n].a = MAX(s0[n].a,s1[n].a);
    }
}

void _core_fp64_fp64_maxloc2(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
  const fp64_fp64_t *s0 = srcs[0], *s1 = srcs[1];
  register int n = 0;
  for (n = 0; n < count; n++)
    {
      if(s0[n].a > s1[n].a)
        dst[n].b = s0[n].b;
      else if(s0[n].a < s1[n].a)
        dst[n].b = s1[n].b;
      else
        dst[n].b = MIN(s0[n].b, s1[n].b);
      dst[n].a = MAX(s0[n].a,s1[n].a);
    }
}

void _core_fp64_int32_minloc2(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
  const fp64_int32_t *s0 = srcs[0], *s1 = srcs[1];
  register int n = 0;
  for (n = 0; n < count; n++)
    {
      if(s0[n].a < s1[n].a)
        dst[n].b = s0[n].b;
      else if(s0[n].a > s1[n].a)
        dst[n].b = s1[n].b;
      else
        dst[n].b = MIN(s0[n].b, s1[n].b);
      dst[n].a = MIN(s0[n].a,s1[n].a);
    }
}

void _core_fp64_fp64_minloc2(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
  const fp64_fp64_t *s0 = srcs[0], *s1 = srcs[1];
  register int n = 0;
  for (n = 0; n < count; n++)
    {
      if(s0[n].a < s1[n].a)
        dst[n].b = s0[n].b;
      else if(s0[n].a > s1[n].a)
        dst[n].b = s1[n].b;
      else
        dst[n].b = MIN(s0[n].b, s1[n].b);
      dst[n].a = MIN(s0[n].a,s1[n].a);
    }
}

void _core_fp64_pre_max_o(double *dst, const double *src, int count) {
  uint64_t *d=(uint64_t*)dst, *s=(uint64_t*)src;
  register int n=0;
  register unsigned long long d0, d1, d2, d3;
  register unsigned long long s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MAX_DOUBLE_TOTREE(s0);
      d1 = MAX_DOUBLE_TOTREE(s1);
      d2 = MAX_DOUBLE_TOTREE(s2);
      d3 = MAX_DOUBLE_TOTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MAX_DOUBLE_TOTREE(s0);
      d[n] = d0;
    }
}

void _core_fp64_post_max_o(double *dst, const double *src, int count) {
  uint64_t *d=(uint64_t*)dst, *s=(uint64_t*)src;
  register int n=0;
  register unsigned long long d0, d1, d2, d3;
  register unsigned long long s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MAX_DOUBLE_FRTREE(s0);
      d1 = MAX_DOUBLE_FRTREE(s1);
      d2 = MAX_DOUBLE_FRTREE(s2);
      d3 = MAX_DOUBLE_FRTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MAX_DOUBLE_FRTREE(s0);
      d[n] = d0;
    }
}

void _core_fp64_int32_pre_maxloc_o(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s=(uint64_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_DOUBLE_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
      d[n].z = 0;
    }
}

void _core_fp64_int32_post_maxloc_o(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s=(uint64_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_DOUBLE_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _core_fp64_fp64_pre_maxloc_o(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s=(uint64_uint64_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_DOUBLE_TOTREE(s[n].a);
      d[n].b = MIN_DOUBLE_TOTREE(s[n].b);
    }
}

void _core_fp64_fp64_post_maxloc_o(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s=(uint64_uint64_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MAX_DOUBLE_FRTREE(s[n].a);
      d[n].b = MIN_DOUBLE_FRTREE(s[n].b);
    }
}

void _core_fp64_pre_min_o(double *dst, const double *src, int count) {
  uint64_t *d=(uint64_t*)dst, *s=(uint64_t*)src;
  register int n=0;
  register unsigned long long d0, d1, d2, d3;
  register unsigned long long s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MIN_DOUBLE_TOTREE(s0);
      d1 = MIN_DOUBLE_TOTREE(s1);
      d2 = MIN_DOUBLE_TOTREE(s2);
      d3 = MIN_DOUBLE_TOTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MIN_DOUBLE_TOTREE(s0);
      d[n] = d0;
    }
}

void _core_fp64_post_min_o(double *dst, const double *src, int count) {
  uint64_t *d=(uint64_t*)dst, *s=(uint64_t*)src;
  register int n=0;
  register unsigned long long d0, d1, d2, d3;
  register unsigned long long s0, s1, s2, s3;
  for(; n<count-3; n+=4)
    {
      s0 = s[n+0];
      s1 = s[n+1];
      s2 = s[n+2];
      s3 = s[n+3];

      d0 = MIN_DOUBLE_FRTREE(s0);
      d1 = MIN_DOUBLE_FRTREE(s1);
      d2 = MIN_DOUBLE_FRTREE(s2);
      d3 = MIN_DOUBLE_FRTREE(s3);

      d[n+0] = d0;
      d[n+1] = d1;
      d[n+2] = d2;
      d[n+3] = d3;
    }
  for(; n<count; n++)
    {
      s0   = s[n];
      d0   = MIN_DOUBLE_FRTREE(s0);
      d[n] = d0;
    }
}

void _core_fp64_int32_pre_minloc_o(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s=(uint64_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_DOUBLE_TOTREE(s[n].a);
      d[n].b = LOC_INT_TOTREE(s[n].b);
      d[n].z = 0;
    }
}

void _core_fp64_int32_post_minloc_o(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s=(uint64_int32_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_DOUBLE_FRTREE(s[n].a);
      d[n].b = LOC_INT_FRTREE(s[n].b);
    }
}

void _core_fp64_fp64_pre_minloc_o(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s=(uint64_uint64_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_DOUBLE_TOTREE(s[n].a);
      d[n].b = MIN_DOUBLE_TOTREE(s[n].b);
    }
}

void _core_fp64_fp64_post_minloc_o(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s=(uint64_uint64_t*)src;
  for (n = 0; n < count; n++)
    {
      d[n].a = MIN_DOUBLE_FRTREE(s[n].a);
      d[n].b = MIN_DOUBLE_FRTREE(s[n].b);
    }
}

void _core_fp64_max_marshall2(double *dst, const double **srcs, int nsrc, int count) {
  register int n=0;
  uint64_t *d=(uint64_t*)dst;
  for(; n<count; n++)
    {
      volatile double       *a =(double *)&srcs[0][n];
      volatile double       *b =(double *)&srcs[1][n];
      volatile double      val = MAX((*a), (*b));
      volatile double    *dval = &val;
      volatile unsigned long long *temp = (unsigned long long *)dval;
      d[n] = MAX_DOUBLE_TOTREE(*temp);
    }
}

void _core_fp64_int32_maxloc_marshall2(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s0=(uint64_int32_t*)srcs[0], *s1=(uint64_int32_t*)srcs[1];
  for (n = 0; n < count; n++)
    {
      register unsigned long long tmp = MAX(s0[n].a,s1[n].a);
      d[n].a = MAX_DOUBLE_TOTREE (tmp);
      if(s0[n].a > s1[n].a)
        d[n].b = s0[n].b;
      else if(s0[n].a < s1[n].a)
        d[n].b = s1[n].b;
      else
        d[n].b = MIN(s0[n].b, s1[n].b);
      d[n].b = ~d[n].b;
    }
}

void _core_fp64_fp64_maxloc_marshall2(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s0=(uint64_uint64_t*)srcs[0], *s1=(uint64_uint64_t*)srcs[1];
  for (n = 0; n < count; n++)
    {
      register unsigned long long tmp0 = MAX(s0[n].a,s1[n].a);
      register unsigned long long tmp1 = MIN(s0[n].b,s1[n].b);
      d[n].a = MAX_DOUBLE_TOTREE (tmp0);
      if(s0[n].a > s1[n].a)
        d[n].b = s0[n].b;
      else if(s0[n].a < s1[n].a)
        d[n].b = s1[n].b;
      else
        d[n].b = MIN(s0[n].b, s1[n].b);
      tmp1 = (unsigned long long)d[n].b;
      d[n].b = MIN_DOUBLE_TOTREE (tmp1);
    }
}

void _core_fp64_min_marshall2(double *dst, const double **srcs, int nsrc, int count) {
  register int n=0;
  uint64_t *d=(uint64_t*)dst;
  for(; n<count; n++)
    {
      volatile double       *a =(double *)&srcs[0][n];
      volatile double       *b =(double *)&srcs[1][n];
      volatile double      val = MIN((*a), (*b));
      volatile double    *dval = &val;
      volatile unsigned long long *temp = (unsigned long long *)dval;
      d[n] = MIN_DOUBLE_TOTREE(*temp);
    }
}

void _core_fp64_int32_minloc_marshall2(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
  register int n = 0;
  uint64_int32_t *d=(uint64_int32_t*)dst, *s0=(uint64_int32_t*)srcs[0], *s1=(uint64_int32_t*)srcs[1];
  for (n = 0; n < count; n++)
    {
      register unsigned long long tmp = MIN(s0[n].a,s1[n].a);
      d[n].a = MIN_DOUBLE_TOTREE (tmp);
      if(s0[n].a < s1[n].a)
        d[n].b = s0[n].b;
      else if(s0[n].a > s1[n].a)
        d[n].b = s1[n].b;
      else
        d[n].b = MIN(s0[n].b, s1[n].b);
      d[n].b = ~d[n].b;
    }
}

void _core_fp64_fp64_minloc_marshall2(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
  register int n = 0;
  uint64_uint64_t *d=(uint64_uint64_t*)dst, *s0=(uint64_uint64_t*)srcs[0], *s1=(uint64_uint64_t*)srcs[1];
  for (n = 0; n < count; n++)
    {
      register unsigned long long tmp0 = MIN(s0[n].a,s1[n].a);
      register unsigned long long tmp1 = MIN(s0[n].b,s1[n].b);
      d[n].a = MIN_DOUBLE_TOTREE (tmp0);
      if(s0[n].a < s1[n].a)
        d[n].b = s0[n].b;
      else if(s0[n].a > s1[n].a)
        d[n].b = s1[n].b;
      else
        d[n].b = MIN(s0[n].b, s1[n].b);
      tmp1 = (unsigned long long)d[n].b;
      d[n].b = MIN_DOUBLE_TOTREE (tmp1);
    }
}

void _core_fp64_max4(double *dst, const double **srcs, int nsrc, int count) {
const double *src0 = srcs[0];
const double *src1 = srcs[1];
const double *src2 = srcs[2];
const double *src3 = srcs[3];
#define OP(a,b,c,d)	MAX(MAX(a,b),MAX(c,d))
#define TYPE		double
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_fp64_min4(double *dst, const double **srcs, int nsrc, int count) {
const double *src0 = srcs[0];
const double *src1 = srcs[1];
const double *src2 = srcs[2];
const double *src3 = srcs[3];
#define OP(a,b,c,d)	MIN(MIN(a,b),MIN(c,d))
#define TYPE		double
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_fp64_prod4(double *dst, const double **srcs, int nsrc, int count) {
const double *src0 = srcs[0];
const double *src1 = srcs[1];
const double *src2 = srcs[2];
const double *src3 = srcs[3];
#define OP(a,b,c,d)	(a*b*c*d)
#define TYPE		double
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

void _core_fp64_sum4(double *dst, const double **srcs, int nsrc, int count) {
const double *src0 = srcs[0];
const double *src1 = srcs[1];
const double *src2 = srcs[2];
const double *src3 = srcs[3];
#define OP(a,b,c,d)	(a+b+c+d)
#define TYPE		double
#include "_quad_src.x.h"
#undef OP
#undef TYPE
	return;
}

/**
 * \brief Code template to split a double into exponent and mantissa
 *
 * \param[in] src	Pointer to doubles to be split
 * \param[out] exp	Where to save exponent
 * \param[out] mp	Where to save 96-bit mantissa
 */
static inline void split_dbl(const double *src, uint32_t *exp, uint32_t *mp) {
	uint32_t *sp = (uint32_t *)src;
	uint32_t mh, ml, x;
	uint32_t neg;

	mh = sp[0];
	ml = sp[1];
	x = (mh >> 20) & 0x000007ffUL;
	neg = ((mh & 0x80000000UL) != 0);
	mh &= 0x000fffff;
	if (x) mh |= 0x00100000UL;
	if (neg) {
		asm volatile (
			"nor %0,%0,%0;"
			"addic %0,%0,1;"
			"nor %1,%1,%1;"
			"addze %1,%1;"
			: "+r"(ml),
			  "+r"(mh)
			);
	}
	mp[0] = mh;
	mp[1] = ml;
	*exp = x;
}

/**
 * dst_e must be count * sizeof(uint16_t) in length.
 * dst_m must be count * 3 * sizeof(uint32_t) in length.
 *
 * Manitssas are not (yet) shifted. That must wait for the MAX(exp).
 *
 * This routine works on exponent-packet units.
 * This routine is not capable of spanning packet boundaries.
 * since that causes more overhead, both here and in caller's code,
 * it is probably OK.
 *
 * It does, however, handle mantissa packet boundaries.
 */
void _core_fp64_pre1_2pass(uint16_t *dst_e, uint32_t *dst_m, const double *src, int count) {
	int n;
	uint32_t x, x2;
	uint32_t *ep = (uint32_t *)dst_e;
	uint32_t *mp = (uint32_t *)dst_m;
	uint32_t align = (unsigned)mp & 0x00ffUL;
	const double *sp = src;

	// DCMF_assert_debug(((unsigned)mp & 0x00ffUL) == 0x0000);
	// NOTE: first 4 bytes of mant pkt are unused,
	//       last 2 bytes of expo pkt are unused...
	n = count >> 1;
	while (n-- > 0) {
		if (((unsigned)mp & 0x00ffUL) == align) {
			++mp;
		}
		split_dbl(sp, &x, mp);
		++sp;
		mp += 3;
		if (((unsigned)mp & 0x00ffUL) == align) {
			++mp;
		}
		split_dbl(sp, &x2, mp);
		++sp;
		mp += 3;
		*ep++ = (x << 16) | x2;
	}
	if (count & 1) {
		if (((unsigned)mp & 0x00ffUL) == align) {
			++mp;
		}
		split_dbl(sp, &x, mp);
		//++sp;
		//mp += 3;
		*((uint16_t *)ep) = x;
	}
}

/**
 * \brief Code template to shift mantissas based on MAX(exponent)
 *
 * \param[in] max_e		Pointer to MAX(exponent)
 * \param[in] expo		Pointer to local exponent
 * \param[in,out] src_dst	Where to load 52-bit mantissa and store 96-bit result
 */
static inline void adj_mant(uint16_t max_e, uint16_t expo, uint32_t *src_dst) {
	uint16_t dx;
	uint32_t *mp = src_dst;
	int32_t mh, ml, mt;

	// NOTE: since we add 6 here, dx will never be zero.
	dx = (max_e + EXPO_PRE_OFFSET) - expo;
	if (dx >= 96) {
		mp[0] = 0;
		mp[1] = 0;
		mp[2] = 0;
	} else if (dx >= 64) {
		dx -= 64;
		mt = mp[0];
		mh = (mt >> 31); // sign only
		mp[0] = mh;
		mp[1] = mh;
		mp[2] = (mt >> dx);
	} else if (dx >= 32) {
		dx -= 32;
		uint32_t umt = mp[1];
		ml = mp[0];
		mp[0] = (ml >> 31); // sign only
		int64_t m = ((int64_t)ml << 32) | umt;
		m = m >> dx;
		mp[1] = (m >> 32) & 0x00000000ffffffffULL;
		mp[2] = m & 0x00000000ffffffffULL;
	} else {
		uint32_t uml = mp[1];
		mt = uml << (32 - dx);
		mh = mp[0];
		uint64_t m = ((((uint64_t)mh << 32) | uml) >> dx);
		ml = m & 0x00000000ffffffffULL;
		mp[0] = (mh >> dx);	// keep sign
		mp[1] = ml;
		mp[2] = mt;
	}
}

/**
 * Shift mantissa into a normalized position, based on MAX(exponent)
 *
 * This routine works on exponent-packet units.
 * This routine is not capable of spanning packet boundaries.
 * since that causes more overhead, both here and in caller's code,
 * it is probably OK.
 *
 * It does, however, handle mantissa packet boundaries.
 *
 * We always are starting at a Mantissa Packet boundary, but the
 * exponents might be mid-packet. They must however be synced.
 */
void _core_fp64_pre2_2pass(uint32_t *dst_src_m, uint16_t *src_e,
				uint16_t *src_ee, int count) {
	uint32_t *oe = (uint32_t *)src_e;
	uint32_t *xe = (uint32_t *)src_ee;
	uint32_t *mp = dst_src_m;
	uint32_t mxe, exp;
	uint32_t align = (unsigned)mp & 0x00ffUL;
	int n;
	// DCMF_assert_debug(((unsigned)mp & 0x00ffUL) == 0x0000);
	// DCMF_assert_debug((((unsigned)oe ^ (unsigned)xe) & 0x00ffUL) == 0x0000);
	n = count >> 1;
	while (n-- > 0) {
		mxe = *xe++;
		exp = *oe++;
		if (((unsigned)mp & 0x00ffUL) == align) {
			++mp;
		}
		adj_mant(mxe >> 16, exp >> 16, mp);
		mp += 3;
		if (((unsigned)mp & 0x00ffUL) == align) {
			++mp;
		}
		adj_mant(mxe & 0x0ffffUL, exp & 0x0ffffUL, mp);
		mp += 3;
	}
	if (count & 1) {
		if (((unsigned)mp & 0x00ffUL) == align) {
			++mp;
		}
		adj_mant(*((uint16_t *)xe), *((uint16_t *)oe), mp);
		// mp += 3;
	}
}

/**
 * \brief Code template to regenerate double from mantissa (SUM) and exponent (MAX)
 *
 * \param[in] mh	Mantissa, hi 32 bits
 * \param[in] ml	Mantissa, mid 32 bits
 * \param[in] mt	Mantissa, lo 32 bits
 * \param[in] ex	Exponent
 * \param[out] dst	Where to store result (double, as two 32-bit ints)
 */
static inline void regen(int32_t mh, int32_t ml, int32_t mt, uint16_t ex, double *dst) {
	uint32_t *dp = (uint32_t *)dst;
	uint32_t neg, dx = 0;
	int32_t nx;

	if (mh | ml | mt) {
		// only normalize if non-zero
		neg = (mh & 0x80000000UL);
		if (neg) {
			asm volatile (
				"nor %0,%0,%0;"
				"addic %0,%0,1;"
				"nor %1,%1,%1;"
				"addze %1,%1;"
				"nor %2,%2,%2;"
				"addze %2,%2;"
				: "+r"(mt),
				  "+r"(ml),
				  "+r"(mh)
				);
		}
		if (ex >= 0x7ff) {
			dp[0] = 0x07ff7000UL;	// NAN
			dp[1] = 0;
			return;
		}
		nx = ex + EXPO_POST_OFFSET;
		if (mh) {
		} else if (ml) {
			mh = ml;
			ml = mt;
			mt = 0;
			nx -= 32;
		} else {
			mh = mt;
			ml = 0;
			mt = 0;
			nx -= 64;
		}
		asm volatile (
			"cntlzw %1,%0"
			: "+r"(mh),
			  "+r"(dx)
			);
		nx -= dx;
		if (nx >= 0x7ff) {
			dp[0] = 0x07ff0000UL;	// INF
			dp[1] = 0;
			return;
		}
		if (dx < 11) {
			// shift right...
			dx = 11 - dx;
			uint32_t uml = ml;
			ml = ((((uint64_t)mh << 32) | uml) >> dx) & 0x00000000ffffffffULL;
			mh = mh >> dx;
		} else {
			dx -= 11;
			if (nx < 1) {
				nx = 0;
				dx = ex + EXPO_PRE_OFFSET - (ex != 0);
			}
			if (dx) {
				dx = 32 - dx;
				uint32_t uml = ml;
				uint32_t umt = mt;
				mh = ((((uint64_t)mh << 32) |
					uml) >> dx) & 0x00000000ffffffffULL;
				ml = ((((uint64_t)uml << 32) |
					umt) >> dx) & 0x00000000ffffffffULL;
			}
		}
		dp[0] = neg | ((nx & 0x07ff) << 20) | (mh & 0x000fffffUL);
		dp[1] = ml;
	} else {
		dp[0] = 0;
		dp[1] = 0;
	}
}

/**
 * This routine is not capable of spanning exponent packet boundaries.
 * since that causes more overhead, both here and in caller's code,
 * it is probably OK.
 *
 * It does, however, handle mantissa packet boundaries.
 *
 * Note, exponents are not signed quantities. 0x000 represents the
 * smallest exponent (~ e-309 if mantissa bits are one).
 * therefore we don't have to worry about exponent signs.
 */
void _core_fp64_post_2pass(double *dst, uint16_t *src_e, uint32_t *src_m, int count) {
	uint32_t *ep = (uint32_t *)src_e;
	uint32_t *mp = src_m;
	double *dp = dst;
	uint32_t ux;
	int32_t mh, ml, mt;
	uint32_t align = (unsigned)mp & 0x00ffUL;

	// DCMF_assert_debug(((unsigned)mp & 0x00ffUL) == 0x0000);
	// DCMF_assert_debug(n <= EXPO_PER_PKT);
	int n = count >> 1;
	while (n-- > 0) {
		ux = *ep++;
		if (((unsigned)mp & 0x00ffUL) == align) {
			++mp;
		}
		mh = *mp++;
		ml = *mp++;
		mt = *mp++;
		regen(mh, ml, mt, (ux >> 16), dp);
		++dp;

		if (((unsigned)mp & 0x00ffUL) == align) {
			++mp;
		}
		mh = *mp++;
		ml = *mp++;
		mt = *mp++;
		regen(mh, ml, mt, (ux & 0x0000ffffUL), dp);
		++dp;
	}
	if (count & 1) {
		if (((unsigned)mp & 0x00ffUL) == align) {
			++mp;
		}
		mh = *mp++;
		ml = *mp++;
		mt = *mp++;
		regen(mh, ml, mt, *((uint16_t *)ep), dp);
		//++dp;
	}
}
