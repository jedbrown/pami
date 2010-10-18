/**
 * \file components/devices/shmemcoll/8way_sum.h
 * \brief ???
 */
#include "asmheader.h"

#ifdef __GNUC__
#define __MYASM__(p) asm(p)
#else
#define __MYASM__(p) /* nothing */
#endif

#define ASM asm volatile

#if 0
//adding 'num' doubles num >= 16 in multiples of 8
inline int quad_double_sum_8way(double* dest, double* src0, double *src1, double* src2, double* src3,
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
#if 1
inline int quad_double_sum_8way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, uint64_t num )
{
  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src0_2, *src1_2, *src2_2, *src3_2, *src4_2, *src5_2, *src6_2, *src7_2;
  double *dst_1, *dst_2;
  register int inc ;
  register int	y;

  src0_1 = src0 -8;
  src0_2 = src0 -4;

  src1_1 = src1 -8;
  src1_2 = src1 -4;

  src2_1 = src2 -8;
  src2_2 = src2 -4;

  src3_1 = src3 -8;
  src3_2 = src3 -4;

  src4_1 = src4 -8;
  src4_2 = src4 -4;

  src5_1 = src5 -8;
  src5_2 = src5 -4;

  src6_1 = src6 -8;
  src6_2 = src6 -4;

  src7_1 = src7 -8;
  src7_2 = src7 -4;

  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=64;

  y= num/8 -1;

  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);

  ASM("qvfmr 28, 0" ::  );
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src0_2,inc,8);

  ASM("qvfmr 29, 1" ::  );
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src1_2,inc,9);

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src2_2,inc,10);
  ASM("qvfmr 30, 8" ::  );

  ASM("qvfadd 29, 29, 3" ::  );
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src3_2,inc,11);
  ASM("qvfmr 31, 9" ::  );

  while (1){

  ASM("qvfadd 28, 28, 4" ::  );
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src4_2,inc,12);
  ASM("qvfadd 30, 30, 10" ::  );

  ASM("qvfadd 29, 29, 5" ::  );
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src5_2,inc,13);
  ASM("qvfadd 31, 31, 11" ::  );

  ASM("qvfadd 28, 28, 6" ::  );
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src6_2,inc,14);
  ASM("qvfadd 30, 30, 12" ::  );

  ASM("qvfadd 29, 29, 7" ::  );
  VECTOR_LOAD(src7_1,inc,7);
  VECTOR_LOAD(src7_2,inc,15);
  ASM("qvfadd 31, 31, 13" ::  );

  if (!--y) break;

  ASM("qvfadd 29, 29, 28" ::  );

  ASM("qvfmr 28, 0" ::  );
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src0_2,inc,8);
  ASM("qvfadd 30, 30, 14" ::  );

  VECTOR_STORE(dst_1,inc,29);

  ASM("qvfmr 29, 1" ::  );
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src1_2,inc,9);
  ASM("qvfadd 31, 31, 15" ::  );

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src2_2,inc,10);
  ASM("qvfadd 31, 31, 30" ::  );
  ASM("qvfmr 30, 8" ::  );

  ASM("qvfadd 29, 29, 3" ::  );
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src3_2,inc,11);
  VECTOR_STORE(dst_2,inc,31);
  ASM("qvfmr 31, 9" ::  );

  }

  ASM("qvfadd 29, 29, 28" ::  );

  ASM("qvfmr 28, 0" ::  );
  VECTOR_LOAD(src0_2,inc,8);
  ASM("qvfadd 30, 30, 14" ::  );

  VECTOR_STORE(dst_1,inc,29);

  ASM("qvfmr 29, 1" ::  );
  VECTOR_LOAD(src1_2,inc,9);
  ASM("qvfadd 31, 31, 15" ::  );

  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_2,inc,10);
  ASM("qvfadd 31, 31, 30" ::  );
  ASM("qvfmr 30, 8" ::  );

  ASM("qvfadd 29, 29, 3" ::  );
  VECTOR_LOAD(src3_2,inc,11);
  VECTOR_STORE(dst_2,inc,31);
  ASM("qvfmr 31, 9" ::  );

  ASM("qvfadd 28, 28, 4" ::  );
  VECTOR_LOAD(src4_2,inc,12);
  ASM("qvfadd 30, 30, 10" ::  );

  ASM("qvfadd 29, 29, 5" ::  );
  VECTOR_LOAD(src5_2,inc,13);
  ASM("qvfadd 31, 31, 11" ::  );

  ASM("qvfadd 28, 28, 6" ::  );
  VECTOR_LOAD(src6_2,inc,14);
  ASM("qvfadd 30, 30, 12" ::  );

  ASM("qvfadd 29, 29, 7" ::  );
  VECTOR_LOAD(src7_2,inc,15);
  ASM("qvfadd 31, 31, 13" ::  );

  ASM("qvfadd 29, 29, 28" ::  );
  ASM("qvfadd 30, 30, 14" ::  );
  ASM("qvfadd 31, 31, 15" ::  );
  ASM("qvfadd 31, 31, 30" ::  );

  VECTOR_STORE(dst_1,inc,29);
  VECTOR_STORE(dst_2,inc,31);
  return 0;
}
#endif
#if 0
inline int quad_double_sum_8way(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, uint64_t num )
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
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
  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=32;
  inc1=64;

  y= num/8;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);

  ASM("qvfmr 28, 0" ::  );
  VECTOR_LOAD(src0_1,inc,0);

  VECTOR_LOAD(src4_1,inc,4);
  //ASM("qvfadd 28, 28, 1" ::  );
  ASM("qvfmr 29, 1" ::  );
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfmr 30, 0" ::  );

  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfmr 31, 1" ::  );


  while (1){

  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 29, 29, 3" ::  );
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 2" ::  );


  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 28, 28, 4" ::  );
  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 31, 31, 3" ::  );

  if (!--y) break;
  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 29, 29, 5" ::  );
  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 30, 30, 4" ::  );


  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 28, 28, 6" ::  );
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 31, 31, 5" ::  );

  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 29, 29, 7" ::  );
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 30, 30, 6" ::  );

  VECTOR_LOAD(src3_1,inc,3);
  //VECTOR_STORE(dst_1,inc1,28);
  ASM("qvfadd 29, 29, 28" ::  );
  ASM("qvfmr 28, 0" ::  );
  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 31, 31, 7" ::  );

  VECTOR_LOAD(src4_1,inc,4);
  //ASM("qvfadd 28, 28, 1" ::  );
  VECTOR_STORE(dst_1,inc1,29);
  ASM("qvfmr 29, 1" ::  );
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 31, 31, 30" ::  );
  ASM("qvfmr 30, 0" ::  );

  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 28, 28, 2" ::  );
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_STORE(dst_2,inc1,31);
  ASM("qvfmr 31, 1" ::  );

  }

  ASM("qvfadd 29, 29, 5" ::  );
  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 30, 30, 4" ::  );

  ASM("qvfadd 28, 28, 6" ::  );
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 31, 31, 5" ::  );

  ASM("qvfadd 29, 29, 7" ::  );
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 30, 30, 6" ::  );

  ASM("qvfadd 29, 29, 28" ::  );
  VECTOR_STORE(dst_1,inc1,29);
  ASM("qvfadd 31, 31, 7" ::  );

  ASM("qvfadd 31, 31, 30" ::  );
  VECTOR_STORE(dst_2,inc1,31);


  return 0;
}
#endif
#if 0
//Not finished yet..dont use
inline int quad_double_sum_8way_opt(double* dest, double* src0, double *src1, double* src2, double* src3,
			double* src4, double* src5, double* src6, double* src7, uint64_t num )
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src8_1, *src9_1, *src10_1, *src11_1, *src12_1, *src13_1, *src14_1, *src15_1;
  double *dst_1, *dst_2;
  //register int inc __MYASM__("r7");
  //register int inc __MYASM__("r7");
  register int inc ;
  register int inc1;
  register int	y;

  src0_1 = src0 -8;
  src1_1 = src1 -8;
  src2_1 = src2 -8;
  src3_1 = src3 -8;
  src4_1 = src4 -8;
  src5_1 = src5 -8;
  src6_1 = src6 -8;
  src7_1 = src7 -8;
  src0_2 = src0 -4;
  src1_2 = src1 -4;
  src2_2 = src2 -4;
  src3_2 = src3 -4;
  src4_2 = src4 -4;
  src5_2 = src5 -4;
  src6_2 = src6 -4;
  src7_2 = src7 -4;

  dst_1 = dest -8;
  dst_2 = dest -4;

  inc=64;

  y= num/8;


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfmr 28, 0" ::  );


  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src0_2,inc,8);
  ASM("qvfadd 28, 28, 1" ::  );

  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src1_2,inc,9);
  ASM("qvfadd 28, 28, 2" ::  );

  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src2_2,inc,10);
  ASM("qvfadd 28, 28, 3" ::  );

  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src3_2,inc,11);
  ASM("qvfmr 28, 0" ::  );
  ASM("qvfadd 28, 28, 4" ::  );


  VECTOR_LOAD(src10_1,inc,10);
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 28, 28, 3" ::  );


  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfmr 30, 0" ::  );
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 28, 28, 4" ::  );

  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 30, 30, 1" ::  );
  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 28, 28, 5" ::  );

  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 30, 30, 2" ::  );
  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 28, 28, 6" ::  );

  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 30, 30, 3" ::  );
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 28, 28, 7" ::  );

  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 30, 30, 4" ::  );
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 28, 28, 8" ::  );

  while (1)
  {

  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 30, 30, 5" ::  );
  VECTOR_LOAD(src8_1,inc,8);
  ASM("qvfadd 28, 28, 9" ::  );

  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 30, 30, 6" ::  );
  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 28, 28, 10" ::  );

  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 30, 30, 7" ::  );
  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 28, 28, 11" ::  );

  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 30, 30, 8" ::  );
  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfadd 28, 28, 12" ::  );

  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 30, 30, 9" ::  );
  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 28, 28, 13" ::  );

  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 30, 30, 10" ::  );
  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 28, 28, 14" ::  );

  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 30, 30, 11" ::  );
  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 28, 28, 15" ::  ); //first first-half sum done


  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 30, 30, 12" ::  );
  VECTOR_LOAD(src15_1,inc,15);


  VECTOR_LOAD(src8_1,inc,8);
  ASM("qvfadd 30, 30, 13" ::  );
  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 29, 0, 1" ::  ); 	//second first-half sum start


  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 30, 30, 14" ::  );
  VECTOR_STORE(dst_1,inc1,28);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 29, 29, 2" ::  );


  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 30, 30, 15" ::  ); //first second-half sum done
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 29, 29, 3" ::  );

  VECTOR_LOAD(src11_1,inc,11);
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 29, 29, 4" ::  );


  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 31, 0, 1" ::  ); //second second-half sum start
  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 29, 29, 5" ::  );
  VECTOR_STORE(dst_2,inc1,30);


  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 31, 31, 2" ::  );
  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 29, 29, 6" ::  );


  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 31, 31, 3" ::  );
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 29, 29, 7" ::  );


  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 31, 31, 4" ::  );
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 29, 29, 8" ::  );

	y-=2;
	if (!y) break;

  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 31, 31, 5" ::  );
  VECTOR_LOAD(src8_1,inc,8);
  ASM("qvfadd 29, 29, 9" ::  );


  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 31, 31, 6" ::  );
  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 29, 29, 10" ::  );

  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 31, 31, 7" ::  );
  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 29, 29, 11" ::  );

  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 31, 31, 8" ::  );
  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfadd 29, 29, 12" ::  );

  VECTOR_LOAD(src4_1,inc,4);
  ASM("qvfadd 31, 31, 9" ::  );
  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 29, 29, 13" ::  );

  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 31, 31, 10" ::  );
  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 29, 29, 14" ::  );

  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 31, 31, 11" ::  );
  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 29, 29, 15" ::  );

  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 31, 31, 12" ::  );
  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfmr 28, 0" ::  );

  VECTOR_LOAD(src8_1,inc,8);
  ASM("qvfadd 31, 31, 13" ::  );
  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 28, 28, 1" ::  );

  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 31, 31, 14" ::  );
  VECTOR_STORE(dst_1,inc1,29);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 28, 28, 2" ::  );

  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 31, 31, 15" ::  );
  VECTOR_LOAD(src2_1,inc,2);
  ASM("qvfadd 28, 28, 3" ::  );

  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfmr 30, 0" ::  );
  VECTOR_LOAD(src3_1,inc,3);
  ASM("qvfadd 28, 28, 4" ::  );

  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 30, 30, 1" ::  );
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_STORE(dst_2,inc1,31);
  ASM("qvfadd 28, 28, 5" ::  );

  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 30, 30, 2" ::  );
  VECTOR_LOAD(src5_1,inc,5);
  ASM("qvfadd 28, 28, 6" ::  );

  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 30, 30, 3" ::  );
  VECTOR_LOAD(src6_1,inc,6);
  ASM("qvfadd 28, 28, 7" ::  );

  VECTOR_LOAD(src15_1,inc,15);
  ASM("qvfadd 30, 30, 4" ::  );
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfadd 28, 28, 8" ::  );
  }

  ASM("qvfadd 31, 31, 5" ::  );
  VECTOR_LOAD(src8_1,inc,8);
  ASM("qvfadd 29, 29, 9" ::  );

  ASM("qvfadd 31, 31, 6" ::  );
  VECTOR_LOAD(src9_1,inc,9);
  ASM("qvfadd 29, 29, 10" ::  );

  ASM("qvfadd 31, 31, 7" ::  );
  VECTOR_LOAD(src10_1,inc,10);
  ASM("qvfadd 29, 29, 11" ::  );

  ASM("qvfadd 31, 31, 8" ::  );
  VECTOR_LOAD(src11_1,inc,11);
  ASM("qvfadd 29, 29, 12" ::  );

  ASM("qvfadd 31, 31, 9" ::  );
  VECTOR_LOAD(src12_1,inc,12);
  ASM("qvfadd 29, 29, 13" ::  );

  ASM("qvfadd 31, 31, 10" ::  );
  VECTOR_LOAD(src13_1,inc,13);
  ASM("qvfadd 29, 29, 14" ::  );

  ASM("qvfadd 31, 31, 11" ::  );
  VECTOR_LOAD(src14_1,inc,14);
  ASM("qvfadd 29, 29, 15" ::  );

  ASM("qvfadd 31, 31, 12" ::  );
  VECTOR_LOAD(src15_1,inc,15);

  ASM("qvfadd 31, 31, 13" ::  );

  ASM("qvfadd 31, 31, 14" ::  );
  VECTOR_STORE(dst_1,inc1,29);

  ASM("qvfadd 31, 31, 15" ::  );
  VECTOR_STORE(dst_2,inc1,31);

}
#endif
