/**
 * \file components/devices/shmemcoll/quad_sum.h
 * \brief ???
 */
#ifndef __components_devices_shmemcoll_quad_sum_h__
#define __components_devices_shmemcoll_quad_sum_h__

#include "sum.h"

#ifdef __GNUC__
#define __MYASM__(p) asm(p)
#else
#define __MYASM__(p) /* nothing */
#endif

#define ASM asm volatile


//copying 'num' doubles
inline int quad_double_sum_4way( double* dest, double* src0, double *src1, double* src2, double* src3, uint64_t num )
{

  double *src0_1, *src0_2, *src1_1, *src1_2, *src2_1, *src2_2, *src3_1, *src3_2;
  double *dst_1, *dst_2;
  uint64_t y;

  register int inc __MYASM__("r7");


  //register double f0=0.0, f1=0.0, f2=0.0, f3=0.0, f4=0.0, f5=0.0, f6=0.0, f7=0.0, f8=0.0, f9=0.0, f10=0.0, f11=0.0, f12=0.0;
  //register double f13=0.0, f14=0.0, f15=0.0, f16=0.0, f17=0.0, f18=0.0, f19=0.0, f20=0.0, f21=0.0, f22=0.0, f23=0.0, f24=0.0, f25=0.0, f26=0.0, f27=0.0;

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

  y=(num-128)/128;

  // may speed up the 2nd to 7th load by using non-update versions of load, tbd later...
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


#if 1
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
#endif
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

#if 0
//adding 'num' doubles
int quad_double_sum_8way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, uint64_t num )
{

  double *src0_1, *src0_2, *src1_1, *src1_2, *src2_1, *src2_2, *src3_1, *src3_2;
  double *src4_1, *src4_2, *src5_1, *src5_2, *src6_1, *src6_2, *src7_1, *src7_2;
  double *dst_1, *dst_2;
  register int inc __MYASM__("r7");
  uint32_t	y;

  src0_1 = src0 -8;  //offset by stride=0 bytes
  src0_2 = src0 -4;

  src1_1 = src1 -8;  //offset by stride=0 bytes
  src1_2 = src1 -4;

  src2_1 = src2 -8;  //offset by stride=0 bytes
  src2_2 = src2 -4;

  src3_1 = src3 -8;  //offset by stride=0 bytes
  src3_2 = src3 -4;

  src4_1 = src4 -8;  //offset by stride=0 bytes
  src4_2 = src4 -4;

  src5_1 = src5 -8;  //offset by stride=0 bytes
  src5_2 = src5 -4;

  src6_1 = src6 -8;  //offset by stride=0 bytes
  src6_2 = src6 -4;

  src7_1 = src7 -8;  //offset by stride=0 bytes
  src7_2 = src7 -4;

  dst_1 = dest -8;  //offset by stride=0 bytes
  dst_2 = dest -4;

  inc=64;

  y= num/8 - 1;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);

  ASM("qvfadd 16, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_2,inc,8);
  VECTOR_LOAD(src1_2,inc,9);

  ASM("qvfadd 17, 4, 5" ::  );
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src4_2,inc,12);
  VECTOR_LOAD(src5_2,inc,13);

  ASM("qvfadd 16, 16, 2" ::  );
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src2_2,inc,10);
  ASM("qvfadd 30, 8, 9" ::  );

  ASM("qvfadd 17, 17, 6" ::  );
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src6_2,inc,14);
  ASM("qvfadd 31, 12, 13" ::  );

  ASM("qvfadd 16, 16, 3" ::  );
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src3_2,inc,11);
  ASM("qvfadd 30, 30, 10" ::  );

  while (1) {


  ASM("qvfadd 17, 17, 7" ::  );
  VECTOR_LOAD(src7_1,inc,7);
  VECTOR_LOAD(src7_2,inc,15);
  ASM("qvfadd 31, 31, 14" ::  );


  if (!--y) break;

  ASM("qvfadd 17, 17, 16" ::  );
  ASM("qvfadd 16, 1, 0" ::  );
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_2,inc,8);
  VECTOR_LOAD(src1_2,inc,9);
  ASM("qvfadd 30, 30, 11" ::  );
  ASM("qvfadd 31, 31, 15" ::  );

  VECTOR_STORE(dst_1,inc,17);
  ASM("qvfadd 17, 5, 4" ::  );
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src4_2,inc,12);
  VECTOR_LOAD(src5_2,inc,13);

  ASM("qvfadd 31, 31, 30" ::  );
  ASM("qvfadd 16, 16, 2" ::  );
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src2_2,inc,10);
  ASM("qvfadd 30, 8, 9" ::  );

  ASM("qvfadd 17, 17, 6" ::  );
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src6_2,inc,14);

  VECTOR_STORE(dst_2,inc,31);
  ASM("qvfadd 31, 12, 13" ::  );
  ASM("qvfadd 16, 16, 3" ::  );
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src3_2,inc,11);
  ASM("qvfadd 30, 30, 10" ::  );

  }

  ASM("qvfadd 17, 17, 16" ::  );
  ASM("qvfadd 16, 1, 0" ::  );
  VECTOR_LOAD(src0_2,inc,8);
  VECTOR_LOAD(src1_2,inc,9);
  ASM("qvfadd 30, 30, 11" ::  );
  ASM("qvfadd 31, 31, 15" ::  );

  VECTOR_STORE(dst_1,inc,17);
  ASM("qvfadd 17, 5, 4" ::  );
  ASM("qvfadd 16, 16, 2" ::  );
  VECTOR_LOAD(src4_2,inc,12);
  VECTOR_LOAD(src5_2,inc,13);

  ASM("qvfadd 31, 31, 30" ::  );
  ASM("qvfadd 30, 8, 9" ::  );
  ASM("qvfadd 17, 17, 6" ::  );
  VECTOR_LOAD(src2_2,inc,10);
  VECTOR_LOAD(src6_2,inc,14);
  VECTOR_STORE(dst_2,inc,31);
  ASM("qvfadd 31, 12, 13" ::  );

  ASM("qvfadd 17, 17, 7" ::  );
  ASM("qvfadd 16, 16, 3" ::  );
  VECTOR_LOAD(src3_2,inc,11);
  ASM("qvfadd 30, 30, 10" ::  );
  VECTOR_LOAD(src7_2,inc,15);
  ASM("qvfadd 31, 31, 14" ::  );

  ASM("qvfadd 17, 17, 16" ::  );
  VECTOR_STORE(dst_1,inc,17);

  ASM("qvfadd 30, 30, 11" ::  );
  ASM("qvfadd 31, 31, 15" ::  );
  ASM("qvfadd 31, 31, 30" ::  );
  VECTOR_STORE(dst_2,inc,31);

  return 0;
}
#endif

