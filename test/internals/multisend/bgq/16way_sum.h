/**
 * \file test/internals/multisend/bgq/16way_sum.h
 * \brief ???
 */
#ifndef __test_internals_multisend_bgq_16way_sum_h__
#define __test_internals_multisend_bgq_16way_sum_h__
#include "sum.h"

#ifdef __GNUC__
#define __MYASM__(p) asm(p)
#else
#define __MYASM__(p) /* nothing */
#endif

#define ASM asm volatile


#if 0
inline int quad_double_sum_16way_opt(double* dest, double* src0, double *src1, double* src2, double* src3,
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
#if 1
inline int quad_double_sum_16way_opt(double* dest, double* src0, double *src1, double* src2, double* src3,
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

	return 0;
}
#endif
#endif
