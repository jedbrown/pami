/**
 * \file components/devices/shmemcoll/opt_copy_a2.h
 * \brief ???
 */
#ifndef __math_a2qpx_Core_memcpy__
#define __math_a2qpx_Core_memcpy__

#define OPTIMIZED_Core_memcpy

#include "math/a2qpx/a2qpx_asm.h"

static inline void* _int8Cpy( void *dest, const void *src, size_t n )
{
  const uint8_t *sp = (const uint8_t *)src;
  uint8_t *dp = (uint8_t *)dest;
  size_t n4 = n >> 2;
  size_t nt = n & 3;

  while ( n4-- )
    {
      uint8_t a = *(sp+0);
      uint8_t b = *(sp+1);
      uint8_t c = *(sp+2);
      uint8_t d = *(sp+3);
      *(dp+0) = a;
      *(dp+1) = b;
      *(dp+2) = c;
      *(dp+3) = d;
      sp += 4;
      dp += 4;
    }

  while( nt-- )
    *dp++ = *sp++;

  return( dest );
}

static inline void* _int64Cpy( void *dest, const void *src , size_t n )
{
  const uint64_t *sp = (const uint64_t *)src;
  uint64_t *dp = (uint64_t *)dest;
  size_t dwords = n >> 5;
  size_t dwords_2 = n>>4;
  size_t new_n  = n&0xf;

  if (dwords--) {
    uint64_t a = *(sp+0);
    uint64_t b = *(sp+1);
    uint64_t c = *(sp+2);
    uint64_t d = *(sp+3);

    while ( dwords-- )
      {
	*(dp+0) = a;
	*(dp+1) = b;
	*(dp+2) = c;
	*(dp+3) = d;

	a = *(sp+4);
	b = *(sp+5);
	c = *(sp+6);
	d = *(sp+7);

	sp += 4;
	dp += 4;
      }
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

  if ( new_n )
    _int8Cpy( dp, sp, new_n );
  
  return( dest );
}

static inline int quad_double_copy_128( double* dest, double* src ) {  
    register double *fpp1_1;
    register double *fpp2_1;

    int r0;
    int r1;
    int r2;
    int r3;

    r0 = 0;
    r1 = 64;
    r2 = 32;
    r3 = 96;    

    fpp1_1 = src;
    fpp2_1 = dest;

    // may speed up the 2nd to 7th load by using non-update versions of load, tbd later...
    VECT_LOAD_NU(fpp1_1,r0,0);
    VECT_LOAD_NU(fpp1_1,r1,1);        

    //Wait for the load offset 0 to complete    
    VECT_STORE_NU(fpp2_1,r0,0);
    //load bottom half of cache line
    VECT_LOAD_NU(fpp1_1,r2,1);        

    VECT_STORE_NU(fpp2_1,r1,0);
    VECT_LOAD_NU(fpp1_1,r3,1);        
    
    VECT_STORE_NU(fpp2_1,r2,0);
    VECT_STORE_NU(fpp2_1,r3,0);

    return 0;
}

static inline int quad_double_copy_512( double* dest, double* src ) {  
    register double *fpp1_1, *fpp1_2;
    register double *fpp2_1, *fpp2_2;

    int r0;
    int r1;
    int r2;
    int r3;
    int r4;
    int r5;
    int r6;
    int r7;
    r0 = 0;
    r1 = 64;
    r2 = 128;
    r3 = 192;
    r4 = 256;
    r5 = 320;
    r6 = 384;
    r7 = 448;

    fpp1_1 = src;
    fpp1_2 = src +4;

    fpp2_1 = dest;
    fpp2_2 = dest +4;

    // may speed up the 2nd to 7th load by using non-update versions of load, tbd later...
    VECT_LOAD_NU(fpp1_1,r0,0);
    //asm volatile("qvlfdx 0,%0,%1": : "Ob" (fpp1_1), "r"(r0) :"memory");
    VECT_LOAD_NU(fpp1_1,r1,1);
    VECT_LOAD_NU(fpp1_1,r2,2);
    VECT_LOAD_NU(fpp1_1,r3,3);
    VECT_LOAD_NU(fpp1_1,r4,4);
    VECT_LOAD_NU(fpp1_1,r5,5);
    VECT_LOAD_NU(fpp1_1,r6,6);
    VECT_LOAD_NU(fpp1_1,r7,7);

    VECT_STORE_NU(fpp2_1,r0,0);
    VECT_LOAD_NU(fpp1_2,r0,0);
    VECT_STORE_NU(fpp2_1,r1,1);
    VECT_LOAD_NU(fpp1_2,r1,1);
    VECT_STORE_NU(fpp2_1,r2,2);
    VECT_LOAD_NU(fpp1_2,r2,2);   
    VECT_STORE_NU(fpp2_1,r3,3);
    VECT_LOAD_NU(fpp1_2,r3,3);
    VECT_STORE_NU(fpp2_1,r4,4);
    VECT_LOAD_NU(fpp1_2,r4,4);
    VECT_STORE_NU(fpp2_1,r5,5);
    VECT_LOAD_NU(fpp1_2,r5,5);
    VECT_STORE_NU(fpp2_1,r6,6);
    VECT_LOAD_NU(fpp1_2,r6,6);
    VECT_STORE_NU(fpp2_1,r7,7);
    VECT_LOAD_NU(fpp1_2,r7,7);
    
    VECT_STORE_NU(fpp2_2,r0,0);
    VECT_STORE_NU(fpp2_2,r1,1);
    VECT_STORE_NU(fpp2_2,r2,2);
    VECT_STORE_NU(fpp2_2,r3,3);
    VECT_STORE_NU(fpp2_2,r4,4);
    VECT_STORE_NU(fpp2_2,r5,5);
    VECT_STORE_NU(fpp2_2,r6,6);
    VECT_STORE_NU(fpp2_2,r7,7);

    return 0;
}


//copying 'num' doubles, num >= 128, in multiples of 64
static inline int quad_double_copy_1024n( double* dest, double* src, size_t num )
{

  double *fpp1_1, *fpp1_2;
  double *fpp2_1, *fpp2_2;
  uint64_t y;

  register int inc asm("r7");

  fpp1_1 = src -8;  //offset by stride=0 bytes
  fpp1_2 = src -4;

  fpp2_1 = dest -8;  //offset by stride=0 bytes
  fpp2_2 = dest -4;

  inc=64;

  y=(num-64)/64;

  VECTOR_LOAD(fpp1_1,inc,0);
  VECTOR_LOAD(fpp1_1,inc,1);
  VECTOR_LOAD(fpp1_1,inc,2);
  VECTOR_LOAD(fpp1_1,inc,3);
  VECTOR_LOAD(fpp1_1,inc,4);
  VECTOR_LOAD(fpp1_1,inc,5);
  VECTOR_LOAD(fpp1_1,inc,6);
  VECTOR_LOAD(fpp1_1,inc,7);

  VECTOR_STORE(fpp2_1,inc,0);
  VECTOR_LOAD(fpp1_1,inc,0);
  VECTOR_LOAD(fpp1_2,inc,8);

  VECTOR_STORE(fpp2_1,inc,1);
  VECTOR_LOAD(fpp1_1,inc,1);
  VECTOR_LOAD(fpp1_2,inc,9);

  do{

    VECTOR_STORE(fpp2_1,inc,2);
    VECTOR_LOAD(fpp1_1,inc,2);
    VECTOR_LOAD(fpp1_2,inc,10);
    VECTOR_STORE(fpp2_2,inc,8);

    VECTOR_STORE(fpp2_1,inc,3);
    VECTOR_LOAD(fpp1_1,inc,3);
    VECTOR_LOAD(fpp1_2,inc,11);
    VECTOR_STORE(fpp2_2,inc,9);

    VECTOR_STORE(fpp2_1,inc,4);
    VECTOR_LOAD(fpp1_1,inc,4);
    VECTOR_LOAD(fpp1_2,inc,8);
    VECTOR_STORE(fpp2_2,inc,10);

    VECTOR_STORE(fpp2_1,inc,5);
    VECTOR_LOAD(fpp1_1,inc,5);
    VECTOR_LOAD(fpp1_2,inc,9);
    VECTOR_STORE(fpp2_2,inc,11);

    VECTOR_STORE(fpp2_1,inc,6);
    VECTOR_LOAD(fpp1_1,inc,6);
    VECTOR_LOAD(fpp1_2,inc,10);
    VECTOR_STORE(fpp2_2,inc,8);

    VECTOR_STORE(fpp2_1,inc,7);
    VECTOR_LOAD(fpp1_1,inc,7);
    VECTOR_LOAD(fpp1_2,inc,11);
    VECTOR_STORE(fpp2_2,inc,9);

    if(!--y) break;

    VECTOR_STORE(fpp2_1,inc,0);
    VECTOR_LOAD(fpp1_1,inc,0);
    VECTOR_LOAD(fpp1_2,inc,8);
    VECTOR_STORE(fpp2_2,inc,10);

    VECTOR_STORE(fpp2_1,inc,1);
    VECTOR_LOAD(fpp1_1,inc,1);
    VECTOR_LOAD(fpp1_2,inc,9);
    VECTOR_STORE(fpp2_2,inc,11);

  }while(1);

  VECTOR_STORE(fpp2_1,inc,0);
  VECTOR_LOAD(fpp1_2,inc,8);
  VECTOR_STORE(fpp2_2,inc,10);

  VECTOR_STORE(fpp2_1,inc,1);
  VECTOR_LOAD(fpp1_2,inc,9);
  VECTOR_STORE(fpp2_2,inc,11);

  VECTOR_STORE(fpp2_1,inc,2);
  VECTOR_LOAD(fpp1_2,inc,10);
  VECTOR_STORE(fpp2_2,inc,8);

  VECTOR_STORE(fpp2_1,inc,3);
  VECTOR_LOAD(fpp1_2,inc,11);
  VECTOR_STORE(fpp2_2,inc,9);

  VECTOR_STORE(fpp2_1,inc,4);
  VECTOR_LOAD(fpp1_2,inc,8);
  VECTOR_STORE(fpp2_2,inc,10);

  VECTOR_STORE(fpp2_1,inc,5);
  VECTOR_LOAD(fpp1_2,inc,9);
  VECTOR_STORE(fpp2_2,inc,11);

  VECTOR_STORE(fpp2_1,inc,6);
  VECTOR_LOAD(fpp1_2,inc,10);
  VECTOR_STORE(fpp2_2,inc,8);

  VECTOR_STORE(fpp2_1,inc,7);
  VECTOR_LOAD(fpp1_2,inc,11);
  VECTOR_STORE(fpp2_2,inc,9);

  VECTOR_STORE(fpp2_2,inc,10);

  VECTOR_STORE(fpp2_2,inc,11);


  return 0;
}

inline void* Core_memcpy_512(void* dst, void* src) {
  uint64_t alignment = 32;    
  if ( unlikely(((uint64_t)dst | (uint64_t)src) & (alignment-1)) )
  {
    // 8b aligned.
    alignment = 8;
    if ( (((uint64_t)dst | (uint64_t)src) & (alignment-1)) == 0 )
      return _int64Cpy( dst, src, 512 );
    else
      return _int8Cpy( dst, src, 512 );
  }
  else 
    quad_double_copy_512((double*)dst, (double*)src);
  return dst;
}

inline void* Core_memcpy(void* dst, void* src, size_t bytes)
{
  uint64_t alignment = 32;  
  if (((uint64_t)dst | (uint64_t)src) & (alignment-1))
  {
    // 8b aligned.
    alignment = 8;
    if ( (((uint64_t)dst | (uint64_t)src) & (alignment-1)) == 0 )
      return _int64Cpy( dst, src, bytes );
    else
      return _int8Cpy( dst, src, bytes );
  }
  else //32b aligned
  {
    //multiples of 1024
    size_t quad_copy_bytes = (bytes /1024)* 1024;
    size_t bytes_remaining = bytes - quad_copy_bytes;
    
    if (quad_copy_bytes > 0)
      {
	quad_double_copy_1024n((double*)dst, (double*)src, (quad_copy_bytes)/sizeof(double));
      }
    
    if (bytes_remaining > 0)
      {
	_int64Cpy( (char*)dst + quad_copy_bytes, (char*)src + quad_copy_bytes, bytes_remaining);
      }
    
  }
  return dst;
}


#endif
