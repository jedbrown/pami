/**
 * \file components/devices/shmemcoll/4way_sum.h
 * \brief ???
 */
#ifndef __components_devices_shmemcoll_4way_sum_h__
#define __components_devices_shmemcoll_4way_sum_h__

#include "asmheader.h"

#ifdef __GNUC__
#define __MYASM__(p) asm(p)
#else
#define __MYASM__(p) /* nothing */
#endif

#define ASM asm volatile

//summing 'num' doubles, num >= 32 in multiples of 16
inline int quad_double_sum_4way( double* dest, double* src0, double *src1, double* src2, double* src3, uint64_t num )
{

  double *src0_1, *src0_2, *src1_1, *src1_2, *src2_1, *src2_2, *src3_1, *src3_2;
  double *dst_1, *dst_2;
  uint64_t y;

  register int inc __MYASM__("r7");

  src0_1 = src0 -8;  //offset by stride=0 bytes
  src0_2 = src0 -4;

  src1_1 = src1 -8;  //offset by stride=0 bytes
  src1_2 = src1 -4;

  src2_1 = src2 -8;  //offset by stride=0 bytes
  src2_2 = src2 -4;

  src3_1 = src3 -8;  //offset by stride=0 bytes
  src3_2 = src3 -4;

  dst_1 = dest -8;  //offset by stride=0 bytes
  dst_2 = dest -4;

  inc=64;

  y=(num-16)/16;

  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src0_1,inc,4);
  VECTOR_LOAD(src1_1,inc,5);
  VECTOR_LOAD(src2_1,inc,6);
  VECTOR_LOAD(src3_1,inc,7);

  ASM("qvfadd 16, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_2,inc,8);
  VECTOR_LOAD(src1_2,inc,9);

  ASM("qvfadd 17, 2, 3" ::  );
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src2_2,inc,10);
  VECTOR_LOAD(src3_2,inc,11);
  ASM("qvfadd 22, 8, 9" ::  );


  do{
  	  ASM("qvfadd 18, 16, 17" ::  );


  	  ASM("qvfadd 19, 4, 5" ::  );
	  VECTOR_LOAD(src0_1,inc,4);
	  VECTOR_LOAD(src1_1,inc,5);
	  VECTOR_LOAD(src0_2,inc,12);
	  VECTOR_LOAD(src1_2,inc,13);

  	  ASM("qvfadd 23, 10, 11" ::  );
      VECTOR_STORE(dst_1,inc,18);

  	  ASM("qvfadd 20, 6, 7" ::  );
	  VECTOR_LOAD(src2_1,inc,6);
	  VECTOR_LOAD(src3_1,inc,7);
	  VECTOR_LOAD(src2_2,inc,14);
	  VECTOR_LOAD(src3_2,inc,15);

  	  ASM("qvfadd 25, 12, 13" ::  );

  	  ASM("qvfadd 24, 22, 23" ::  );

  	  ASM("qvfadd 21, 19, 20" ::  );

 	  if(!--y) break;

  	  ASM("qvfadd 16, 0, 1" ::  );
	  VECTOR_LOAD(src0_1,inc,0);
	  VECTOR_LOAD(src1_1,inc,1);
  	  ASM("qvfadd 26, 14, 15" ::  );
	  VECTOR_LOAD(src0_2,inc,8);
	  VECTOR_LOAD(src1_2,inc,9);

	  VECTOR_STORE(dst_2,inc,24);

	  VECTOR_STORE(dst_1,inc,21);

  	  ASM("qvfadd 17, 2, 3" ::  );
  	  ASM("qvfadd 27, 25, 26" ::  );
	  VECTOR_LOAD(src2_1,inc,2);
	  VECTOR_LOAD(src3_1,inc,3);
	  VECTOR_LOAD(src2_2,inc,10);
	  VECTOR_LOAD(src3_2,inc,11);
  	  ASM("qvfadd 22, 8, 9" ::  );
	  VECTOR_STORE(dst_2,inc,27);

   }while(1);

	  VECTOR_STORE(dst_2,inc,24);

  	  ASM("qvfadd 26, 14, 15" ::  );

  	  ASM("qvfadd 16, 0, 1" ::  );
  	  ASM("qvfadd 17, 2, 3" ::  );
	  VECTOR_LOAD(src0_2,inc,8);
	  VECTOR_LOAD(src1_2,inc,9);
  	  ASM("qvfadd 22, 8, 9" ::  );

	  VECTOR_STORE(dst_1,inc,21);
  	  ASM("qvfadd 27, 25, 26" ::  );

	  VECTOR_LOAD(src2_2,inc,10);
	  VECTOR_LOAD(src3_2,inc,11);
  	  ASM("qvfadd 23, 10, 11" ::  );

  	  ASM("qvfadd 18, 16, 17" ::  );
  	  ASM("qvfadd 24, 22, 23" ::  );
	  VECTOR_STORE(dst_2,inc,27);

  	  ASM("qvfadd 19, 4, 5" ::  );
	  VECTOR_LOAD(src0_2,inc,12);
	  VECTOR_LOAD(src1_2,inc,13);

	  VECTOR_STORE(dst_1,inc,18);
	  VECTOR_STORE(dst_2,inc,24);

  	  ASM("qvfadd 20, 6, 7" ::  );
	  VECTOR_LOAD(src2_2,inc,14);
	  VECTOR_LOAD(src3_2,inc,15);
  	  ASM("qvfadd 25, 12, 13" ::  );
  	  ASM("qvfadd 26, 14, 15" ::  );


  	  ASM("qvfadd 21, 19, 20" ::  );
  	  ASM("qvfadd 27, 25, 26" ::  );

	  VECTOR_STORE(dst_1,inc,21);
	  VECTOR_STORE(dst_2,inc,27);

  return 0;
}

#endif