#if 0
int quad_double_sum_8way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, uint64_t num )
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *dst_1, *dst_2;
  register int inc __MYASM__("r7");
  register int inc1 __MYASM__("r8");
  uint32_t	y;

  src0_1 = src0 -4;
  src1_1 = src1 -4;
  src2_1 = src2 -4;
  src3_1 = src3 -4;
  src4_1 = src4 -4;
  src5_1 = src5 -4;
  src6_1 = src6 -4;
  src7_1 = src7 -4;

  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=32;
  inc1=64;

  y= num/8 - 1;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);

  ASM("qvfadd 16, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,8);
  VECTOR_LOAD(src1_1,inc,9);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);

  ASM("qvfadd 17, 4, 5" ::  );
  VECTOR_LOAD(src4_1,inc,12);
  VECTOR_LOAD(src5_1,inc,13);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);

  ASM("qvfadd 16, 16, 2" ::  );
  VECTOR_LOAD(src2_1,inc,10);
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 8, 9" ::  );

  ASM("qvfadd 17, 17, 6" ::  );
  VECTOR_LOAD(src6_1,inc,14);
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 31, 12, 13" ::  );

  ASM("qvfadd 16, 16, 3" ::  );
  VECTOR_LOAD(src3_1,inc,11);
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 10" ::  );

  while (1) {


  ASM("qvfadd 17, 17, 7" ::  );
  VECTOR_LOAD(src7_1,inc,15);
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 31, 31, 14" ::  );


  if (!--y) break;

  ASM("qvfadd 17, 17, 16" ::  );
  ASM("qvfadd 16, 1, 0" ::  );
  VECTOR_LOAD(src0_1,inc,8);
  VECTOR_LOAD(src1_1,inc,9);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 30, 30, 11" ::  );
  ASM("qvfadd 31, 31, 15" ::  );

  VECTOR_STORE(dst_1,inc1,17);
  ASM("qvfadd 17, 5, 4" ::  );
  VECTOR_LOAD(src4_1,inc,12);
  VECTOR_LOAD(src5_1,inc,13);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);

  ASM("qvfadd 31, 31, 30" ::  );
  ASM("qvfadd 16, 16, 2" ::  );
  VECTOR_LOAD(src2_1,inc,10);
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 8, 9" ::  );

  ASM("qvfadd 17, 17, 6" ::  );
  VECTOR_LOAD(src6_1,inc,14);
  VECTOR_LOAD(src6_1,inc,6);

  VECTOR_STORE(dst_2,inc1,31);
  ASM("qvfadd 31, 12, 13" ::  );
  ASM("qvfadd 16, 16, 3" ::  );
  VECTOR_LOAD(src3_1,inc,11);
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 10" ::  );

  }

  ASM("qvfadd 17, 17, 16" ::  );
  ASM("qvfadd 16, 1, 0" ::  );
  VECTOR_LOAD(src0_1,inc,8);
  VECTOR_LOAD(src1_1,inc,9);
  ASM("qvfadd 30, 30, 11" ::  );
  ASM("qvfadd 31, 31, 15" ::  );

  VECTOR_STORE(dst_1,inc1,17);
  ASM("qvfadd 17, 5, 4" ::  );
  ASM("qvfadd 16, 16, 2" ::  );
  VECTOR_LOAD(src4_1,inc,12);
  VECTOR_LOAD(src5_1,inc,13);

  ASM("qvfadd 31, 31, 30" ::  );
  ASM("qvfadd 30, 8, 9" ::  );
  ASM("qvfadd 17, 17, 6" ::  );
  VECTOR_LOAD(src2_1,inc,10);
  VECTOR_LOAD(src6_1,inc,14);
  VECTOR_STORE(dst_2,inc1,31);
  ASM("qvfadd 31, 12, 13" ::  );

  ASM("qvfadd 17, 17, 7" ::  );
  ASM("qvfadd 16, 16, 3" ::  );
  VECTOR_LOAD(src3_1,inc,11);
  ASM("qvfadd 30, 30, 10" ::  );
  VECTOR_LOAD(src7_1,inc,15);
  ASM("qvfadd 31, 31, 14" ::  );

  ASM("qvfadd 17, 17, 16" ::  );
  VECTOR_STORE(dst_1,inc1,17);

  ASM("qvfadd 30, 30, 11" ::  );
  ASM("qvfadd 31, 31, 15" ::  );
  ASM("qvfadd 31, 31, 30" ::  );
  VECTOR_STORE(dst_2,inc1,31);

  return 0;
}
#endif

