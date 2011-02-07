/**
 * \file components/devices/shmemcoll/16way_sum.h
 * \brief ???
 */
#ifndef __components_devices_shmemcoll_16way_sum_h__
#define __components_devices_shmemcoll_16way_sum_h__

#include "asmheader.h"

//atleast 128B(16 doubles) ..in multiples of 64B
//alignment expected is L1 cache line == 64B

inline unsigned _quad_double_sum_16way_align64B(double* dst, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, double* src8, double *src9, double* src10, double* src11,
    double* src12, double* src13, double* src14, double* src15, unsigned num_dbls)
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

  dst_1 = dst -8;
  dst_2 = dst -4;

  inc=32;
  inc1=64;

  y = (num_dbls>>3) >> 1;

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

	//y-=2;
	y-=1;
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

  return ((num_dbls>>(3+1)) << (3+1));
}

#if 0
inline unsigned _quad_double_sum_16way_align64B(double* dst, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, double* src8, double *src9, double* src10, double* src11,
    double* src12, double* src13, double* src14, double* src15, unsigned num_dbls)
{

  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src8_1, *src9_1, *src10_1, *src11_1, *src12_1, *src13_1, *src14_1, *src15_1;
  double *dst_1, *dst_2;
  //register int inc __MYASM__("r7");
  //register int inc __MYASM__("r7");
  register int inc ;
  register int inc1;
  register int  y;
  
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

  y = (num_dbls>>3) >> 1;

  VECTOR_LOAD(src0_1,inc,0); 
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src4_1,inc,4);
  VECTOR_LOAD(src5_1,inc,5);
  VECTOR_LOAD(src6_1,inc,6);
  VECTOR_LOAD(src7_1,inc,7);
  ASM("qvfmr 28, 0" ::  );

  VECTOR_LOAD(src8_1,inc,8);
  VECTOR_LOAD(src0_1,inc,0);
  ASM("qvfadd 28, 28, 1" ::  );


  VECTOR_LOAD(src9_1,inc,9);
  VECTOR_LOAD(src1_1,inc,1);
  ASM("qvfadd 28, 28, 2" ::  );


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
  ASM("qvfadd 29, 0, 1" ::  );  //second first-half sum start


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

  //y-=2;
  y-=1;
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

  return ((num_dbls>>(3+1)) << 3+1);

}
#endif

