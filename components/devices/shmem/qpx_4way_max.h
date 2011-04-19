/**
 * \file components/devices/shmemcoll/4way_max.h
 * \brief ???
 */
#ifndef __components_devices_shmemcoll_4way_max_h__
#define __components_devices_shmemcoll_4way_max_h__

#include "asmheader.h"

//atleast 128B(16 doubles) ..in multiples of 64B
//alignment expected is L1 cache line == 64B

inline unsigned _quad_double_max_4way_align64B(double* dst, double* src0, double *src1, double* src2, double* src3, unsigned num_dbls)
{

  double *src0_1, *src0_2, *src1_1, *src1_2, *src2_1, *src2_2, *src3_1, *src3_2;
  double *dst_1, *dst_2;

  register int inc;
  register int	y;

  src0_1 = src0 -8;  //offset by stride=0 bytes
  src0_2 = src0 -4;

  src1_1 = src1 -8;  //offset by stride=0 bytes
  src1_2 = src1 -4;

  src2_1 = src2 -8;  //offset by stride=0 bytes
  src2_2 = src2 -4;

  src3_1 = src3 -8;  //offset by stride=0 bytes
  src3_2 = src3 -4;

  dst_1 = dst -8;  //offset by stride=0 bytes
  dst_2 = dst -4;

  inc=64;

  y = (num_dbls >> 3) >> 1;
  //y=(num-128)/128;

  // may speed up the 2nd to 7th load by using non-update versions of load, tbd later...
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src0_1,inc,4);
  VECTOR_LOAD(src1_1,inc,5);
  VECTOR_LOAD(src2_1,inc,6);
  VECTOR_LOAD(src3_1,inc,7);

  (--y);
  
  //ASM("qvfadd 16, 0, 1" ::  );
  ASM("qvfcmpgt 31, 0,1" :: );
  ASM("qvfsel 16, 31, 0, 1" :: );
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_2,inc,8);
  VECTOR_LOAD(src1_2,inc,9);

  //ASM("qvfadd 17, 2, 3" ::  );
  ASM("qvfcmpgt 31, 2,3" :: );
  ASM("qvfsel 17, 31, 2, 3" :: );
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);
  VECTOR_LOAD(src2_2,inc,10);
  VECTOR_LOAD(src3_2,inc,11);
  //ASM("qvfadd 22, 8, 9" ::  );
  ASM("qvfcmpgt 31, 8,9" :: );
  ASM("qvfsel 22, 31, 8, 9" :: );

  do{
    //ASM("qvfadd 18, 16, 17" ::  );
    ASM("qvfcmpgt 31, 16, 17" :: );
    ASM("qvfsel 18, 31, 16, 17" :: );


    //ASM("qvfadd 19, 4, 5" ::  );
    ASM("qvfcmpgt 31, 4, 5" :: );
    ASM("qvfsel 19, 31, 4, 5" :: );
    VECTOR_LOAD(src0_1,inc,4);
    VECTOR_LOAD(src1_1,inc,5);
    VECTOR_LOAD(src0_2,inc,12);
    VECTOR_LOAD(src1_2,inc,13);

    //ASM("qvfadd 23, 10, 11" ::  );
    ASM("qvfcmpgt 31, 10, 11" :: );
    ASM("qvfsel 23, 31, 10, 11" :: );
    VECTOR_STORE(dst_1,inc,18);

    //ASM("qvfadd 20, 6, 7" ::  );
    ASM("qvfcmpgt 31, 6, 7" :: );
    ASM("qvfsel 20, 31, 6, 7" :: );
    VECTOR_LOAD(src2_1,inc,6);
    VECTOR_LOAD(src3_1,inc,7);
    VECTOR_LOAD(src2_2,inc,14);
    VECTOR_LOAD(src3_2,inc,15);

    //ASM("qvfadd 25, 12, 13" ::  );
    ASM("qvfcmpgt 31, 12, 13" :: );
    ASM("qvfsel 25, 31, 12, 13" :: );

    //ASM("qvfadd 24, 22, 23" ::  );
    ASM("qvfcmpgt 31, 22, 23" :: );
    ASM("qvfsel 24, 31, 22, 23" :: );

    //ASM("qvfadd 21, 19, 20" ::  );
    ASM("qvfcmpgt 31, 19, 20" :: );
    ASM("qvfsel 21, 31, 19, 20" :: );

    if(!--y) break;

    //ASM("qvfadd 16, 0, 1" ::  );
    ASM("qvfcmpgt 31, 0,1" :: );
    ASM("qvfsel 16, 31, 0, 1" :: );
    VECTOR_LOAD(src0_1,inc,0);
    VECTOR_LOAD(src1_1,inc,1);
    //ASM("qvfadd 26, 14, 15" ::  );
    ASM("qvfcmpgt 31, 14, 15" :: );
    ASM("qvfsel 26, 31, 14, 15" :: );
    VECTOR_LOAD(src0_2,inc,8);
    VECTOR_LOAD(src1_2,inc,9);

    VECTOR_STORE(dst_2,inc,24);

    VECTOR_STORE(dst_1,inc,21);

    //ASM("qvfadd 17, 2, 3" ::  );
    ASM("qvfcmpgt 31, 2,3" :: );
    ASM("qvfsel 17, 31, 2, 3" :: );
    //ASM("qvfadd 27, 25, 26" ::  );
    ASM("qvfcmpgt 31, 25, 26" :: );
    ASM("qvfsel 27, 31, 25, 26" :: );
    VECTOR_LOAD(src2_1,inc,2);
    VECTOR_LOAD(src3_1,inc,3);
    VECTOR_LOAD(src2_2,inc,10);
    VECTOR_LOAD(src3_2,inc,11);
    //ASM("qvfadd 22, 8, 9" ::  );
    ASM("qvfcmpgt 31, 8,9" :: );
    ASM("qvfsel 22, 31, 8, 9" :: );
    VECTOR_STORE(dst_2,inc,27);

  }while(1);
  VECTOR_STORE(dst_2,inc,24);

  //ASM("qvfadd 26, 14, 15" ::  );
   ASM("qvfcmpgt 31, 14, 15" :: );
   ASM("qvfsel 26, 31, 14, 15" :: );

  //ASM("qvfadd 16, 0, 1" ::  );
  ASM("qvfcmpgt 31, 0,1" :: );
  ASM("qvfsel 16, 31, 0, 1" :: );
  //ASM("qvfadd 17, 2, 3" ::  );
  ASM("qvfcmpgt 31, 2,3" :: );
  ASM("qvfsel 17, 31, 2, 3" :: );
  VECTOR_LOAD(src0_2,inc,8);
  VECTOR_LOAD(src1_2,inc,9);
  //ASM("qvfadd 22, 8, 9" ::  );
  ASM("qvfcmpgt 31, 8,9" :: );
  ASM("qvfsel 22, 31, 8, 9" :: );

  VECTOR_STORE(dst_1,inc,21);
  //ASM("qvfadd 27, 25, 26" ::  );
  ASM("qvfcmpgt 31, 25, 26" :: );
  ASM("qvfsel 27, 31, 25, 26" :: );

  VECTOR_LOAD(src2_2,inc,10);
  VECTOR_LOAD(src3_2,inc,11);
  //ASM("qvfadd 23, 10, 11" ::  );
  ASM("qvfcmpgt 31, 10, 11" :: );
  ASM("qvfsel 23, 31, 10, 11" :: );

  //ASM("qvfadd 18, 16, 17" ::  );
  ASM("qvfcmpgt 31, 16, 17" :: );
  ASM("qvfsel 18, 31, 16, 17" :: );
  //ASM("qvfadd 24, 22, 23" ::  );
  ASM("qvfcmpgt 31, 22, 23" :: );
  ASM("qvfsel 24, 31, 22, 23" :: );
  VECTOR_STORE(dst_2,inc,27);

  //ASM("qvfadd 19, 4, 5" ::  );
  ASM("qvfcmpgt 31, 4, 5" :: );
  ASM("qvfsel 19, 31, 4, 5" :: );
  VECTOR_LOAD(src0_2,inc,12);
  VECTOR_LOAD(src1_2,inc,13);

  VECTOR_STORE(dst_1,inc,18);
  VECTOR_STORE(dst_2,inc,24);

  //ASM("qvfadd 20, 6, 7" ::  );
  ASM("qvfcmpgt 31, 6, 7" :: );
  ASM("qvfsel 20, 31, 6, 7" :: );
  VECTOR_LOAD(src2_2,inc,14);
  VECTOR_LOAD(src3_2,inc,15);
  //ASM("qvfadd 25, 12, 13" ::  );
  ASM("qvfcmpgt 31, 12, 13" :: );
  ASM("qvfsel 25, 31, 12, 13" :: );
  //ASM("qvfadd 26, 14, 15" ::  );
  ASM("qvfcmpgt 31, 14, 15" :: );
  ASM("qvfsel 26, 31, 14, 15" :: );


  //ASM("qvfadd 21, 19, 20" ::  );
  ASM("qvfcmpgt 31, 19, 20" :: );
  ASM("qvfsel 21, 31, 19, 20" :: );
  //ASM("qvfadd 27, 25, 26" ::  );
  ASM("qvfcmpgt 31, 25, 26" :: );
  ASM("qvfsel 27, 31, 25, 26" :: );

  VECTOR_STORE(dst_1,inc,21);
  VECTOR_STORE(dst_2,inc,27);

  return ((num_dbls >> (3+1)) << (3+1));
}