//version 1
//Derived from double_sum_8way below
//register set used for the pointers
// {0..15} -> {src0,....src15}
// {16..19} -> 2nd_half{src0,..src7}
// {20..23} -> 2nd_half{src8,..src15}
// (28) - Sum of first half from src0...src7
// (29) - Sum of first half from src8...src15
// (30) - Sum of second half from src0...src7
// (31) - Sum of second half from src8...src15
#if 0
inline int quad_double_sum_16way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, double* src8, double* src9,
			double*	src10, double* src11, double* src12, double* src13, double* src14, double* src15,
			uint64_t num )
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src8_1, *src9_1, *src10_1, *src11_1, *src12_1, *src13_1, *src14_1, *src15_1;
  double *dst_1, *dst_2;
  register int inc __MYASM__("r7");
  register int inc1 __MYASM__("r8");
  uint32_t	y;

  src0_1 = src0 -4;
  src1_1 = src1 -4;
  src2_1 = src2 -4;
  src3_1 = src3 -4;
  src4_1 = src4 -4;
  src5_1 = src5 -4;
  src6_1 = src6 -4;
  src7_1 = src7 -4;
  src8_1 = src8 -4;
  src9_1 = src9 -4;
  src10_1 = src10 -4;
  src11_1 = src11 -4;
  src12_1 = src12 -4;
  src13_1 = src13 -4;
  src14_1 = src14 -4;
  src15_1 = src15 -4;

  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=32;
  inc1=64;

  y= num/8 - 1;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);
  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src9_1,inc,9);
  VECTOR_LOAD(src10_1,inc,10);
  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src12_1,inc,22);
  VECTOR_LOAD(src13_1,inc,23);
  VECTOR_LOAD(src14_1,inc,24);
  VECTOR_LOAD(src15_1,inc,25);

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);

  ASM("qvfadd 29, 8, 9" ::  );
  VECTOR_LOAD(src8_1,inc,20);
  VECTOR_LOAD(src9_1,inc,21);
  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src9_1,inc,9);

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 16, 17" ::  );

  ASM("qvfadd 29, 29, 10" ::  );
  VECTOR_LOAD(src10_1,inc,22);
  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 31, 20, 21" ::  );

  ASM("qvfadd 28, 28, 3" ::  );
  VECTOR_LOAD(src3_1,inc,19);
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 18" ::  );

  ASM("qvfadd 29, 29, 11" ::  );
  VECTOR_LOAD(src11_1,inc,23);
  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfadd 31, 31, 22" ::  );

  ASM("qvfadd 28, 28, 4" ::  ); 			//Extension
  VECTOR_LOAD(src4_1,inc,16);
  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 30, 30, 19" ::  );

  ASM("qvfadd 29, 29, 12" ::  );			//Extension
  VECTOR_LOAD(src12_1,inc,20);
  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 31, 31, 23" ::  );

  ASM("qvfadd 28, 28, 5" ::  ); 			//Extension
  VECTOR_LOAD(src5_1,inc,17);
  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 30, 30, 16" ::  );

  ASM("qvfadd 29, 29, 13" ::  );			//Extension
  VECTOR_LOAD(src13_1,inc,21);
  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 31, 31, 20" ::  );

  ASM("qvfadd 28, 28, 6" ::  ); 			//Extension
  VECTOR_LOAD(src6_1,inc,18);
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 30, 30, 17" ::  );

  ASM("qvfadd 29, 29, 14" ::  );			//Extension
  VECTOR_LOAD(src14_1,inc,22);
  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 31, 31, 21" ::  );

  ASM("qvfadd 28, 28, 7" ::  ); 			//Extension
  VECTOR_LOAD(src7_1,inc,19);
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 30, 30, 18" ::  );


  while (1) {

  ASM("qvfadd 29, 29, 15" ::  );			//Extension
  VECTOR_LOAD(src15_1,inc,23);
  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 31, 31, 22" ::  );


  if (!--y) break;

  ASM("qvfadd 29, 29, 28" ::  );

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 30, 30, 19" ::  );


  ASM("qvfadd 31, 31, 23" ::  );
  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 29, 8, 9" ::  );
  VECTOR_LOAD(src8_1,inc,20);
  VECTOR_LOAD(src9_1,inc,21);
  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src9_1,inc,9);
  //ASM("qvfadd 31, 31, 23" ::  );


 // ASM("qvfadd 31, 31, 30" ::  );

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 31, 31, 30" ::  );
  ASM("qvfadd 30, 16, 17" ::  );

 // VECTOR_STORE(dst_2,inc1,31);

  ASM("qvfadd 29, 29, 10" ::  );
  VECTOR_LOAD(src10_1,inc,22);
  VECTOR_LOAD(src10_1,inc,10);
  VECTOR_STORE(dst_2,inc1,31);
  ASM("qvfadd 31, 20, 21" ::  );

  ASM("qvfadd 28, 28, 3" ::  );
  VECTOR_LOAD(src3_1,inc,19);
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 18" ::  );

  ASM("qvfadd 29, 29, 11" ::  );
  VECTOR_LOAD(src11_1,inc,23);
  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfadd 31, 31, 22" ::  );

  ASM("qvfadd 28, 28, 4" ::  ); 			//Extension
  VECTOR_LOAD(src4_1,inc,16);
  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 30, 30, 19" ::  );

  ASM("qvfadd 29, 29, 12" ::  );			//Extension
  VECTOR_LOAD(src12_1,inc,20);
  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 31, 31, 23" ::  );

  ASM("qvfadd 28, 28, 5" ::  ); 			//Extension
  VECTOR_LOAD(src5_1,inc,17);
  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 30, 30, 16" ::  );

  ASM("qvfadd 29, 29, 13" ::  );			//Extension
  VECTOR_LOAD(src13_1,inc,21);
  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 31, 31, 20" ::  );

  ASM("qvfadd 28, 28, 6" ::  ); 			//Extension
  VECTOR_LOAD(src6_1,inc,18);
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 30, 30, 17" ::  );

  ASM("qvfadd 29, 29, 14" ::  );			//Extension
  VECTOR_LOAD(src14_1,inc,22);
  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 31, 31, 21" ::  );

  ASM("qvfadd 28, 28, 7" ::  ); 			//Extension
  VECTOR_LOAD(src7_1,inc,19);
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 30, 30, 18" ::  );



  }

 //Loaded first half from all..start summing the first halves
 //Load second half from all..sum all the second halves

  ASM("qvfadd 29, 29, 28" ::  );

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 30, 30, 19" ::  );


  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 29, 8, 9" ::  );
  VECTOR_LOAD(src8_1,inc,20);
  VECTOR_LOAD(src9_1,inc,21);
  ASM("qvfadd 31, 31, 23" ::  );


  ASM("qvfadd 31, 31, 30" ::  );

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 30, 16, 17" ::  );

  VECTOR_STORE(dst_2,inc1,31);

  ASM("qvfadd 29, 29, 10" ::  );
  VECTOR_LOAD(src10_1,inc,22);
  ASM("qvfadd 31, 20, 21" ::  );

  ASM("qvfadd 28, 28, 3" ::  );
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 30, 30, 18" ::  );

  ASM("qvfadd 29, 29, 11" ::  );
  VECTOR_LOAD(src11_1,inc,23);
  ASM("qvfadd 31, 31, 22" ::  );

  ASM("qvfadd 28, 28, 4" ::  ); 			//Extension
  VECTOR_LOAD(src4_1,inc,16);
  ASM("qvfadd 30, 30, 19" ::  );

  ASM("qvfadd 29, 29, 12" ::  );			//Extension
  VECTOR_LOAD(src12_1,inc,20);
  ASM("qvfadd 31, 31, 23" ::  );

  ASM("qvfadd 28, 28, 5" ::  ); 			//Extension
  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 30, 30, 16" ::  );

  ASM("qvfadd 29, 29, 13" ::  );			//Extension
  VECTOR_LOAD(src13_1,inc,21);
  ASM("qvfadd 31, 31, 20" ::  );

  ASM("qvfadd 28, 28, 6" ::  ); 			//Extension
  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 30, 30, 17" ::  );

  ASM("qvfadd 29, 29, 14" ::  );			//Extension
  VECTOR_LOAD(src14_1,inc,22);
  ASM("qvfadd 31, 31, 21" ::  );

  ASM("qvfadd 28, 28, 7" ::  ); 			//Extension
  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 30, 30, 18" ::  );

  ASM("qvfadd 29, 29, 15" ::  );			//Extension
  VECTOR_LOAD(src15_1,inc,23);
  ASM("qvfadd 31, 31, 22" ::  );


  ASM("qvfadd 29, 29, 28" ::  );

  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 30, 30, 19" ::  );

  ASM("qvfadd 31, 31, 23" ::  );
  ASM("qvfadd 31, 31, 30" :: );
  VECTOR_STORE(dst_2,inc1,31);

  return 0;
}
#endif

