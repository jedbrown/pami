/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/ppc450d/Arch.h
 * \brief ???
 */

#ifndef __arch_ppc450d_Arch_h__
#define __arch_ppc450d_Arch_h__

#include "arch/ArchInterface.h"

// Here go things specific to this processor architecture

#include <bpcore/ppc450_inlines.h>
#include <bpcore/bgp_atomic_ops.h>
#include <spi/kernel_interface.h>

#undef  mem_sync
#define mem_sync()      _bgp_msync()

#undef  mem_barrier
#define mem_barrier()   _bgp_mbar()

#define LQU(x, ptr, incr) \
asm volatile ("lfpdux %0,%1,%2" : "=f"(x), "+Ob"(ptr) : "r"(incr) : "memory")
#define SQU(x, ptr, incr) \
asm volatile ("stfpdux %2,%0,%1": "+Ob" (ptr) : "r" (incr), "f" (x) : "memory")


///
/// \brief Template specialization for size_t copies.
///
template <>
template <unsigned N>
void Type<size_t>::copy (size_t * dst, size_t * src)
{
  size_t i;
  for (i=0; i<(N/sizeof(size_t)); i++) dst[i] = src[i];

  if (N%(sizeof(size_t)))
  {
    uint8_t * const d = (uint8_t * const) dst[N%(sizeof(size_t))];
    uint8_t * const s = (uint8_t * const) src[N%(sizeof(size_t))];

    for (i=0; i<(N%(sizeof(size_t))); i++) d[i] = s[i];
  }
};


#if 0
template <>
template <unsigned N>
void Type<xmi_quad_t>::copy (xmi_quad_t * dst, xmi_quad_t * src)
{
  static const unsigned PWIDTH = 3;

  register unsigned i16 = sizeof(xmi_quad_t);
  register void * d = (void *)(((size_t)dst) - sizeof(xmi_quad_t));
  register void * s = (void *)(((size_t)src) - sizeof(xmi_quad_t));

  register double r[9];
  unsigned ld = 0;
  unsigned st = 0;

  unsigned i,p;

#ifdef ENABLE_TRACE
  char out[3][16];
  sprintf (out[0],"---");
  sprintf (out[1],"---");
  sprintf (out[2],"---");
#endif

  // Fill pipeline
  for (p=0; p<PWIDTH; p++)
  {
    if (N > ld++)
    {
      LQU(r[ld],s,i16);
#ifdef ENABLE_TRACE
      snprintf (out[p], "ld%p", ld);
#endif
    }
  }

#ifdef ENABLE_TRACE
  TRACE((stderr, "%s,%s,%s\n", out[0],out[1],out[2]));
  sprintf (out[0],"---");
  sprintf (out[1],"---");
  sprintf (out[2],"---");
#endif

  for (i=0; i<N; i+=3)
  {
    for (p=0; p<PWIDTH; p++)
    {
      if (N > ld++)
      {
        LQU(r[ld%9],s,i16);
#ifdef ENABLE_TRACE
        snprintf (out[p], "ld%p", ld%9);
#endif
      }
    }

#ifdef ENABLE_TRACE
    TRACE((stderr, "%s,%s,%s\n", out[0],out[1],out[2]));
    sprintf (out[0],"---");
    TRACE((stderr, "%s,%s,%s\n", out[0],out[1],out[2]));
    sprintf (out[0],"---");
    sprintf (out[1],"---");
    sprintf (out[2],"---");
#endif

    for (p=0; p<PWIDTH; p++)
    {
      if (N > st++)
      {
        SQU(r[st%9],d,i16);
#ifdef ENABLE_TRACE
        snprintf (out[p], "st%p", st%9);
#endif
      }
    }

#ifdef ENABLE_TRACE
    TRACE((stderr, "%s,%s,%s\n", out[0],out[1],out[2]));
    sprintf (out[0],"---");
    sprintf (out[1],"---");
    sprintf (out[2],"---");
#endif
  }
}
#endif

#endif // __xmi_arch_ppc450d_h__
