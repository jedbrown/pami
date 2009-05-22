/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/FloatUtil.cc
 * \brief Floating point operation utilities
 *
 * \todo the functions in this file are not truely inline and lack any
 * sort of name scoping.  They need to be changed to avoid collision
 * with application names
 */
#include "math_coremath.h"
#include "FloatUtil.h"
/*
 * u2 = firstword - u2
 * Need f since PPC unhappy with: *w1=(ADD_OP_T*)&firstword
 */
static void subfromfirstword(int ulen, uint64_t firstword, uint64_t *u2)
{
  int ratio=sizeof(uint64_t)/sizeof(ADD_OP_T);
  int i, wlen=ulen*ratio, carry=0;

  /**  \note In order for this code to work, we need these volatiles
   *  The gcc compiler seems to be generating bad code for this chunk
   *  of code without the volatiles
   *
   *  \todo track down root cause of the need for these volatiles
   */
  volatile uint64_t f=firstword;
  volatile uint64_t *df = &f;
  ADD_OP_T *w1=(ADD_OP_T*)df, *w2=(ADD_OP_T*)u2;
  ADD_OP2_T sub;
  for (i=wlen-1; i>=ratio; i--)
    {
      sub = (OP_10000 - w2[i]) - carry;
      w2[i] = sub & OP_FFFF;
      carry = (sub < OP_10000) ? 1 : 0;
    }
  for (i=ratio-1; i>=0; i--)
    {
      sub = ((OP_10000 + w1[i]) - w2[i]) - carry;
      w2[i] = sub & OP_FFFF;
      carry = (sub < OP_10000) ? 1 : 0;
    }
}

/*
 * u1 = u1 - firstword
 */
static void subfirstword(int ulen, uint64_t *u1, uint64_t firstword)
{
  int ratio=sizeof(uint64_t)/sizeof(ADD_OP_T);
  int i, wlen=ulen*ratio, carry=0;
  uint64_t f=firstword;
  uint64_t *df=&f;
  ADD_OP_T *w1=(ADD_OP_T*)u1, *w2=(ADD_OP_T*)df;
  ADD_OP2_T sub;
  for (i=wlen-1; i>=ratio; i--)
    {
      sub = (OP_10000 + w1[i]) - carry;
      w1[i] = sub & OP_FFFF;
      carry = (sub < OP_10000) ? 1 : 0;
    }
  for (i=ratio-1; i>=0; i--)
    {
      sub = ((OP_10000 + w1[i]) - w2[i]) - carry;
      w1[i] = sub & OP_FFFF;
      carry = (sub < OP_10000) ? 1 : 0;
    }
}
int double2uint(double *d, int ulen, uint64_t *u)
{
  uint64_t b = *((uint64_t*)d);  /* b is original*/
  uint64_t m = *((uint64_t*)d);  /* m is used to shift up mantissa.*/
  int blen = ulen*64;            /* length of u in bits.*/
  int i;

  int origfirstbit;     /* bit position in u, before exponent shift.*/
  int newfirstbit;      /* bit position in u, after  exponent shift.*/
  int firstword;        /* newfirstbit starts in u[firstword]
                         * The remaining bits are in u[firstword+1].*/
  int bitsinfirstword;  /*The number of bits in u[firstword].*/

  if (DOUBLE_EXP(b) == 2047)
    {
      u[0] = BGL_TR_BAD_OPERAND;  /* No need to set other u[i].*/
      return -1; /* NAN */
    }
  /*
   * Shift up just the mantissa, so turn off the sign.
   */
  m &= ~DOUBLE_BITS_SIGN;
  if (m & DOUBLE_BITS_EXP)
    {
      /*
       * Normalized number, i.e. non-0 exponent,
       * has implicit leading 1 on mantissa.
       * Use bit-layout feature that exponent LSB precedes mantissa MSB.
       */
      SET_DOUBLE_EXP(m,1);
    }
  /* Shift up the entire 64-bit mantissa m.
   * Bit 11 is the implicit 1.
   * For the maximum exponent supported, bit 10 is effective sign bit.
   * So first 10 bits 0:9 in m are always 0.
   * To avoid overflow on 2^16==65536 nodes, need topmost 16 bits of u to be 0.
   * So newfirstbit==0 would have only 10 free bits.
   * So require newfirstbit >= 6.
   * So bit 16 in BGL_TR_HUMUNGOUS gets added to every number,
   * leaving 16 bits 0:15 as 0.
   * Addendum:
   * Prepending the above, need the topmost 17 bits to be 0,
   * in order to propagate one or more nodes inserting NaN or Inf.
   * 17 bits need when all 2^16 nodes contribute NAn or Inf.
   * So require newfirstbit >=6+17.
   * So bit 16+17=33 in BGL_TR_HUMUNGOUS gets added to every number,
   * leaving 16+17=33 bits 0:32 as 0.

   * Optimization possibility:
   * Instead of assuming 2^16 participants,
   * could get actual number of participants as argument
   * and decrease the 16+17=33 bits appropriately.
   */
  origfirstbit = blen - 64;
  if (DOUBLE_EXP(b) <= 1)
    {
      /* Don't shift at all if e = 0 or if e = 1.*/
      newfirstbit  = origfirstbit;
    }
  else
    {
      /* Shift up by e-1. */
      newfirstbit  = origfirstbit - (DOUBLE_EXP(b)-1);
    }
  if (newfirstbit<6+17)
    {
      /* I.e. ulen is too small.*/
      u[0] = BGL_TR_BAD_OPERAND;  /* No need to set other u[i].*/
      return -1; /* exponent too large */
    }

  for (i=0; i<ulen; i++)
    u[i] = 0;

  firstword = newfirstbit / 64;
  bitsinfirstword = 64 - (newfirstbit % 64);
  u[firstword  ] = m >> (64 - bitsinfirstword);
  if (firstword < ulen-1)
    {
      u[firstword+1] = m << (     bitsinfirstword);
    }

  /* Tree only does unsigned add.
   * Tree doesn't like two's complement since alu_overflow_irq may fire.
   * e.g. ADD(-1,-1) == ADD(FFFF,FFFF) assuming 16-bit operands.
   * [If sacrifice initial 16-bit of 0, alu_overflow_irq won't fire.]
   * So instead treat negative numbers by moving numbers to positive range.
   */
  if (*d >= 0)
    {
      /* Positive numbers just set bit 16.*/
      u[0] |= BGL_TR_HUMUNGOUS;
    }
  else
    {
      uint64_t d = BGL_TR_HUMUNGOUS;
      /* u = d - u*/
      subfromfirstword(ulen, d, u);
    }
  return 0;
}


