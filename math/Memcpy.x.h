
#ifdef __bgq__
#include "math/a2qpx/Core_memcpy.h"
#endif

#ifndef OPTIMIZED_Core_memcpy
void * Core_memcpy (void *dest, const void *src, size_t n)
{
  return memcpy(dest, src, n);
}
#endif