//version 2..time shifting the 2nd loads

#if 0
inline int quad_double_sum_16way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, double* src8, double* src9,
			double*	src10, double* src11, double* src12, double* src13, double* src14, double* src15,
			uint64_t num )
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src8_1, *src9_1, *src10_1, *src11_1, *src12_1, *src13_1, *src14_1, *src15_1;
  double *dst_1, *dst_2;
  //register int inc __MYASM__("r7");
  //register int inc __MYASM__("r7");
  register int inc ;
  register int inc1;
  register int	y;

  src0_1 = src0 -4;
  src1_1 = src1 -4;
  src2_1 = src2 -4;
  src3_1 = src3 -4;
  src4_1 = src4 -4;
  src5_1 = src5 -4;
  src6_1 = src6 -4;
  src7_1 = src7 -4;
  src8_1 = src8 -4;
  src9_1 = src9 -4;
  src10_1 = src10 -4;
  src11_1 = src11 -4;
  src12_1 = src12 -4;
  src13_1 = src13 -4;
  src14_1 = src14 -4;
  src15_1 = src15 -4;

  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=32;
  inc1=64;

  y= num/8 - 1;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);
  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src9_1,inc,9);
  VECTOR_LOAD(src10_1,inc,10);
  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src12_1,inc,22);
  VECTOR_LOAD(src13_1,inc,23);
  VECTOR_LOAD(src14_1,inc,24);
  VECTOR_LOAD(src15_1,inc,25);

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);

  ASM("qvfadd 29, 8, 9" ::  );
  VECTOR_LOAD(src8_1,inc,20);
  VECTOR_LOAD(src9_1,inc,21);

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 30, 16, 17" ::  );

  ASM("qvfadd 29, 29, 10" ::  );
  VECTOR_LOAD(src10_1,inc,22);
  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 31, 20, 21" ::  );

  ASM("qvfadd 28, 28, 3" ::  );
  VECTOR_LOAD(src3_1,inc,19);
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 30, 18" ::  );

  ASM("qvfadd 29, 29, 11" ::  );
  VECTOR_LOAD(src11_1,inc,23);
  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 31, 31, 22" ::  );

  ASM("qvfadd 28, 28, 4" ::  ); 			//Extension
  VECTOR_LOAD(src4_1,inc,16);
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 19" ::  );

  ASM("qvfadd 29, 29, 12" ::  );			//Extension
  VECTOR_LOAD(src12_1,inc,20);
  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfadd 31, 31, 23" ::  );

  ASM("qvfadd 28, 28, 5" ::  ); 			//Extension
  VECTOR_LOAD(src5_1,inc,17);
  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 30, 30, 16" ::  );

  ASM("qvfadd 29, 29, 13" ::  );			//Extension
  VECTOR_LOAD(src13_1,inc,21);
  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 31, 31, 20" ::  );

  ASM("qvfadd 28, 28, 6" ::  ); 			//Extension
  VECTOR_LOAD(src6_1,inc,18);
  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 30, 30, 17" ::  );

  ASM("qvfadd 29, 29, 14" ::  );			//Extension
  VECTOR_LOAD(src14_1,inc,22);
  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 31, 31, 21" ::  );

  ASM("qvfadd 28, 28, 7" ::  ); 			//Extension
  VECTOR_LOAD(src7_1,inc,19);
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 30, 30, 18" ::  );


  while (1) {

  ASM("qvfadd 29, 29, 15" ::  );			//Extension
  VECTOR_LOAD(src15_1,inc,23);
  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 31, 31, 22" ::  );


  if (!--y) break;

  ASM("qvfadd 29, 29, 28" ::  );

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 30, 30, 19" ::  );


  ASM("qvfadd 31, 31, 23" ::  );
  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 29, 8, 9" ::  );
  VECTOR_LOAD(src8_1,inc,20);
  VECTOR_LOAD(src9_1,inc,21);
  VECTOR_LOAD(src15_1,inc,15);
  //ASM("qvfadd 31, 31, 23" ::  );


 // ASM("qvfadd 31, 31, 30" ::  );

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 31, 31, 30" ::  );
  ASM("qvfadd 30, 16, 17" ::  );

 // VECTOR_STORE(dst_2,inc1,31);

  ASM("qvfadd 29, 29, 10" ::  );
  VECTOR_LOAD(src10_1,inc,22);
  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src9_1,inc,9);
  VECTOR_STORE(dst_2,inc1,31);
  ASM("qvfadd 31, 20, 21" ::  );

  ASM("qvfadd 28, 28, 3" ::  );
  VECTOR_LOAD(src3_1,inc,19);
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 30, 18" ::  );

  ASM("qvfadd 29, 29, 11" ::  );
  VECTOR_LOAD(src11_1,inc,23);
  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 31, 31, 22" ::  );

  ASM("qvfadd 28, 28, 4" ::  ); 			//Extension
  VECTOR_LOAD(src4_1,inc,16);
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 19" ::  );

  ASM("qvfadd 29, 29, 12" ::  );			//Extension
  VECTOR_LOAD(src12_1,inc,20);
  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfadd 31, 31, 23" ::  );

  ASM("qvfadd 28, 28, 5" ::  ); 			//Extension
  VECTOR_LOAD(src5_1,inc,17);
  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 30, 30, 16" ::  );

  ASM("qvfadd 29, 29, 13" ::  );			//Extension
  VECTOR_LOAD(src13_1,inc,21);
  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 31, 31, 20" ::  );

  ASM("qvfadd 28, 28, 6" ::  ); 			//Extension
  VECTOR_LOAD(src6_1,inc,18);
  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 30, 30, 17" ::  );

  ASM("qvfadd 29, 29, 14" ::  );			//Extension
  VECTOR_LOAD(src14_1,inc,22);
  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 31, 31, 21" ::  );

  ASM("qvfadd 28, 28, 7" ::  ); 			//Extension
  VECTOR_LOAD(src7_1,inc,19);
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 30, 30, 18" ::  );



  }

 //Loaded first half from all..start summing the first halves
 //Load second half from all..sum all the second halves

  ASM("qvfadd 29, 29, 28" ::  );

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 30, 30, 19" ::  );


  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 29, 8, 9" ::  );
  VECTOR_LOAD(src8_1,inc,20);
  VECTOR_LOAD(src9_1,inc,21);
  ASM("qvfadd 31, 31, 23" ::  );


  ASM("qvfadd 31, 31, 30" ::  );

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 30, 16, 17" ::  );

  VECTOR_STORE(dst_2,inc1,31);

  ASM("qvfadd 29, 29, 10" ::  );
  VECTOR_LOAD(src10_1,inc,22);
  ASM("qvfadd 31, 20, 21" ::  );

  ASM("qvfadd 28, 28, 3" ::  );
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 30, 30, 18" ::  );

  ASM("qvfadd 29, 29, 11" ::  );
  VECTOR_LOAD(src11_1,inc,23);
  ASM("qvfadd 31, 31, 22" ::  );

  ASM("qvfadd 28, 28, 4" ::  ); 			//Extension
  VECTOR_LOAD(src4_1,inc,16);
  ASM("qvfadd 30, 30, 19" ::  );

  ASM("qvfadd 29, 29, 12" ::  );			//Extension
  VECTOR_LOAD(src12_1,inc,20);
  ASM("qvfadd 31, 31, 23" ::  );

  ASM("qvfadd 28, 28, 5" ::  ); 			//Extension
  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 30, 30, 16" ::  );

  ASM("qvfadd 29, 29, 13" ::  );			//Extension
  VECTOR_LOAD(src13_1,inc,21);
  ASM("qvfadd 31, 31, 20" ::  );

  ASM("qvfadd 28, 28, 6" ::  ); 			//Extension
  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 30, 30, 17" ::  );

  ASM("qvfadd 29, 29, 14" ::  );			//Extension
  VECTOR_LOAD(src14_1,inc,22);
  ASM("qvfadd 31, 31, 21" ::  );

  ASM("qvfadd 28, 28, 7" ::  ); 			//Extension
  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 30, 30, 18" ::  );

  ASM("qvfadd 29, 29, 15" ::  );			//Extension
  VECTOR_LOAD(src15_1,inc,23);
  ASM("qvfadd 31, 31, 22" ::  );


  ASM("qvfadd 29, 29, 28" ::  );

  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 30, 30, 19" ::  );

  ASM("qvfadd 31, 31, 23" ::  );
  ASM("qvfadd 31, 31, 30" :: );
  VECTOR_STORE(dst_2,inc1,31);

  return 0;
}
#endif

