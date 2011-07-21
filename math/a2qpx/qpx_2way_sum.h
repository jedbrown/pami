/**
 * \file components/devices/shmem/qpx_2way_sum.h
 * \brief ???
 */
#ifndef __components_devices_shmem_qpx_2way_sum_h__
#define __components_devices_shmem_qpx_2way_sum_h__

#include "asmheader.h"

//atleast 128B(16 doubles) ..in multiples of 64B
//alignment expected is L1 cache line == 64B

void _pami_core_double_sum_2way(double* dst, double* src0, double *src1, unsigned num_dbls);

inline unsigned _quad_double_sum_2way_align64B(double* dst, double* src0, double *src1, unsigned num_dbls)
{
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
  register double f16 __asm__("fr16");
  register double f17 __asm__("fr17");
  register double f18 __asm__("fr18");
  register double f19 __asm__("fr19");
  register double f20 __asm__("fr20");
  register double f21 __asm__("fr21");
  register double f22 __asm__("fr22");
  register double f23 __asm__("fr23");
  register double f24 __asm__("fr24");
  register double f25 __asm__("fr25");
  register double f26 __asm__("fr26");
  register double f27 __asm__("fr27");

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

  QPX_LOAD(src0_1,inc,f0);
  QPX_LOAD(src1_1,inc,f1);
  QPX_LOAD(src0_1,inc,f2);
  QPX_LOAD(src1_1,inc,f3);
  QPX_LOAD(src0_1,inc,f4);
  QPX_LOAD(src1_1,inc,f5);
  QPX_LOAD(src0_1,inc,f6);
  QPX_LOAD(src1_1,inc,f7);

  if (y == 1)
  {
    ASM("qvfadd 8, 0, 1" ::: "fr8"  );

    QPX_LOAD(src0_2,inc,f16);
    QPX_LOAD(src1_2,inc,f17);

    ASM("qvfadd 9, 2, 3" ::: "fr9"  );

    QPX_LOAD(src0_2,inc,f18);
    QPX_LOAD(src1_2,inc,f19);

    QPX_STORE(dst_1,inc,f8);
    ASM("qvfadd 10, 4, 5" ::: "fr10"  );
    ASM("qvfadd 24, 16, 17" ::: "fr24"  );

    QPX_LOAD(src0_2,inc,f20);
    QPX_LOAD(src1_2,inc,f21);

    QPX_STORE(dst_1,inc,f9);
    ASM("qvfadd 11, 6, 7" ::: "fr11"  );
    ASM("qvfadd 25, 18, 19" ::: "fr25"  );

    QPX_LOAD(src0_2,inc,f22);
    QPX_LOAD(src1_2,inc,f23);

    QPX_STORE(dst_1,inc,f10);
    QPX_STORE(dst_2,inc,f24);
    ASM("qvfadd 26, 20, 21" ::: "fr26"  );

    QPX_STORE(dst_1,inc,f11);
    QPX_STORE(dst_2,inc,f25);
    ASM("qvfadd 27, 22, 23" ::: "fr27"  );

    QPX_STORE(dst_2,inc,f26);
    QPX_STORE(dst_2,inc,f27);

    return 32;
  }

  ASM("qvfadd 8, 0, 1" ::: "fr8"  );

  QPX_LOAD(src0_1,inc,f0); //5
  QPX_LOAD(src1_1,inc,f1);
  QPX_LOAD(src0_2,inc,f16);
  QPX_LOAD(src1_2,inc,f17);

  ASM("qvfadd 9, 2, 3" ::: "fr9"  );

  QPX_LOAD(src0_1,inc,f2);//6
  QPX_LOAD(src1_1,inc,f3);
  QPX_LOAD(src0_2,inc,f18);
  QPX_LOAD(src1_2,inc,f19);


  QPX_STORE(dst_1,inc,f8);
  ASM("qvfadd 10, 4, 5" ::: "fr10"  );
  ASM("qvfadd 24, 16, 17" ::: "fr24"  );

  QPX_LOAD(src0_1,inc,f4); //7
  QPX_LOAD(src1_1,inc,f5);
  QPX_LOAD(src0_2,inc,f20);
  QPX_LOAD(src1_2,inc,f21);

  QPX_STORE(dst_1,inc,f9);
  ASM("qvfadd 11, 6, 7" ::: "fr11"  );
  ASM("qvfadd 25, 18, 19" ::: "fr25"  );

  QPX_LOAD(src0_1,inc,f6); //8
  QPX_LOAD(src1_1,inc,f7);
  QPX_LOAD(src0_2,inc,f22);
  QPX_LOAD(src1_2,inc,f23);

  QPX_STORE(dst_1,inc,f10);
  QPX_STORE(dst_2,inc,f24);
  ASM("qvfadd 8, 0, 1" ::: "fr8"  );
  ASM("qvfadd 26, 20, 21" ::: "fr26"  );

   //y-=4;
   y-=2;

  while (y>0)
  {
  QPX_LOAD(src0_1,inc,f0);
  QPX_LOAD(src1_1,inc,f1);
  QPX_LOAD(src0_2,inc,f16);
  QPX_LOAD(src1_2,inc,f17);

  QPX_STORE(dst_1,inc,f11);
  QPX_STORE(dst_2,inc,f25);
  ASM("qvfadd 9, 2, 3" ::: "fr9"  );
  ASM("qvfadd 27, 22, 23" ::: "fr27"  );

  QPX_LOAD(src0_1,inc,f2);
  QPX_LOAD(src1_1,inc,f3);
  QPX_LOAD(src0_2,inc,f18);
  QPX_LOAD(src1_2,inc,f19);

  QPX_STORE(dst_1,inc,f8);
  QPX_STORE(dst_2,inc,f26);
  ASM("qvfadd 10, 4, 5" ::: "fr10"  );
  ASM("qvfadd 24, 16, 17" ::: "fr24"  );

  QPX_LOAD(src0_1,inc,f4);
  QPX_LOAD(src1_1,inc,f5);
  QPX_LOAD(src0_2,inc,f20);
  QPX_LOAD(src1_2,inc,f21);

  QPX_STORE(dst_1,inc,f9);
  QPX_STORE(dst_2,inc,f27);
  ASM("qvfadd 11, 6, 7" ::: "fr11"  );
  ASM("qvfadd 25, 18, 19" ::: "fr25"  );

  QPX_LOAD(src0_1,inc,f6);
  QPX_LOAD(src1_1,inc,f7);
  QPX_LOAD(src0_2,inc,f22);
  QPX_LOAD(src1_2,inc,f23);

  QPX_STORE(dst_1,inc,f10);
  QPX_STORE(dst_2,inc,f24);
  ASM("qvfadd 8, 0, 1" ::: "fr8"  );
  ASM("qvfadd 26, 20, 21" ::: "fr26"  );

	y-=1;
	//y-=4;

  }

  QPX_LOAD(src0_2,inc,f16);
  QPX_LOAD(src1_2,inc,f17);

  QPX_STORE(dst_1,inc,f11);
  QPX_STORE(dst_2,inc,f25);
  ASM("qvfadd 9, 2, 3" ::: "fr9"  );
  ASM("qvfadd 27, 22, 23" ::: "fr27"  );

  QPX_LOAD(src0_2,inc,f18);
  QPX_LOAD(src1_2,inc,f19);

  QPX_STORE(dst_1,inc,f8);
  QPX_STORE(dst_2,inc,f26);
  ASM("qvfadd 10, 4, 5" ::: "fr10"  );
  ASM("qvfadd 24, 16, 17" ::: "fr24"  );

  QPX_LOAD(src0_2,inc,f20);
  QPX_LOAD(src1_2,inc,f21);

  QPX_STORE(dst_1,inc,f9);
  QPX_STORE(dst_2,inc,f27);
  ASM("qvfadd 11, 6, 7" ::: "fr11"  );
  ASM("qvfadd 25, 18, 19" ::: "fr25"  );

  QPX_LOAD(src0_2,inc,f22);
  QPX_LOAD(src1_2,inc,f23);

  QPX_STORE(dst_1,inc,f10);
  QPX_STORE(dst_2,inc,f24);
  ASM("qvfadd 26, 20, 21" ::: "fr26"  );

  QPX_STORE(dst_1,inc,f11);
  QPX_STORE(dst_2,inc,f25);
  ASM("qvfadd 27, 22, 23" ::: "fr27"  );
  QPX_STORE(dst_2,inc,f26);
  QPX_STORE(dst_2,inc,f27);

  return ((num_dbls >> (3+2)) << (3+2));
}

