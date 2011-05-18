/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/a2qpx/qpx_copy.cc
 * \brief ???
 */

#include <stdint.h>
#include "util/common.h"
#include "math/a2qpx/qpx_copy.h"

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
