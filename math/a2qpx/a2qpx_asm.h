/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/a2qpx/a2qpx_asm.h
 * \brief ???
 */

#ifndef __math_a2qpx_a2qpx_asm_h__
#define __math_a2qpx_a2qpx_asm_h__

#define VECTOR_LOAD(si,sb,tgt) \
do { \
  asm volatile("qvlfdux %2,%0,%1": "+Ob" (si) : "r" (sb), "i" (tgt) :"memory"); \
} while(0)

#define VECT_LOAD_NU(si,sb,tgt) \
do { \
  asm volatile("qvlfdx %2,%0,%1": : "Ob" (si), "r" (sb), "i" (tgt) :"memory"); \
} while(0)

#define VECTOR_STORE(si,sb,tgt) \
do { \
  asm volatile("qvstfdux %2,%0,%1": "+Ob" (si) : "r" (sb), "i" (tgt) :"memory"); \
} while(0)

#define VECT_STORE_NU(si,sb,tgt) \
do { \
  asm volatile("qvstfdx %2,%0,%1": "+Ob" (si) : "r" (sb), "i" (tgt) :"memory"); \
} while(0)

#define VECTOR_STORE_REG(si,sb,tgt) \
do { \
  asm volatile("qvstfdux %2,%0,%1": "+Ob" (si) : "r" (sb), "f" (tgt) :"memory"); \
} while(0)

#define VECTOR_SUM(si,sb,tgt) \
do { \
  asm volatile("qvfadd tgt,%0,%1":  : "i" (si), "i" (sb)); \
} while(0)

#define VECTOR_SUM_REG(si,sb,tgt) \
do { \
  asm volatile("qvfadd %0,%1,%2": "=f" (tgt) : "i" (si), "f" (sb)); \
} while(0)

#endif
