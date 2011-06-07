/**
 * \file components/devices/shmem/qpx_2way_max.h
 * \brief ???
 */
#ifndef __components_devices_shmem_qpx_2way_max_h__
#define __components_devices_shmem_qpx_2way_max_h__

#include "asmheader.h"

//atleast 128B(16 doubles) ..in multiples of 64B
//alignment expected is L1 cache line == 64B

inline unsigned _quad_double_max_2way_align64B(double* dst, double* src0, double *src1, unsigned num_dbls)
{

  double *src0_1, *src1_1;
  double *src0_2, *src1_2;
  double *dst_1, *dst_2;
  register int inc ;
  register int	y;

  src0_1 = src0 -8;
  src1_1 = src1 -8;
  src0_2 = src0 -4;
  src1_2 = src1 -4;

  dst_1 = dst -8;
  dst_2 = dst -4;

  inc=64;

  y = (num_dbls >> 3) >> 2;

  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_1,inc,2);
  VECTOR_LOAD(src1_1,inc,3);
  VECTOR_LOAD(src0_1,inc,4);
  VECTOR_LOAD(src1_1,inc,5);
  VECTOR_LOAD(src0_1,inc,6);
  VECTOR_LOAD(src1_1,inc,7);

  if (y == 1)
  {
    //ASM("qvfadd 8, 0, 1" ::: "fr8"  );
    ASM("qvfcmpgt 31, 0,1" ::: "fr31" );
    ASM("qvfsel 8, 31, 0, 1" ::: "fr8" );

    VECTOR_LOAD(src0_2,inc,16);
    VECTOR_LOAD(src1_2,inc,17);

    //ASM("qvfadd 9, 2, 3" ::: "fr9"  );
    ASM("qvfcmpgt 31, 2,3" ::: "fr31" );
    ASM("qvfsel 9, 31, 2, 3" ::: "fr9" );

    VECTOR_LOAD(src0_2,inc,18);
    VECTOR_LOAD(src1_2,inc,19);

    VECTOR_STORE(dst_1,inc,8);
    //ASM("qvfadd 10, 4, 5" ::: "fr10"  );
    ASM("qvfcmpgt 31, 4,5" ::: "fr31" );
    ASM("qvfsel 10, 31, 4, 5" ::: "fr10" );
    //ASM("qvfadd 24, 16, 17" ::: "fr24"  );
    ASM("qvfcmpgt 31, 16,17" ::: "fr31" );
    ASM("qvfsel 24, 31, 16, 17" ::: "fr24" );

    VECTOR_LOAD(src0_2,inc,20);
    VECTOR_LOAD(src1_2,inc,21);

    VECTOR_STORE(dst_1,inc,9);
    //ASM("qvfadd 11, 6, 7" ::: "fr11"  );
    ASM("qvfcmpgt 31, 6,7" ::: "fr31" );
    ASM("qvfsel 11, 31, 6, 7" ::: "fr11" );
    //ASM("qvfadd 25, 18, 19" ::: "fr25"  );
    ASM("qvfcmpgt 31, 18, 19" ::: "fr31" );
    ASM("qvfsel 25, 31, 18, 19" ::: "fr25" );

    VECTOR_LOAD(src0_2,inc,22);
    VECTOR_LOAD(src1_2,inc,23);

    VECTOR_STORE(dst_1,inc,10);
    VECTOR_STORE(dst_2,inc,24);
    //ASM("qvfadd 26, 20, 21" ::: "fr26"  );
    ASM("qvfcmpgt 31, 20, 21" ::: "fr31" );
    ASM("qvfsel 26, 31, 20, 21" ::: "fr26" );

    VECTOR_STORE(dst_1,inc,11);
    VECTOR_STORE(dst_2,inc,25);
    //ASM("qvfadd 27, 22, 23" ::: "fr27"  );
    ASM("qvfcmpgt 31, 22, 23" ::: "fr31" );
    ASM("qvfsel 27, 31, 22, 23" ::: "fr27" );

    VECTOR_STORE(dst_2,inc,26);
    VECTOR_STORE(dst_2,inc,27);

    return 32;
  }

  //ASM("qvfadd 8, 0, 1" ::: "fr8"  );
  ASM("qvfcmpgt 31, 0,1" ::: "fr31" );
  ASM("qvfsel 8, 31, 0, 1" ::: "fr8" );

  VECTOR_LOAD(src0_1,inc,0); //5
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_2,inc,16);
  VECTOR_LOAD(src1_2,inc,17);

  //ASM("qvfadd 9, 2, 3" ::: "fr9"  );
  ASM("qvfcmpgt 31, 2,3" ::: "fr31" );
  ASM("qvfsel 9, 31, 2, 3" ::: "fr9" );

  VECTOR_LOAD(src0_1,inc,2);//6
  VECTOR_LOAD(src1_1,inc,3);
  VECTOR_LOAD(src0_2,inc,18);
  VECTOR_LOAD(src1_2,inc,19);


  VECTOR_STORE(dst_1,inc,8);
  //ASM("qvfadd 10, 4, 5" ::: "fr10"  );
  ASM("qvfcmpgt 31, 4,5" ::: "fr31" );
  ASM("qvfsel 10, 31, 4, 5" ::: "fr10" );
  //ASM("qvfadd 24, 16, 17" ::: "fr24"  );
  ASM("qvfcmpgt 31, 16,17" ::: "fr31" );
  ASM("qvfsel 24, 31, 16, 17" ::: "fr24" );

  VECTOR_LOAD(src0_1,inc,4); //7
  VECTOR_LOAD(src1_1,inc,5);
  VECTOR_LOAD(src0_2,inc,20);
  VECTOR_LOAD(src1_2,inc,21);

  VECTOR_STORE(dst_1,inc,9);
  //ASM("qvfadd 11, 6, 7" ::: "fr11"  );
  ASM("qvfcmpgt 31, 6,7" ::: "fr31" );
  ASM("qvfsel 11, 31, 6, 7" ::: "fr11" );
  //ASM("qvfadd 25, 18, 19" ::: "fr25"  );
  ASM("qvfcmpgt 31, 18, 19" ::: "fr31" );
  ASM("qvfsel 25, 31, 18, 19" ::: "fr25" );

  VECTOR_LOAD(src0_1,inc,6); //8
  VECTOR_LOAD(src1_1,inc,7);
  VECTOR_LOAD(src0_2,inc,22);
  VECTOR_LOAD(src1_2,inc,23);

  VECTOR_STORE(dst_1,inc,10);
  VECTOR_STORE(dst_2,inc,24);
  //ASM("qvfadd 8, 0, 1" ::: "fr8"  );
  ASM("qvfcmpgt 31, 0,1" ::: "fr31" );
  ASM("qvfsel 8, 31, 0, 1" ::: "fr8" );
  //ASM("qvfadd 26, 20, 21" ::: "fr26"  );
  ASM("qvfcmpgt 31, 20, 21" ::: "fr31" );
  ASM("qvfsel 26, 31, 20, 21" ::: "fr26" );

   //y-=4;
   y-=2;

  while (y>0)
  {
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_2,inc,16);
  VECTOR_LOAD(src1_2,inc,17);

  VECTOR_STORE(dst_1,inc,11);
  VECTOR_STORE(dst_2,inc,25);
  //ASM("qvfadd 9, 2, 3" ::: "fr9"  );
  ASM("qvfcmpgt 31, 2,3" ::: "fr31" );
  ASM("qvfsel 9, 31, 2, 3" ::: "fr9" );
  //ASM("qvfadd 27, 22, 23" ::: "fr27"  );
  ASM("qvfcmpgt 31, 22, 23" ::: "fr31" );
  ASM("qvfsel 27, 31, 22, 23" ::: "fr27" );

  VECTOR_LOAD(src0_1,inc,2);
  VECTOR_LOAD(src1_1,inc,3);
  VECTOR_LOAD(src0_2,inc,18);
  VECTOR_LOAD(src1_2,inc,19);

  VECTOR_STORE(dst_1,inc,8);
  VECTOR_STORE(dst_2,inc,26);
  //ASM("qvfadd 10, 4, 5" ::: "fr10"  );
  ASM("qvfcmpgt 31, 4,5" ::: "fr31" );
  ASM("qvfsel 10, 31, 4, 5" ::: "fr10" );
  //ASM("qvfadd 24, 16, 17" ::: "fr24"  );
  ASM("qvfcmpgt 31, 16,17" ::: "fr31" );
  ASM("qvfsel 24, 31, 16, 17" ::: "fr24" );

  VECTOR_LOAD(src0_1,inc,4);
  VECTOR_LOAD(src1_1,inc,5);
  VECTOR_LOAD(src0_2,inc,20);
  VECTOR_LOAD(src1_2,inc,21);

  VECTOR_STORE(dst_1,inc,9);
  VECTOR_STORE(dst_2,inc,27);
  //ASM("qvfadd 11, 6, 7" ::: "fr11"  );
  ASM("qvfcmpgt 31, 6,7" ::: "fr31" );
  ASM("qvfsel 11, 31, 6, 7" ::: "fr11" );
  //ASM("qvfadd 25, 18, 19" ::: "fr25"  );
  ASM("qvfcmpgt 31, 18, 19" ::: "fr31" );
  ASM("qvfsel 25, 31, 18, 19" ::: "fr25" );

  VECTOR_LOAD(src0_1,inc,6);
  VECTOR_LOAD(src1_1,inc,7);
  VECTOR_LOAD(src0_2,inc,22);
  VECTOR_LOAD(src1_2,inc,23);

  VECTOR_STORE(dst_1,inc,10);
  VECTOR_STORE(dst_2,inc,24);
  //ASM("qvfadd 8, 0, 1" ::: "fr8"  );
  ASM("qvfcmpgt 31, 0,1" ::: "fr31" );
  ASM("qvfsel 8, 31, 0, 1" ::: "fr8" );
  //ASM("qvfadd 26, 20, 21" ::: "fr26"  );
  ASM("qvfcmpgt 31, 20, 21" ::: "fr31" );
  ASM("qvfsel 26, 31, 20, 21" ::: "fr26" );

	y-=1;
	//y-=4;

  }

  VECTOR_LOAD(src0_2,inc,16);
  VECTOR_LOAD(src1_2,inc,17);

  VECTOR_STORE(dst_1,inc,11);
  VECTOR_STORE(dst_2,inc,25);
  //ASM("qvfadd 9, 2, 3" ::: "fr9"  );
  ASM("qvfcmpgt 31, 2,3" ::: "fr31" );
  ASM("qvfsel 9, 31, 2, 3" ::: "fr9" );
  //ASM("qvfadd 27, 22, 23" ::: "fr27"  );
  ASM("qvfcmpgt 31, 22, 23" ::: "fr31" );
  ASM("qvfsel 27, 31, 22, 23" ::: "fr27" );

  VECTOR_LOAD(src0_2,inc,18);
  VECTOR_LOAD(src1_2,inc,19);

  VECTOR_STORE(dst_1,inc,8);
  VECTOR_STORE(dst_2,inc,26);
  //ASM("qvfadd 10, 4, 5" ::: "fr10"  );
  ASM("qvfcmpgt 31, 4,5" ::: "fr31" );
  ASM("qvfsel 10, 31, 4, 5" ::: "fr10" );
  //ASM("qvfadd 24, 16, 17" ::: "fr24"  );
  ASM("qvfcmpgt 31, 16,17" ::: "fr31" );
  ASM("qvfsel 24, 31, 16, 17" ::: "fr24" );

  VECTOR_LOAD(src0_2,inc,20);
  VECTOR_LOAD(src1_2,inc,21);

  VECTOR_STORE(dst_1,inc,9);
  VECTOR_STORE(dst_2,inc,27);
  //ASM("qvfadd 11, 6, 7" ::: "fr11"  );
  ASM("qvfcmpgt 31, 6,7" ::: "fr31" );
  ASM("qvfsel 11, 31, 6, 7" ::: "fr11" );
  //ASM("qvfadd 25, 18, 19" ::: "fr25"  );
  ASM("qvfcmpgt 31, 18, 19" ::: "fr31" );
  ASM("qvfsel 25, 31, 18, 19" ::: "fr25" );

  VECTOR_LOAD(src0_2,inc,22);
  VECTOR_LOAD(src1_2,inc,23);

  VECTOR_STORE(dst_1,inc,10);
  VECTOR_STORE(dst_2,inc,24);
  //ASM("qvfadd 26, 20, 21" ::: "fr26"  );
  ASM("qvfcmpgt 31, 20, 21" ::: "fr31" );
  ASM("qvfsel 26, 31, 20, 21" ::: "fr26" );

  VECTOR_STORE(dst_1,inc,11);
  VECTOR_STORE(dst_2,inc,25);
  //ASM("qvfadd 27, 22, 23" ::: "fr27"  );
  ASM("qvfcmpgt 31, 22, 23" ::: "fr31" );
  ASM("qvfsel 27, 31, 22, 23" ::: "fr27" );
  VECTOR_STORE(dst_2,inc,26);
  VECTOR_STORE(dst_2,inc,27);

  return ((num_dbls >> (3+2)) << (3+2));
}

