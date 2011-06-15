/**
 * \file components/devices/shmem/opt_copy_a2.h
 * \brief ???
 */
#ifndef __components_devices_shmem_opt_copy_a2_h__
#define __components_devices_shmem_opt_copy_a2_h__

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
  register double f0  __asm__("fr0");
  register double f1  __asm__("fr1");
  register double f2  __asm__("fr2");
  register double f3  __asm__("fr3");
  register double f4  __asm__("fr4");
  register double f5  __asm__("fr5");
  register double f6  __asm__("fr6");
  register double f7  __asm__("fr7");
  register double f8  __asm__("fr8");
  register double f9  __asm__("fr9");
  register double f10 __asm__("fr10");
  register double f11 __asm__("fr11");

  register int inc __MYASM__("r7");

  fpp1_1 = src -8;  //offset by stride=0 bytes
  fpp1_2 = src -4;

  fpp2_1 = dest -8;  //offset by stride=0 bytes
  fpp2_2 = dest -4;

  inc=64;

  y=(num-64)/64;

  QPX_LOAD(fpp1_1,inc,f0);
  QPX_LOAD(fpp1_1,inc,f1);
  QPX_LOAD(fpp1_1,inc,f2);
  QPX_LOAD(fpp1_1,inc,f3);
  QPX_LOAD(fpp1_1,inc,f4);
  QPX_LOAD(fpp1_1,inc,f5);
  QPX_LOAD(fpp1_1,inc,f6);
  QPX_LOAD(fpp1_1,inc,f7);

  QPX_STORE(fpp2_1,inc,f0);
  QPX_LOAD(fpp1_1,inc,f0);
  QPX_LOAD(fpp1_2,inc,f8);

  QPX_STORE(fpp2_1,inc,f1);
  QPX_LOAD(fpp1_1,inc,f1);
  QPX_LOAD(fpp1_2,inc,f9);

  do{

    QPX_STORE(fpp2_1,inc,f2);
    QPX_LOAD(fpp1_1,inc,f2);
    QPX_LOAD(fpp1_2,inc,f10);
    QPX_STORE(fpp2_2,inc,f8);

    QPX_STORE(fpp2_1,inc,f3);
    QPX_LOAD(fpp1_1,inc,f3);
    QPX_LOAD(fpp1_2,inc,f11);
    QPX_STORE(fpp2_2,inc,f9);

    QPX_STORE(fpp2_1,inc,f4);
    QPX_LOAD(fpp1_1,inc,f4);
    QPX_LOAD(fpp1_2,inc,f8);
    QPX_STORE(fpp2_2,inc,f10);

    QPX_STORE(fpp2_1,inc,f5);
    QPX_LOAD(fpp1_1,inc,f5);
    QPX_LOAD(fpp1_2,inc,f9);
    QPX_STORE(fpp2_2,inc,f11);

    QPX_STORE(fpp2_1,inc,f6);
    QPX_LOAD(fpp1_1,inc,f6);
    QPX_LOAD(fpp1_2,inc,f10);
    QPX_STORE(fpp2_2,inc,f8);

    QPX_STORE(fpp2_1,inc,f7);
    QPX_LOAD(fpp1_1,inc,f7);
    QPX_LOAD(fpp1_2,inc,f11);
    QPX_STORE(fpp2_2,inc,f9);

    if(!--y) break;

    QPX_STORE(fpp2_1,inc,f0);
    QPX_LOAD(fpp1_1,inc,f0);
    QPX_LOAD(fpp1_2,inc,f8);
    QPX_STORE(fpp2_2,inc,f10);

    QPX_STORE(fpp2_1,inc,f1);
    QPX_LOAD(fpp1_1,inc,f1);
    QPX_LOAD(fpp1_2,inc,f9);
    QPX_STORE(fpp2_2,inc,f11);

  }while(1);

  QPX_STORE(fpp2_1,inc,f0);
  QPX_LOAD(fpp1_2,inc,f8);
  QPX_STORE(fpp2_2,inc,f10);

  QPX_STORE(fpp2_1,inc,f1);
  QPX_LOAD(fpp1_2,inc,f9);
  QPX_STORE(fpp2_2,inc,f11);

  QPX_STORE(fpp2_1,inc,f2);
  QPX_LOAD(fpp1_2,inc,f10);
  QPX_STORE(fpp2_2,inc,f8);

  QPX_STORE(fpp2_1,inc,f3);
  QPX_LOAD(fpp1_2,inc,f11);
  QPX_STORE(fpp2_2,inc,f9);

  QPX_STORE(fpp2_1,inc,f4);
  QPX_LOAD(fpp1_2,inc,f8);
  QPX_STORE(fpp2_2,inc,f10);

  QPX_STORE(fpp2_1,inc,f5);
  QPX_LOAD(fpp1_2,inc,f9);
  QPX_STORE(fpp2_2,inc,f11);

  QPX_STORE(fpp2_1,inc,f6);
  QPX_LOAD(fpp1_2,inc,f10);
  QPX_STORE(fpp2_2,inc,f8);

  QPX_STORE(fpp2_1,inc,f7);
  QPX_LOAD(fpp1_2,inc,f11);
  QPX_STORE(fpp2_2,inc,f9);

  QPX_STORE(fpp2_2,inc,f10);

  QPX_STORE(fpp2_2,inc,f11);


  return 0;
}



inline int opt_bgq_memcpy(void* dst, void* src, size_t bytes)
{
	uint64_t alignment = 32;
	uint64_t mask = (alignment - 1);
	uint64_t dst_buf = (uint64_t)dst;
	uint64_t src_buf = (uint64_t)src;

	//assert ((dst_buf & mask) == 0);
	//assert ((src_buf & mask) == 0);
	//PAMI_assertf(((dst_buf & mask) == 0), "dst_buf:%p bytes:%zd\n", dst, bytes);
	//PAMI_assertf(((src_buf & mask) == 0), "src_buf:%p bytes:%zd\n", src, bytes);

	if ( ((dst_buf & mask) != 0) ||
	     ((src_buf & mask) != 0) ) {
	  memcpy (dst, src, bytes);
	  return 0;
	}

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