//register set used for the pointers
// {0..15} -> {src0,....src15}
// {16..19} -> 2nd_half{src0,..src8}
// {20..23} -> 2nd_half{src9,..src15}

#if 0
int quad_double_sum_8way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src8, double* src9, double* src10, double* src11, uint64_t num )
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src8_1, *src9_1, *src10_1, *src11_1, *src12_1, *src13_1, *src14_1, *src15_1;
  double *dst_1, *dst_2;
  register int inc __MYASM__("r7");
  register int inc1 __MYASM__("r8");
  uint32_t	y;

  src0_1 = src0 -4;
  src1_1 = src1 -4;
  src2_1 = src2 -4;
  src3_1 = src3 -4;

  src8_1 = src8 -4;
  src9_1 = src9 -4;
  src10_1 = src10 -4;
  src11_1 = src11 -4;

  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=32;
  inc1=64;

  y= num/8 - 1;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);

  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src9_1,inc,9);
  VECTOR_LOAD(src10_1,inc,10);
  VECTOR_LOAD(src11_1,inc,11);

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);

  ASM("qvfadd 29, 8, 9" ::  );
  VECTOR_LOAD(src8_1,inc,20);
  VECTOR_LOAD(src9_1,inc,21);
  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src9_1,inc,9);

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 16, 17" ::  );

  ASM("qvfadd 29, 29, 10" ::  );
  VECTOR_LOAD(src10_1,inc,22);
  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 31, 20, 21" ::  );

  ASM("qvfadd 28, 28, 3" ::  );
  VECTOR_LOAD(src3_1,inc,19);
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 18" ::  );

  while (1) {


  ASM("qvfadd 29, 29, 11" ::  );
  VECTOR_LOAD(src11_1,inc,23);
  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfadd 31, 31, 22" ::  );


  if (!--y) break;

  ASM("qvfadd 29, 29, 28" ::  );

  ASM("qvfadd 28, 0, 1" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_LOAD(src1_1,inc,17);
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 30, 30, 19" ::  );


  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 29, 8, 9" ::  );
  VECTOR_LOAD(src8_1,inc,20);
  VECTOR_LOAD(src9_1,inc,21);
  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 31, 31, 23" ::  );


  ASM("qvfadd 31, 31, 30" ::  );

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 16, 17" ::  );

  VECTOR_STORE(dst_2,inc1,31);

  ASM("qvfadd 29, 29, 10" ::  );
  VECTOR_LOAD(src10_1,inc,22);
  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 31, 20, 21" ::  );

  ASM("qvfadd 28, 28, 3" ::  );
  VECTOR_LOAD(src3_1,inc,19);
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 18" ::  );

  }

 //Loaded first half from all..start summing the first halves
 //Load second half from all..sum all the second halves

  ASM("qvfadd 29, 29, 28" :: );
  ASM("qvfadd 28, 1, 0" ::  );
  VECTOR_LOAD(src0_1,inc,16);  //2nd half
  VECTOR_LOAD(src1_1,inc,17);	// 2nd half
  ASM("qvfadd 30, 30, 19" :: );
  ASM("qvfadd 31, 31, 23" :: );

  VECTOR_STORE(dst_1,inc1,29);
  ASM("qvfadd 29, 8, 9" ::  );
  ASM("qvfadd 28, 28, 2" :: );
  VECTOR_LOAD(src8_1,inc,20);  //2nd half
  VECTOR_LOAD(src9_1,inc,21);  //2nd half

  ASM("qvfadd 31, 31, 30" :: );
  ASM("qvfadd 30, 16, 17" :: );
  ASM("qvfadd 29, 29, 10" :: );

  VECTOR_LOAD(src2_1,inc,18); //2nd half
  VECTOR_LOAD(src10_1,inc,22);
  VECTOR_STORE(dst_2,inc1,31);
  ASM("qvfadd 31, 20, 21" :: );

  ASM("qvfadd 29, 29, 11" :: );
  ASM("qvfadd 28, 28, 3" ::  );

  VECTOR_LOAD(src3_1,inc,19); //2nd half
  ASM("qvfadd 30, 30, 18" :: );
  VECTOR_LOAD(src11_1,inc,23);
  ASM("qvfadd 31, 31, 22" :: );

  ASM("qvfadd 29, 29, 28" :: );
  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 30, 30, 19" :: );
  ASM("qvfadd 31, 31, 23" :: );
  ASM("qvfadd 31, 31, 30" :: );
  VECTOR_STORE(dst_2,inc1,31);

  return 0;
}
#endif