//alteast 32B (4 Dbls) expected
inline unsigned _quad_double_max_4way_align32B_short(double* dst, double* src0, double *src1, double* src2, double* src3, unsigned num_dbls)
{
  double *src0_1, *src1_1, *src2_1, *src3_1;
  double *dst_1;
  register int inc ;
  register int	y;

  src0_1 = src0 -4;
  src1_1 = src1 -4;
  src2_1 = src2 -4;
  src3_1 = src3 -4;

  dst_1 = dst -4;

  inc=32;

  y= num_dbls>>2;

  if (y == 0) return 0;

  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src2_1,inc,2);
  VECTOR_LOAD(src3_1,inc,3);

  if (y == 1)
  {
    ASM("qvfmr 28, 0" ::  );
    ASM("qvfmr 29, 1" ::  );
    //ASM("qvfadd 28, 28, 2" ::  );
    ASM("qvfcmpgt 16, 28,2" :: );
    //ASM("qvfsel 28, 16, 2, 28" :: );
    ASM("qvfsel 28, 16, 28, 2" :: );
    //ASM("qvfadd 29, 29, 3" ::  );
    ASM("qvfcmpgt 16, 29,3" :: );
    //ASM("qvfsel 29, 16, 3, 29" :: );
    ASM("qvfsel 29, 16, 29, 3" :: );
    //ASM("qvfadd 29, 29, 28" ::  );
    ASM("qvfcmpgt 16, 29,28" :: );
    ASM("qvfsel 29, 16, 29, 28" :: );
    VECTOR_STORE(dst_1,inc,29);
    return 4;
  }

  --y;

  ASM("qvfmr 28, 0" ::  );
  VECTOR_LOAD(src0_1,inc,0);

  ASM("qvfmr 29, 1" ::  );
  VECTOR_LOAD(src1_1,inc,1);

  while (1){
    //ASM("qvfadd 28, 28, 2" ::  );
    ASM("qvfcmpgt 16, 28,2" :: );
    ASM("qvfsel 28, 16, 28, 2" :: );
    VECTOR_LOAD(src2_1,inc,2);

    //ASM("qvfadd 29, 29, 3" ::  );
    ASM("qvfcmpgt 16, 29,3" :: );
    ASM("qvfsel 29, 16, 29, 3" :: );
    VECTOR_LOAD(src3_1,inc,3);

    if (!--y) break;

    //ASM("qvfadd 29, 29, 28" ::  );
    ASM("qvfcmpgt 16, 29,28" :: );
    ASM("qvfsel 29, 16, 29, 28" :: );
    ASM("qvfmr 28, 0" ::  );
    VECTOR_LOAD(src0_1,inc,0);

    VECTOR_STORE(dst_1,inc,29);
    ASM("qvfmr 29, 1" ::  );
    VECTOR_LOAD(src1_1,inc,1);
  } 

    //ASM("qvfadd 29, 29, 28" ::  );
    ASM("qvfcmpgt 16, 29,28" :: );
    ASM("qvfsel 29, 16, 29, 28" :: );
    ASM("qvfmr 28, 0" ::  );
    VECTOR_STORE(dst_1,inc,29);
    ASM("qvfmr 29, 1" ::  );
    //ASM("qvfadd 28, 28, 2" ::  );
    ASM("qvfcmpgt 16, 28,2" :: );
    ASM("qvfsel 28, 16, 28, 2" :: );
    //ASM("qvfadd 29, 29, 3" ::  );
    ASM("qvfcmpgt 16, 29,3" :: );
    ASM("qvfsel 29, 16, 29, 3" :: );
    //ASM("qvfadd 29, 29, 28" ::  );
    ASM("qvfcmpgt 16, 29,28" :: );
    ASM("qvfsel 29, 16, 29, 28" :: );
    VECTOR_STORE(dst_1,inc,29);

  return ((num_dbls >> 2) << 2);
}

