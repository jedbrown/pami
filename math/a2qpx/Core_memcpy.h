/**
 * \file math/a2qpx/Core_memcpy.h
 * \brief ???
 */
#ifndef __math_a2qpx_Core_memcpy_h__
#define __math_a2qpx_Core_memcpy_h__

#define OPTIMIZED_Core_memcpy

#include <stdint.h>
#include <stdlib.h>
#include "util/common.h"
#include "math/a2qpx/a2qpx_asm.h"

static inline void* _int8Cpy( void *dest, const void *src, size_t n )
{
  const uint8_t *sp = (const uint8_t *)src;
  uint8_t *dp = (uint8_t *)dest;

  while( n-- )
    *dp++ = *sp++;

  return( dest );
}

//Copy n bytes. Assume 8 byte alignment
static inline void* _int64Cpy( char *dest, char *src , size_t n, bool alignment )
{  
  uint64_t *sp = (uint64_t *)src;
  uint64_t *dp = (uint64_t *)dest;
  size_t new_n    = n;

  if (alignment) {    
    size_t dwords   = n >> 5;
    size_t dwords_2 = n >> 4;
    new_n  = n&0xf;
    
    uint64_t a,b,c,d;
    
    while ( dwords-- )
    {
      a = *(sp+0);
      b = *(sp+1);
      c = *(sp+2);
      d = *(sp+3);

      *(dp+0) = a;
      *(dp+1) = b;
      *(dp+2) = c;
      *(dp+3) = d;
            
      sp += 4;
      dp += 4;
    }
    
    if (dwords_2) {    
      *(dp+0) = *(sp+0);
      *(dp+1) = *(sp+1);
      
      dp += 2;
      sp += 2;
    }
  }

  _int8Cpy( dp, sp, new_n );
  
  return( dest );
}


#include "math/a2qpx/qpx_copy.h"

//copying 'num' bytes. Assume 32b alignment
static inline int quad_copy_128n( char* dest, char* src, size_t num ) 
{
  size_t b128 = num >> 7;
  size_t nr = num & 0x7f;
  
  while (b128 --) {
    quad_copy_128(dest, src);
    dest += 128;
    src  += 128;
  }

  return (num - nr);
}

//Copying 512 bytes. All possible alignments
inline void* Core_memcpy_512(void* dst, void* src) {  
  uint64_t alignment = (uint64_t)dst | (uint64_t)src;
  uint64_t align8  = alignment & 0x7;
  uint64_t align32 = alignment & 0x1f;

  if ( align32 != 0 ) 
  {
    _int64Cpy( (char *)dst, (char *)src, 512, (align8==0) );
  }
  else 
    quad_copy_512((char *)dst, (char *)src);
  return dst;
}

//Copying n bytes. All possible alignments
inline void* Core_memcpy(void* dst, void* src, size_t bytes)
{
  char *sp = (char *) src;
  char *dp = (char *) dst;
  size_t nb = 0;

  uint64_t alignment = (uint64_t)dp | (uint64_t)sp;
  uint64_t align8  = alignment & 0x7;
  uint64_t align32 = alignment & 0x1f;
  
  if (align32 == 0) {
    if (bytes >= 1024)
      nb = quad_copy_1024n(dp, sp, bytes);
    nb += quad_copy_128n( dp + nb, sp + nb, bytes - nb);
  }
  
  if (bytes - nb)
    _int64Cpy( dp + nb, sp + nb, bytes - nb, (align8==0));
  
  return dst;
}


#endif