//martin's schedule from the xls sheet
#if 0
inline int quad_double_sum_16way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, double* src8, double* src9,
			double*	src10, double* src11, double* src12, double* src13, double* src14, double* src15,
			uint64_t num )
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src8_1, *src9_1, *src10_1, *src11_1, *src12_1, *src13_1, *src14_1, *src15_1;
  double *dst_1, *dst_2;
  //register int inc __MYASM__("r7");
  //register int inc __MYASM__("r7");
  register int inc ;
  register int inc1;
  register int	y;

  src0_1 = src0 -4;
  src1_1 = src1 -4;
  src2_1 = src2 -4;
  src3_1 = src3 -4;
  src4_1 = src4 -4;
  src5_1 = src5 -4;
  src6_1 = src6 -4;
  src7_1 = src7 -4;
  src8_1 = src8 -4;
  src9_1 = src9 -4;
  src10_1 = src10 -4;
  src11_1 = src11 -4;
  src12_1 = src12 -4;
  src13_1 = src13 -4;
  src14_1 = src14 -4;
  src15_1 = src15 -4;

  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=32;
  inc1=64;

  y= num/8;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);

  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src0_1,inc,16);
  ASM("qvfadd 28, 0, 1" ::  );


  VECTOR_LOAD(src9_1,inc,9);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 28, 28, 2" ::  );


  VECTOR_LOAD(src10_1,inc,10);
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 28, 28, 3" ::  );


  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 28, 28, 4" ::  );
  ASM("qvfadd 30, 16, 17" ::  );


  VECTOR_LOAD(src12_1,inc,12);
  VECTOR_LOAD(src4_1,inc,16);
  ASM("qvfadd 28, 28, 5" ::  );
  ASM("qvfadd 30, 30, 18" ::  );


  VECTOR_LOAD(src13_1,inc,13);
  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 28, 28, 6" ::  );
  ASM("qvfadd 30, 30, 19" ::  );


  VECTOR_LOAD(src14_1,inc,14);
  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 28, 28, 7" ::  );
  ASM("qvfadd 30, 30, 16" ::  );


  VECTOR_LOAD(src15_1,inc,15);
  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 28, 28, 8" ::  );
  ASM("qvfadd 30, 30, 17" ::  );

  while (1)
  {

	if (!--y) break;
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src8_1,inc,16);
  ASM("qvfadd 28, 28, 9" ::  );
  ASM("qvfadd 30, 30, 18" ::  );

  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src9_1,inc,17);
  ASM("qvfadd 28, 28, 10" ::  );
  ASM("qvfadd 30, 30, 19" ::  );

  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src10_1,inc,18);
  ASM("qvfadd 28, 28, 11" ::  );
  ASM("qvfadd 30, 30, 16" ::  );

  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src11_1,inc,19);
  ASM("qvfadd 28, 28, 12" ::  );
  ASM("qvfadd 30, 30, 17" ::  );

  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src12_1,inc,16);
  ASM("qvfadd 28, 28, 13" ::  );
  ASM("qvfadd 30, 30, 18" ::  );

  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src13_1,inc,17);
  ASM("qvfadd 28, 28, 14" ::  );
  ASM("qvfadd 30, 30, 19" ::  );

  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src14_1,inc,18);
  ASM("qvfadd 28, 28, 15" ::  ); //first first-half sum done
  ASM("qvfadd 30, 30, 16" ::  );

  VECTOR_LOAD(src7_1,inc,7);
  VECTOR_LOAD(src15_1,inc,19);
  ASM("qvfadd 30, 30, 17" ::  );

  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src0_1,inc,16);
  VECTOR_STORE(dst_1,inc1,28);
  ASM("qvfadd 28, 0, 1" ::  ); 	//second first-half sum start
  ASM("qvfadd 30, 30, 18" ::  );


  VECTOR_LOAD(src9_1,inc,9);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 28, 28, 2" ::  );
  ASM("qvfadd 30, 30, 19" ::  ); //first second-half sum done

  VECTOR_LOAD(src10_1,inc,10);
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 28, 28, 3" ::  );

  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 28, 28, 4" ::  );
  VECTOR_STORE(dst_2,inc1,30);
  ASM("qvfadd 30, 16, 17" ::  ); //second second-half sum start

  VECTOR_LOAD(src12_1,inc,12);
  VECTOR_LOAD(src4_1,inc,16);
  ASM("qvfadd 28, 28, 5" ::  );
  ASM("qvfadd 30, 30, 18" ::  );


  VECTOR_LOAD(src13_1,inc,13);
  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 28, 28, 6" ::  );
  ASM("qvfadd 30, 30, 19" ::  );

  VECTOR_LOAD(src14_1,inc,14);
  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 28, 28, 7" ::  );
  ASM("qvfadd 30, 30, 16" ::  );

  VECTOR_LOAD(src15_1,inc,15);
  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 28, 28, 8" ::  );
  ASM("qvfadd 30, 30, 17" ::  );

  }

  VECTOR_LOAD(src8_1,inc,16);
  ASM("qvfadd 28, 28, 9" ::  );
  ASM("qvfadd 30, 30, 18" ::  );

  VECTOR_LOAD(src9_1,inc,17);
  ASM("qvfadd 28, 28, 10" ::  );
  ASM("qvfadd 30, 30, 19" ::  );

  VECTOR_LOAD(src10_1,inc,18);
  ASM("qvfadd 28, 28, 11" ::  );
  ASM("qvfadd 30, 30, 16" ::  );

  VECTOR_LOAD(src11_1,inc,19);
  ASM("qvfadd 28, 28, 12" ::  );
  ASM("qvfadd 30, 30, 17" ::  );

  VECTOR_LOAD(src12_1,inc,16);
  ASM("qvfadd 28, 28, 13" ::  );
  ASM("qvfadd 30, 30, 18" ::  );

  VECTOR_LOAD(src13_1,inc,17);
  ASM("qvfadd 28, 28, 14" ::  );
  ASM("qvfadd 30, 30, 19" ::  );

  VECTOR_LOAD(src14_1,inc,18);
  ASM("qvfadd 28, 28, 15" ::  ); //second first-half sum done
  ASM("qvfadd 30, 30, 16" ::  );

  VECTOR_LOAD(src15_1,inc,19);
  ASM("qvfadd 30, 30, 17" ::  );
  ASM("qvfadd 30, 30, 18" ::  );
  ASM("qvfadd 30, 30, 19" ::  );

  VECTOR_STORE(dst_1,inc1,28);
  VECTOR_STORE(dst_2,inc1,30);

  return 0;
}
#endif

/*#define NOPS_7cycles \
do { \
  asm volatile("nop"); \
  asm volatile("nop"); \
  asm volatile("nop"); \
  asm volatile("nop"); \
  asm volatile("nop"); \
  asm volatile("nop"); \
  asm volatile("nop"); \
} while(0)

#define NOPS_4cycles \
do { \
  asm volatile("nop"); \
  asm volatile("nop"); \
  asm volatile("nop"); \
  asm volatile("nop"); \
} while(0)

#define NOPS_3cycles \
do { \
  asm volatile("nop"); \
  asm volatile("nop"); \
  asm volatile("nop"); \
} while(0)

#define NOPS_1cycles \
do { \
  asm volatile("nop"); \
} while(0)*/

