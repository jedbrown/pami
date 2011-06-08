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

#define QPX_LOAD(si,sb,fp)			\
  do {									\
  asm volatile("qvlfdux %0,%1,%2": "=f"(fp),"+b" (si): "r" (sb));	\
  } while(0);

#define QPX_LOAD_NU(si,sb,fp) \
  do {									\
  asm volatile("qvlfdx %0,%1,%2": "=f"(fp) : "b" (si), "r" (sb));	\
  } while(0);

#define QPX_STORE(si,sb,fp) \
  do {									\
  asm volatile("qvstfdux %2,%0,%1": "+b" (si) : "r" (sb), "f"(fp) :"memory");	\
  } while(0);

#define QPX_STORE_NU(si,sb,fp)						\
  do {									\
  asm volatile("qvstfdx %2,%0,%1": : "b" (si), "r" (sb), "f"(fp) :"memory"); \
  } while(0);

#endif
