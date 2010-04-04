#ifndef __opt_copy_a2_h__
#define __opt_copy_a2_h__

#include "asmheader.h"
#include "assert.h"

#ifdef __GNUC__
#define __MYASM__(p) asm(p)
#else
#define __MYASM__(p) /* nothing */
#endif
  
//copying 'num' doubles, num >= 128, in multiples of 64
inline int quad_double_copy( double* dest, double* src, size_t num )
{

  double *fpp1_1, *fpp1_2;
  double *fpp2_1, *fpp2_2;
  uint64_t y;

  register int inc __MYASM__("r7");
	
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



inline int opt_bgq_memcpy(void* dst, void* src, size_t bytes)
{
	uint64_t alignment = 64;
	uint64_t mask = (alignment - 1);
	uint64_t dst_buf = (uint64_t)dst;
	uint64_t src_buf = (uint64_t)src;

	assert ((dst_buf & mask) == 0);
	assert ((src_buf & mask) == 0);


	//multiples of 1024
	size_t quad_copy_bytes = (bytes /1024)* 1024;
	size_t bytes_remaining = bytes - quad_copy_bytes;

	if (quad_copy_bytes > 0) 
	{
		quad_double_copy((double*)dst_buf, (double*)src_buf, (quad_copy_bytes)/sizeof(double));
	}
	
	if (bytes_remaining > 0)
	{
		memcpy((void*)((char*)dst_buf + quad_copy_bytes), (void*)((char*)src_buf + quad_copy_bytes), bytes_remaining);
	}


#if 0
	//Check if aligned to 128 bytes, L2 cache line size
	if (((uint64_t)dst | (uint64_t)src) & 0x7F)  {
		
		//Check if the bytes is a multiple of 512, 
		if (bytes % 512 == 0)
		{
			quad_double_copy((double*)dst, (double*)src, bytes/sizeof(double));
		}
		else memcpy(dst, src, bytes);
	}
	else{
		memcpy(dst, src, bytes);
	}	
#endif
	return 0;
}

#if 0
inline int opt_bgq_memcpy(void* dst, void* src, size_t bytes)
{
	uint64_t alignment = 64;
	uint64_t mask = (alignment - 1);
	uint64_t dst_buf = (uint64_t)dst;
	uint64_t src_buf = (uint64_t)src;

	// Do the destination and source alignments match ??
	if ((dst_buf & (uint64_t)mask) == (src_buf & (uint64_t)mask))  {

		uint64_t dst_aligned = (dst_buf & ~mask);
		uint64_t src_aligned = (src_buf & ~mask);
		size_t bytes_unaligned = (alignment - (dst_buf & mask));
		
		//multiples of 1024
		size_t quad_copy_bytes = ((bytes - bytes_unaligned)/1024)* 1024;
		size_t bytes_remaining = bytes- bytes_unaligned - quad_copy_bytes;

		if (quad_copy_bytes > 0) 
		{
			quad_double_copy((double*)dst_aligned, (double*)src_aligned, (quad_copy_bytes)/sizeof(double));
		}
		
		if (bytes_remaining > 0)
		{
			memcpy((void*)(dst_aligned + quad_copy_bytes), (void*)(src_aligned + quad_copy_bytes), bytes_remaining);
		}

		if (bytes_unaligned > 0)
		{
			memcpy(dst, src, bytes_unaligned);
		}

	}
	else memcpy(dst, src, bytes);

#if 0
	//Check if aligned to 128 bytes, L2 cache line size
	if (((uint64_t)dst | (uint64_t)src) & 0x7F)  {
		
		//Check if the bytes is a multiple of 512, 
		if (bytes % 512 == 0)
		{
			quad_double_copy((double*)dst, (double*)src, bytes/sizeof(double));
		}
		else memcpy(dst, src, bytes);
	}
	else{
		memcpy(dst, src, bytes);
	}	
#endif
	return 0;
}
#endif

#endif
