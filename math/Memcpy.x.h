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

#include <string.h>
void * Core_memcpy (void *dest, const void *src, size_t n);

#ifdef __bgq__
#include "math/a2qpx/Core_memcpy.h"
#endif
