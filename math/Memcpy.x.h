/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/Memcpy.x.h
 * \brief ???
 */
#include "Util.h"

#ifndef OPTIMIZED_Core_memcpy
void * Core_memcpy (void *dest, const void *src, size_t n)
{
  return memcpy(dest, src, n);
}
#endif