//martin's schedule from the xls sheet with no-ops
#if 0
inline int quad_double_sum_16way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, double* src8, double* src9,
			double*	src10, double* src11, double* src12, double* src13, double* src14, double* src15,
			uint64_t num )
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src8_1, *src9_1, *src10_1, *src11_1, *src12_1, *src13_1, *src14_1, *src15_1;
  double *dst_1, *dst_2;
  //register int inc __MYASM__("r7");
  //register int inc __MYASM__("r7");
  register int inc ;
  register int inc1;
  register int	y;

  src0_1 = src0 -4;
  src1_1 = src1 -4;
  src2_1 = src2 -4;
  src3_1 = src3 -4;
  src4_1 = src4 -4;
  src5_1 = src5 -4;
  src6_1 = src6 -4;
  src7_1 = src7 -4;
  src8_1 = src8 -4;
  src9_1 = src9 -4;
  src10_1 = src10 -4;
  src11_1 = src11 -4;
  src12_1 = src12 -4;
  src13_1 = src13 -4;
  src14_1 = src14 -4;
  src15_1 = src15 -4;

  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=32;
  inc1=64;

  y= num/8;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);

  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src0_1,inc,16);
  ASM("qvfadd 28, 0, 1" ::  );


  VECTOR_LOAD(src9_1,inc,9);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 28, 28, 2" ::  );


  VECTOR_LOAD(src10_1,inc,10);
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 28, 28, 3" ::  );


  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 28, 28, 4" ::  );
  VECTOR_LOAD(src12_1,inc,12);

  ASM("qvfadd 30, 16, 17" ::  );


  VECTOR_LOAD(src4_1,inc,16);
  ASM("qvfadd 28, 28, 5" ::  );
  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 30, 30, 18" ::  );


  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 28, 28, 6" ::  );
  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 30, 30, 19" ::  );


  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 28, 28, 7" ::  );
  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 30, 30, 16" ::  );


  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 28, 28, 8" ::  );

  while (1)
  {

  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 30, 30, 17" ::  );
  VECTOR_LOAD(src8_1,inc,16);
  ASM("qvfadd 28, 28, 9" ::  );



  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 30, 30, 18" ::  );
  VECTOR_LOAD(src9_1,inc,17);
  ASM("qvfadd 28, 28, 10" ::  );

  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 30, 19" ::  );
  VECTOR_LOAD(src10_1,inc,18);
  ASM("qvfadd 28, 28, 11" ::  );



  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 16" ::  );
  VECTOR_LOAD(src11_1,inc,19);
  ASM("qvfadd 28, 28, 12" ::  );


  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 30, 30, 17" ::  );
  VECTOR_LOAD(src12_1,inc,16);
  ASM("qvfadd 28, 28, 13" ::  );


  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 30, 30, 18" ::  );
  VECTOR_LOAD(src13_1,inc,17);
  ASM("qvfadd 28, 28, 14" ::  );


  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 30, 30, 19" ::  );
  VECTOR_LOAD(src14_1,inc,18);
  ASM("qvfadd 28, 28, 15" ::  ); //first first-half sum done


  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 30, 30, 16" ::  );
  VECTOR_LOAD(src15_1,inc,19);


  VECTOR_LOAD(src8_1,inc,8);
  ASM("qvfadd 30, 30, 17" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  ASM("qvfadd 29, 0, 1" ::  ); 	//second first-half sum start



  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 30, 30, 18" ::  );
  VECTOR_STORE(dst_1,inc1,28);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 29, 29, 2" ::  );


  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 30, 30, 19" ::  ); //first second-half sum done
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 29, 29, 3" ::  );

  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 29, 29, 4" ::  );



  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 31, 16, 17" ::  ); //second second-half sum start
  VECTOR_LOAD(src4_1,inc,16);
  ASM("qvfadd 29, 29, 5" ::  );
  VECTOR_STORE(dst_2,inc1,30);


  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 29, 29, 6" ::  );


  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 29, 29, 7" ::  );


  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 29, 29, 8" ::  );

	y-=2;
	if (!y) break;

  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src8_1,inc,16);
  ASM("qvfadd 29, 29, 9" ::  );


  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src9_1,inc,17);
  ASM("qvfadd 29, 29, 10" ::  );

  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src10_1,inc,18);
  ASM("qvfadd 29, 29, 11" ::  );

  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src11_1,inc,19);
  ASM("qvfadd 29, 29, 12" ::  );

  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src12_1,inc,16);
  ASM("qvfadd 29, 29, 13" ::  );

  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src13_1,inc,17);
  ASM("qvfadd 29, 29, 14" ::  );

  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src14_1,inc,18);
  ASM("qvfadd 29, 29, 15" ::  );

  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src15_1,inc,19);

  VECTOR_LOAD(src8_1,inc,8);
  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  ASM("qvfadd 28, 0, 1" ::  );

  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_STORE(dst_1,inc1,29);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 28, 28, 2" ::  );

  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 28, 28, 3" ::  );

  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 28, 28, 4" ::  );

  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 30, 16, 17" ::  );
  VECTOR_LOAD(src4_1,inc,16);
  VECTOR_STORE(dst_2,inc1,31);
  ASM("qvfadd 28, 28, 5" ::  );

  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 30, 30, 18" ::  );
  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 28, 28, 6" ::  );

  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 30, 30, 19" ::  );
  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 28, 28, 7" ::  );

  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 30, 30, 16" ::  );
  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 28, 28, 8" ::  );
  }

  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src8_1,inc,16);
  ASM("qvfadd 29, 29, 9" ::  );


  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src9_1,inc,17);
  ASM("qvfadd 29, 29, 10" ::  );

  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src10_1,inc,18);
  ASM("qvfadd 29, 29, 11" ::  );

  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src11_1,inc,19);
  ASM("qvfadd 29, 29, 12" ::  );

  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src12_1,inc,16);
  ASM("qvfadd 29, 29, 13" ::  );

  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src13_1,inc,17);
  ASM("qvfadd 29, 29, 14" ::  );

  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src14_1,inc,18);
  ASM("qvfadd 29, 29, 15" ::  );

  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src15_1,inc,19);

  ASM("qvfadd 31, 31, 17" ::  );

  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_STORE(dst_2,inc1,31);

}
#endif

