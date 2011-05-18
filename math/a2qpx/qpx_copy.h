/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/a2qpx/qpx_copy.h
 * \brief ???
 */

#ifndef __math_a2qpx_qpx_copy_h__
#define __math_a2qpx_qpx_copy_h__

#include "math/a2qpx/a2qpx_asm.h"

//Copy 128 bytes from 32b aligned pointers
static inline size_t quad_copy_128( char* dest, char* src ) {  
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

    fpp1_1 = (double *) src;
    fpp2_1 = (double *) dest;

    // may speed up the 2nd to 7th load by using non-update versions of load, tbd later...
    VECT_LOAD_NU(fpp1_1,r0,0);
    VECT_LOAD_NU(fpp1_1,r1,1);        

    //Wait for the load offset 0 to complete    
    VECT_STORE_NU(fpp2_1,r0,0);
    //load bottom half of cache line
    VECT_LOAD_NU(fpp1_1,r2,0);        

    VECT_STORE_NU(fpp2_1,r1,1);
    VECT_LOAD_NU(fpp1_1,r3,1);        
    
    VECT_STORE_NU(fpp2_1,r2,0);
    VECT_STORE_NU(fpp2_1,r3,1);

    return 0;
}

//Copy 512 bytes from a 32b aligned pointers
static inline size_t quad_copy_512( char* dest, char* src ) {  
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

    fpp1_1 = (double *)src;
    fpp1_2 = (double *)src +4;

    fpp2_1 = (double *)dest;
    fpp2_2 = (double *)dest +4;

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

//copying 'num' bytes. Assume 32b alignment
static inline int quad_copy_128n( char* dest, char* src, size_t num ) 
{
  size_t b128 = num >> 7;
  size_t nr = num & ~(0x7FL);
  
  while (b128 --) {
    quad_copy_128(dest, src);
    dest += 128;
    src  += 128;
  }
  
  return nr;
}

size_t quad_copy_1024n( char* dest, char* src, size_t num ) __attribute__((noinline,weak));

//copying num bytes num >= 1024
size_t quad_copy_1024n( char* dest, char* src, size_t num )
{
  size_t nb = 0; 
  if (num < 1024)
    goto short_msg;

  nb = num & ~(1023L);

  double *fpp1_1, *fpp1_2;
  double *fpp2_1, *fpp2_2;
  size_t y;

  register int inc asm("r7");
  
  fpp1_1 = (double *)src -8;  //offset by stride=0 bytes
  fpp1_2 = (double *)src -4;

  fpp2_1 = (double *)dest -8;  //offset by stride=0 bytes
  fpp2_2 = (double *)dest -4;

  inc=64;

  y=((nb >> 3) - 64)/64;

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


  src += nb;
  dest += nb;
  num -= nb;
  
 short_msg:
  if (num >= 512) {
    quad_copy_512(dest, src);
    src  += 512;
    dest += 512;
    num  -= 512;
  }

  nb += quad_copy_128n (dest, src, num);
  return nb;
}


#endif
