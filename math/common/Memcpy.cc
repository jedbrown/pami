/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/common/Memcpy.cc
 * \brief ???
 */

#ifndef OPTIMIZED_Core_memcpy
#include <string.h>
void * Core_memcpy (void *dest, const void *src, size_t n)
{
  return memcpy(dest, src, n);
}
#endif