inline unsigned _quad_double_max_2way_single(double* dst, double* src0, double *src1, unsigned num_dbls)
{

  unsigned i;
  for (i = 0; i < num_dbls; i++)
  {
    //printf("src0[%d]:%f,src1[%d]:%f\n", i,src0[i],i, src1[i]);
    dst[i] = (src0[i] > src1[i]) ? src0[i]:src1[i];
  }

  return num_dbls;

}

inline unsigned _quad_double_max_2way_align32B(double* dst, double* src0, double *src1, unsigned num_dbls)
{

  unsigned  dbls = 0;
  uint64_t  mask;
  mask = (uint64_t)dst|(uint64_t)src0|(uint64_t)src1; 
  bool is_64B_aligned;
  is_64B_aligned = !((mask & ((uint64_t)63)) == 0);
  
  if (is_64B_aligned)
  {
    //printf("64B aligned\n");
    dbls = _quad_double_max_2way_align64B(dst, src0, src1, num_dbls);
    if (dbls == num_dbls) return dbls;
      //printf("dbls:%u remaining:%u\n", dbls, num_dbls-dbls);
    _quad_double_max_2way_single(dst+dbls, src0+dbls, src1+dbls, num_dbls - dbls);
    return  num_dbls;
  }

  _quad_double_max_2way_single(dst, src0, src1, 4);

  dbls = _quad_double_max_2way_align64B(dst+4, src0+4, src1+4, num_dbls - 4);
  dbls+=4;

  //printf("dbls:%u\n", dbls);
  return dbls;

}

//buffers aligned to 32B 
inline unsigned quad_double_max_2way(double* dst, double* src0, double *src1, unsigned num_dbls)
{
  
  //assert(0);
  unsigned  dbls = 0;

  if (num_dbls >= 36)
    dbls = _quad_double_max_2way_align32B(dst, src0, src1, num_dbls);

  if (num_dbls - dbls)
    _quad_double_max_2way_single(dst+dbls, src0+dbls, src1+dbls, num_dbls - dbls);

  return num_dbls;
}

#endif