inline unsigned _quad_double_sum_2way_single(double* dst, double* src0, double *src1, unsigned num_dbls)
{

  unsigned i;
  for (i = 0; i < num_dbls; i++)
  {
    //printf("src0[%d]:%f,src1[%d]:%f\n", i,src0[i],i, src1[i]);
    dst[i] = src0[i] + src1[i];
  }

  return num_dbls;

}

inline unsigned _quad_double_sum_2way_align32B(double* dst, double* src0, double *src1, unsigned num_dbls)
{

  unsigned  dbls = 0;
  uint64_t  mask;
  mask = (uint64_t)dst|(uint64_t)src0|(uint64_t)src1; 
  bool is_64B_aligned;
  is_64B_aligned = !((mask & ((uint64_t)63)) == 0);
  
  if (is_64B_aligned)
  {
    //printf("64B aligned\n");
    dbls = _quad_double_sum_2way_align64B(dst, src0, src1, num_dbls);
    if (dbls == num_dbls) return dbls;
      //printf("dbls:%u remaining:%u\n", dbls, num_dbls-dbls);
    _quad_double_sum_2way_single(dst+dbls, src0+dbls, src1+dbls, num_dbls - dbls);
    return  num_dbls;
  }

  _quad_double_sum_2way_single(dst, src0, src1, 4);

  dbls = _quad_double_sum_2way_align64B(dst+4, src0+4, src1+4, num_dbls - 4);
  dbls+=4;

  //printf("dbls:%u\n", dbls);
  return dbls;

}

//buffers aligned to 32B 
inline unsigned quad_double_sum_2way(double* dst, double* src0, double *src1, unsigned num_dbls)
{
  
  const uint64_t      alignment = 32;
  uint64_t    mask    = 0;
  mask = (alignment - 1);

  if (mask & ((uint64_t)src0|(uint64_t)src1|(uint64_t)dst))
  {
    _pami_core_double_sum_2way(dst, src0, src1, num_dbls);
    return 0;
  }

  unsigned  dbls = 0;

  if (num_dbls >= 36)
    dbls = _quad_double_sum_2way_align32B(dst, src0, src1, num_dbls);

  if (num_dbls - dbls)
    _quad_double_sum_2way_single(dst+dbls, src0+dbls, src1+dbls, num_dbls - dbls);

  return num_dbls;
}

#endif