inline unsigned _quad_double_max_4way_single(double* dst, double* src0, double *src1, double* src2, double* src3, unsigned num_dbls)
{

  unsigned i;
  for (i = 0; i < num_dbls; i++)
  {
    dst[i] = (src0[i] > src1[i])? src0[i]:src1[i];
    dst[i] = (dst[i] > src2[i])? dst[i]:src2[i];
    dst[i] = (dst[i] > src3[i])? dst[i]:src3[i];
  }

  return num_dbls;

}

//atleast 20 Dbls exptected..(20-4) == 16 for 64B aligned max
inline unsigned _quad_double_max_4way_align32B(double* dst, double* src0, double *src1, double* src2, double* src3, unsigned num_dbls)
{

  unsigned  dbls = 0;
  uint64_t  mask;
  mask = (uint64_t)dst|(uint64_t)src0|(uint64_t)src1|(uint64_t)src2|(uint64_t)src3 ; 

  bool is_64B_aligned;
  is_64B_aligned = !((mask & ((uint64_t)63)) == 0);
  
  if (is_64B_aligned)
  {
    dbls = _quad_double_max_4way_align64B(dst, src0, src1, src2, src3, num_dbls);
    if (dbls == num_dbls) return dbls;
    _quad_double_max_4way_single(dst+dbls, src0+dbls, src1+dbls, src2+dbls, src3+dbls, num_dbls - dbls);
    return num_dbls;
  }

  _quad_double_max_4way_single(dst, src0, src1, src2, src3, 4);

  dbls = _quad_double_max_4way_align64B(dst+4, src0+4, src1+4, src2+4, src3+4, num_dbls - 4);
  dbls+=4;

  return dbls;

}

//buffers aligned to 32B 
inline unsigned quad_double_max_4way(double* dst, double* src0, double *src1, double* src2, double* src3, unsigned num_dbls)
{
  
  unsigned  dbls = 0;

  if (num_dbls < 36)
   dbls = _quad_double_max_4way_align32B_short(dst, src0, src1, src2, src3, num_dbls);
  else
   dbls = _quad_double_max_4way_align32B(dst, src0, src1, src2, src3,  num_dbls);

  if (num_dbls - dbls)
   _quad_double_max_4way_single(dst+dbls, src0+dbls, src1+dbls, src2+dbls, src3+dbls, num_dbls - dbls);

  return num_dbls;
}



#endif