int uint2double(int participants, double *d, int ulen, uint64_t *u)
{
  uint64_t b=0, nan=0;
  uint64_t m, p;

  int firstword; /* The firstword of u containing non-0 bits.*/
  int firstbit;  /* The first bit within firstword;
                    This does not include the leading 0.*/
  int bitsinfirstword;  /* 64 - firstbit.*/

  if (u[0] & 0xFFFF800000000000ull)
    {
      SET_DOUBLE_EXP(nan,2047);
      SET_DOUBLE_MANTISSA(nan, 1);

      Core_memcpy(d,&nan,sizeof(double));
      return -1; /* NAN */
    }

  *d = 0.; /* In case of early exit.*/

  p = BGL_TR_HUMUNGOUS * participants;
  if (u[0]>=p)
    {
      b &= ~DOUBLE_BITS_SIGN;     /* Turn off the sign.*/
      /* u = u - p */
      subfirstword(ulen, u, p);
    }
  else
    {
      b |=  DOUBLE_BITS_SIGN;     /* Turn on  the sign.*/
      /* u = p - u */
      subfromfirstword(ulen, p, u);
    }

  for (firstword=0; firstword<ulen; firstword++)
    {
      if (u[firstword])
        {
          break;
        }
    }
  if (firstword == ulen)
    {
      return 0;
    }
  firstbit = cntlz64(u[firstword]);
  if (firstword == (ulen-1) && firstbit >= 12)
    {
      /* firstbit==12 and beyond is unnormalized number.*/
      SET_DOUBLE_EXP(b,0);
      firstbit = 12;
    }
  else
    {
      /* E.g. original firstbit 11 is normalized number, but there is no shift.*/
      int e = 64*(ulen-1-firstword) + 12 - firstbit;
      if (e >= 2047)
        {
          SET_DOUBLE_EXP(b,2047);
          SET_DOUBLE_MANTISSA(b, 1);
          Core_memcpy(d,&b,sizeof(double));
          return -1; /* NAN */
        }
      SET_DOUBLE_EXP(b, e);
      /* For normalized number, ++firstbit ignores the implicit 1,
       * which is not included in the mantissa.
       */
      ++firstbit;
    }

  /* firstbit ranges from 1 to 64 inclusive.*/
  bitsinfirstword = 64 - firstbit;

  /* Assemble m starting at bit 0,
   * since we don't know whether to shift up or down.
   */
  m = u[firstword] << firstbit;
  m |= u[firstword+1] >> bitsinfirstword;
  SET_DOUBLE_MANTISSA(b, m >> 12 );

  /* The above truncates the mantissa.
   * Could, but don't bother to round, since the tree ADD doesn't round.
   * So missing single last bit of precision is negligible
   * compared to the much larger differences that can arise
   * between summing many doubles and summing 2048-bit integers.
   */

  Core_memcpy(d,&b,sizeof(double));

  return 0;
}