#if 0
//strictly martin;s schedule..same register for both lower and upper halves
inline int quad_double_sum_16way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, double* src8, double* src9,
			double*	src10, double* src11, double* src12, double* src13, double* src14, double* src15,
			uint64_t num )
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src8_1, *src9_1, *src10_1, *src11_1, *src12_1, *src13_1, *src14_1, *src15_1;
  double *dst_1, *dst_2;
  //register int inc __MYASM__("r7");
  //register int inc __MYASM__("r7");
  register int inc ;
  register int inc1;
  register int	y;

  src0_1 = src0 -4;
  src1_1 = src1 -4;
  src2_1 = src2 -4;
  src3_1 = src3 -4;
  src4_1 = src4 -4;
  src5_1 = src5 -4;
  src6_1 = src6 -4;
  src7_1 = src7 -4;
  src8_1 = src8 -4;
  src9_1 = src9 -4;
  src10_1 = src10 -4;
  src11_1 = src11 -4;
  src12_1 = src12 -4;
  src13_1 = src13 -4;
  src14_1 = src14 -4;
  src15_1 = src15 -4;

  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=32;
  inc1=64;

  y= num/8;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);


  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src0_1,inc,16);
  ASM("qvfadd 28, 0, 1" ::  );


  VECTOR_LOAD(src9_1,inc,9);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 28, 28, 2" ::  );


  VECTOR_LOAD(src10_1,inc,10);
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 28, 28, 3" ::  );


  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 28, 28, 4" ::  );
  VECTOR_LOAD(src12_1,inc,12);

  ASM("qvfadd 30, 16, 17" ::  );


  VECTOR_LOAD(src4_1,inc,16);
  ASM("qvfadd 28, 28, 5" ::  );
  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 30, 30, 18" ::  );


  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 28, 28, 6" ::  );
  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 30, 30, 19" ::  );


  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 28, 28, 7" ::  );
  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 30, 30, 16" ::  );


  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 28, 28, 8" ::  );

  while (1)
  {

  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 30, 30, 17" ::  );
  VECTOR_LOAD(src8_1,inc,16);
  ASM("qvfadd 28, 28, 9" ::  );



  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 30, 30, 18" ::  );
  VECTOR_LOAD(src9_1,inc,17);
  ASM("qvfadd 28, 28, 10" ::  );

  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 30, 19" ::  );
  VECTOR_LOAD(src10_1,inc,18);
  ASM("qvfadd 28, 28, 11" ::  );



  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 16" ::  );
  VECTOR_LOAD(src11_1,inc,19);
  ASM("qvfadd 28, 28, 12" ::  );


  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 30, 30, 17" ::  );
  VECTOR_LOAD(src12_1,inc,16);
  ASM("qvfadd 28, 28, 13" ::  );


  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 30, 30, 18" ::  );
  VECTOR_LOAD(src13_1,inc,17);
  ASM("qvfadd 28, 28, 14" ::  );


  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 30, 30, 19" ::  );
  VECTOR_LOAD(src14_1,inc,18);
  ASM("qvfadd 28, 28, 15" ::  ); //first first-half sum done


  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 30, 30, 16" ::  );
  VECTOR_LOAD(src15_1,inc,19);


  VECTOR_LOAD(src8_1,inc,8);
  ASM("qvfadd 30, 30, 17" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  ASM("qvfadd 29, 0, 1" ::  ); 	//second first-half sum start



  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 30, 30, 18" ::  );
  VECTOR_STORE(dst_1,inc1,28);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 29, 29, 2" ::  );


  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 30, 30, 19" ::  ); //first second-half sum done
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 29, 29, 3" ::  );

  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 29, 29, 4" ::  );



  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 31, 16, 17" ::  ); //second second-half sum start
  VECTOR_LOAD(src4_1,inc,16);
  ASM("qvfadd 29, 29, 5" ::  );
  VECTOR_STORE(dst_2,inc1,30);


  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 29, 29, 6" ::  );


  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 29, 29, 7" ::  );


  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 29, 29, 8" ::  );

	y-=2;
	if (!y) break;

  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src8_1,inc,16);
  ASM("qvfadd 29, 29, 9" ::  );


  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src9_1,inc,17);
  ASM("qvfadd 29, 29, 10" ::  );

  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src10_1,inc,18);
  ASM("qvfadd 29, 29, 11" ::  );

  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src11_1,inc,19);
  ASM("qvfadd 29, 29, 12" ::  );

  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src12_1,inc,16);
  ASM("qvfadd 29, 29, 13" ::  );

  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src13_1,inc,17);
  ASM("qvfadd 29, 29, 14" ::  );

  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src14_1,inc,18);
  ASM("qvfadd 29, 29, 15" ::  );

  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src15_1,inc,19);

  VECTOR_LOAD(src8_1,inc,8);
  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src0_1,inc,16);
  ASM("qvfadd 28, 0, 1" ::  );

  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_STORE(dst_1,inc1,29);
  VECTOR_LOAD(src1_1,inc,17);
  ASM("qvfadd 28, 28, 2" ::  );

  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src2_1,inc,18);
  ASM("qvfadd 28, 28, 3" ::  );

  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src3_1,inc,19);
  ASM("qvfadd 28, 28, 4" ::  );

  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 30, 16, 17" ::  );
  VECTOR_LOAD(src4_1,inc,16);
  VECTOR_STORE(dst_2,inc1,31);
  ASM("qvfadd 28, 28, 5" ::  );

  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 30, 30, 18" ::  );
  VECTOR_LOAD(src5_1,inc,17);
  ASM("qvfadd 28, 28, 6" ::  );

  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 30, 30, 19" ::  );
  VECTOR_LOAD(src6_1,inc,18);
  ASM("qvfadd 28, 28, 7" ::  );

  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 30, 30, 16" ::  );
  VECTOR_LOAD(src7_1,inc,19);
  ASM("qvfadd 28, 28, 8" ::  );
  }

  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src8_1,inc,16);
  ASM("qvfadd 29, 29, 9" ::  );


  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src9_1,inc,17);
  ASM("qvfadd 29, 29, 10" ::  );

  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src10_1,inc,18);
  ASM("qvfadd 29, 29, 11" ::  );

  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src11_1,inc,19);
  ASM("qvfadd 29, 29, 12" ::  );

  ASM("qvfadd 31, 31, 17" ::  );
  VECTOR_LOAD(src12_1,inc,16);
  ASM("qvfadd 29, 29, 13" ::  );

  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_LOAD(src13_1,inc,17);
  ASM("qvfadd 29, 29, 14" ::  );

  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_LOAD(src14_1,inc,18);
  ASM("qvfadd 29, 29, 15" ::  );

  ASM("qvfadd 31, 31, 16" ::  );
  VECTOR_LOAD(src15_1,inc,19);

  ASM("qvfadd 31, 31, 17" ::  );

  ASM("qvfadd 31, 31, 18" ::  );
  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 31, 31, 19" ::  );
  VECTOR_STORE(dst_2,inc1,31);

}

#endif

#endif