//alteast 32B (4 Dbls) expected
inline unsigned _quad_double_sum_16way_align32B_short(double* dst, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, double* src8, double *src9, double* src10, double* src11,
    double* src12, double* src13, double* src14, double* src15, unsigned num_dbls)
{
  double *src0_1, *src1_1, *src2_1, *src3_1, *src4_1, *src5_1, *src6_1, *src7_1;
  double *src8_1, *src9_1, *src10_1, *src11_1, *src12_1, *src13_1, *src14_1, *src15_1;
  double *dst_1;
  register int inc ;
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

  dst_1 = dst -4;

  inc=32;

  y= num_dbls>>2;

  if (y == 0) return 0;

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
  VECTOR_LOAD(src12_1,inc,12);
  VECTOR_LOAD(src13_1,inc,13);
  VECTOR_LOAD(src14_1,inc,14);
  VECTOR_LOAD(src15_1,inc,15);

  if (y == 1)
  {
    ASM("qvfmr 28, 0" ::  );
    ASM("qvfmr 29, 1" ::  );
    ASM("qvfmr 30, 2" ::  );
    ASM("qvfmr 31, 3" ::  );
    ASM("qvfadd 28, 28, 4" ::  );
    ASM("qvfadd 29, 29, 5" ::  );
    ASM("qvfadd 30, 30, 6" ::  );
    ASM("qvfadd 31, 31, 7" ::  );
    ASM("qvfadd 28, 28, 8" ::  );
    ASM("qvfadd 29, 29, 9" ::  );
    ASM("qvfadd 30, 30, 10" ::  );
    ASM("qvfadd 31, 31, 11" ::  );
    ASM("qvfadd 28, 28, 12" ::  );
    ASM("qvfadd 29, 29, 13" ::  );
    ASM("qvfadd 30, 30, 14" ::  );
    ASM("qvfadd 31, 31, 15" ::  );
    ASM("qvfadd 29, 29, 28" ::  );
    ASM("qvfadd 31, 31, 30" ::  );
    ASM("qvfadd 31, 31, 29" ::  );
    VECTOR_STORE(dst_1,inc,31);
    return 4;
  }

  --y;

  ASM("qvfmr 28, 0" ::  );
  VECTOR_LOAD(src0_1,inc,0);

  ASM("qvfmr 29, 1" ::  );
  VECTOR_LOAD(src1_1,inc,1);

  ASM("qvfmr 30, 2" ::  );
  VECTOR_LOAD(src2_1,inc,2);

  ASM("qvfmr 31, 3" ::  );
  VECTOR_LOAD(src3_1,inc,3);

  ASM("qvfadd 28, 28, 4" ::  );
  VECTOR_LOAD(src4_1,inc,4);

  ASM("qvfadd 29, 29, 5" ::  );
  VECTOR_LOAD(src5_1,inc,5);

  ASM("qvfadd 30, 30, 6" ::  );
  VECTOR_LOAD(src6_1,inc,6);

  ASM("qvfadd 31, 31, 7" ::  );
  VECTOR_LOAD(src7_1,inc,7);

  while (1){

    ASM("qvfadd 28, 28, 8" ::  );
    VECTOR_LOAD(src8_1,inc,8);

    ASM("qvfadd 29, 29, 9" ::  );
    VECTOR_LOAD(src9_1,inc,9);

    ASM("qvfadd 30, 30, 10" ::  );
    VECTOR_LOAD(src10_1,inc,10);

    ASM("qvfadd 31, 31, 11" ::  );
    VECTOR_LOAD(src11_1,inc,11);

    ASM("qvfadd 28, 28, 12" ::  );
    VECTOR_LOAD(src12_1,inc,12);

    ASM("qvfadd 29, 29, 13" ::  );
    VECTOR_LOAD(src13_1,inc,13);

    ASM("qvfadd 30, 30, 14" ::  );
    VECTOR_LOAD(src14_1,inc,14);

    ASM("qvfadd 31, 31, 15" ::  );
    VECTOR_LOAD(src15_1,inc,15);

    if (!--y) break;

    ASM("qvfadd 29, 29, 28" ::  );
    ASM("qvfmr 28, 0" ::  );
    VECTOR_LOAD(src0_1,inc,0);

    ASM("qvfadd 31, 31, 30" ::  );
    ASM("qvfadd 31, 31, 29" ::  );

    ASM("qvfmr 29, 1" ::  );
    VECTOR_LOAD(src1_1,inc,1);

    ASM("qvfmr 30, 2" ::  );
    VECTOR_LOAD(src2_1,inc,2);

    VECTOR_STORE(dst_1,inc,31);
    ASM("qvfmr 31, 3" ::  );
    VECTOR_LOAD(src3_1,inc,3);

    ASM("qvfadd 28, 28, 4" ::  );
    VECTOR_LOAD(src4_1,inc,4);

    ASM("qvfadd 29, 29, 5" ::  );
    VECTOR_LOAD(src5_1,inc,5);

    ASM("qvfadd 30, 30, 6" ::  );
    VECTOR_LOAD(src6_1,inc,6);

    ASM("qvfadd 31, 31, 7" ::  );
    VECTOR_LOAD(src7_1,inc,7);
  } 

    ASM("qvfadd 29, 29, 28" ::  );
    ASM("qvfadd 31, 31, 30" ::  );
    ASM("qvfadd 31, 31, 29" ::  );
    VECTOR_STORE(dst_1,inc,31);

    ASM("qvfmr 28, 0" ::  );
    ASM("qvfmr 29, 1" ::  );
    ASM("qvfmr 30, 2" ::  );
    ASM("qvfmr 31, 3" ::  );
    ASM("qvfadd 28, 28, 4" ::  );
    ASM("qvfadd 29, 29, 5" ::  );
    ASM("qvfadd 30, 30, 6" ::  );
    ASM("qvfadd 31, 31, 7" ::  );
    ASM("qvfadd 28, 28, 8" ::  );
    ASM("qvfadd 29, 29, 9" ::  );
    ASM("qvfadd 30, 30, 10" ::  );
    ASM("qvfadd 31, 31, 11" ::  );
    ASM("qvfadd 28, 28, 12" ::  );
    ASM("qvfadd 29, 29, 13" ::  );
    ASM("qvfadd 30, 30, 14" ::  );
    ASM("qvfadd 31, 31, 15" ::  );
    ASM("qvfadd 29, 29, 28" ::  );
    ASM("qvfadd 31, 31, 30" ::  );
    ASM("qvfadd 31, 31, 29" ::  );
    VECTOR_STORE(dst_1,inc,31);

  return ((num_dbls >> 2) << 2);

}

inline unsigned _quad_double_sum_16way_single(double* dst, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, double* src8, double *src9, double* src10, double* src11,
    double* src12, double* src13, double* src14, double* src15, unsigned num_dbls)
{

  unsigned i;
  for (i = 0; i < num_dbls; i++)
    dst[i] = src0[i] + src1[i] + src2[i] + src3[i] + src4[i] + src5[i] + src6[i] + src7[i] + 
            src8[i] + src9[i] + src10[i] + src11[i] + src12[i] + src13[i] + src14[i] + src15[i];

  return num_dbls;

}

//atleast 20 Dbls exptected..(20-4) == 16 for 64B aligned sum
inline unsigned _quad_double_sum_16way_align32B(double* dst, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, double* src8, double *src9, double* src10, double* src11,
    double* src12, double* src13, double* src14, double* src15, unsigned num_dbls)
{

  unsigned  dbls = 0;
  uint64_t  mask;
  mask = (uint64_t)dst|(uint64_t)src0|(uint64_t)src1|(uint64_t)src2|(uint64_t)src3|(uint64_t)src4|(uint64_t)src5|
           (uint64_t)src6|(uint64_t)src7|(uint64_t)src8|(uint64_t)src9|(uint64_t)src10|(uint64_t)src11| 
           (uint64_t)src12|(uint64_t)src13|(uint64_t)src14|(uint64_t)src15;
  bool is_64B_aligned;
  is_64B_aligned = !((mask & ((uint64_t)63)) == 0);
  
  if (is_64B_aligned)
  {
    dbls = _quad_double_sum_16way_align64B(dst, src0, src1, src2, src3, src4, src5, src6, src7, src8, 
                                          src9, src10, src11, src12, src13, src14, src15, num_dbls);
    if (dbls == num_dbls) return dbls;
    _quad_double_sum_16way_single(dst+dbls, src0+dbls, src1+dbls, src2+dbls, src3+dbls, src4+dbls, 
        src5+dbls, src6+dbls, src7+dbls, src8+dbls, src9+dbls, src10+dbls, src11+dbls, 
        src12+dbls, src13+dbls, src14+dbls, src15+dbls, num_dbls - dbls);
    return num_dbls;
  }

  _quad_double_sum_16way_single(dst, src0, src1, src2, src3, src4, src5, src6, src7, src8, 
                                src9, src10, src11, src12, src13, src14, src15, 4);

  dbls = _quad_double_sum_16way_align64B(dst+4, src0+4, src1+4, src2+4, src3+4, src4+4, src5+4, src6+4, src7+4, src8+4, 
                                        src9+4, src10+4, src11+4, src12+4, src13+4, src14+4, src15+4, num_dbls - 4);
  dbls+=4;

  return dbls;

}
#if 0
//buffers aligned to 32B 
inline unsigned quad_double_sum_16way(double* dst, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, double* src8, double *src9, double* src10, double* src11,
    double* src12, double* src13, double* src14, double* src15, unsigned num_dbls)
{
  
  unsigned  dbls = 0;

  if (num_dbls < 20)
   dbls = _quad_double_sum_16way_align32B_short(dst, src0, src1, src2, src3, src4, src5, src6, src7, src8, 
                                          src9, src10, src11, src12, src13, src14, src15, num_dbls);
  else
   dbls = _quad_double_sum_16way_align32B(dst, src0, src1, src2, src3, src4, src5, src6, src7, src8,
                                          src9, src10, src11, src12, src13, src14, src15, num_dbls);

  if (num_dbls - dbls)
   return _quad_double_sum_16way_single(dst+dbls, src0+dbls, src1+dbls, src2+dbls, src3+dbls, src4+dbls, 
                                  src5+dbls, src6+dbls, src7+dbls, src8+dbls, src9+dbls, src10+dbls, src11+dbls, 
                                  src12+dbls, src13+dbls, src14+dbls, src15+dbls, num_dbls - dbls);

  return num_dbls;
}
#endif

inline unsigned quad_double_sum_16way(double* dst, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, double* src8, double *src9, double* src10, double* src11,
    double* src12, double* src13, double* src14, double* src15, unsigned num_dbls)
{
  
  unsigned  dbls = 0;

  if (num_dbls <= 128)
   dbls = _quad_double_sum_16way_align32B_short(dst, src0, src1, src2, src3, src4, src5, src6, src7, src8, 
                                          src9, src10, src11, src12, src13, src14, src15, num_dbls);
  else
  {
    dbls = quad_double_sum_8way(dst, src0, src1, src2, src3, src4, src5, src6, src7, num_dbls);
    /*for (unsigned i=0; i < num_dbls; i++)
    { printf("[%d]:%f\t",i, dst[i]); printf("\n");}*/
    dbls = quad_double_sum_8way(dst, dst, src8, src9, src10, src11, src12, src13, src14, num_dbls);
    /*for (unsigned i=0; i < num_dbls; i++)
    { printf("[%d]:%f\t",i, dst[i]); printf("\n");}*/
    dbls = quad_double_sum_2way(dst, dst, src15, num_dbls);
    /*for (unsigned i=0; i < num_dbls; i++)
    { printf("[%d]:%f\t",i, dst[i]); printf("\n");}*/
  }

  if (num_dbls - dbls)
    _quad_double_sum_16way_single(dst+dbls, src0+dbls, src1+dbls, src2+dbls, src3+dbls, src4+dbls, 
        src5+dbls, src6+dbls, src7+dbls, src8+dbls, src9+dbls, src10+dbls, src11+dbls, 
        src12+dbls, src13+dbls, src14+dbls, src15+dbls, num_dbls - dbls);

  return num_dbls;
}


#endif
