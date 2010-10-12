/**
 * \file components/devices/shmemcoll/2way_sum.h
 * \brief ???
 */
//adding num doubles
inline int quad_double_sum_2way(double* dst, double* src0, double* src1,  uint64_t num )
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

  y= num/8;

  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_1,inc,2);
  VECTOR_LOAD(src1_1,inc,3);
  VECTOR_LOAD(src0_1,inc,4);
  VECTOR_LOAD(src1_1,inc,5);
  VECTOR_LOAD(src0_1,inc,6);
  VECTOR_LOAD(src1_1,inc,7);

  ASM("qvfadd 8, 0, 1" ::  );

  VECTOR_LOAD(src0_1,inc,0); //5
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_2,inc,16);
  VECTOR_LOAD(src1_2,inc,17);

  ASM("qvfadd 9, 2, 3" ::  );

  VECTOR_LOAD(src0_1,inc,2);//6
  VECTOR_LOAD(src1_1,inc,3);
  VECTOR_LOAD(src0_2,inc,18);
  VECTOR_LOAD(src1_2,inc,19);


  VECTOR_STORE(dst_1,inc,8);
  ASM("qvfadd 10, 4, 5" ::  );
  ASM("qvfadd 24, 16, 17" ::  );

  VECTOR_LOAD(src0_1,inc,4); //7
  VECTOR_LOAD(src1_1,inc,5);
  VECTOR_LOAD(src0_2,inc,20);
  VECTOR_LOAD(src1_2,inc,21);

  VECTOR_STORE(dst_1,inc,9);
  ASM("qvfadd 11, 6, 7" ::  );
  ASM("qvfadd 25, 18, 19" ::  );

  VECTOR_LOAD(src0_1,inc,6); //8
  VECTOR_LOAD(src1_1,inc,7);
  VECTOR_LOAD(src0_2,inc,22);
  VECTOR_LOAD(src1_2,inc,23);

  VECTOR_STORE(dst_1,inc,10);
  VECTOR_STORE(dst_2,inc,24);
  ASM("qvfadd 8, 0, 1" ::  );
  ASM("qvfadd 26, 20, 21" ::  );

   y-=4;

  while (y>0)
  {
  VECTOR_LOAD(src0_1,inc,0);
  VECTOR_LOAD(src1_1,inc,1);
  VECTOR_LOAD(src0_2,inc,16);
  VECTOR_LOAD(src1_2,inc,17);

  VECTOR_STORE(dst_1,inc,11);
  VECTOR_STORE(dst_2,inc,25);
  ASM("qvfadd 9, 2, 3" ::  );
  ASM("qvfadd 27, 22, 23" ::  );

  VECTOR_LOAD(src0_1,inc,2);
  VECTOR_LOAD(src1_1,inc,3);
  VECTOR_LOAD(src0_2,inc,18);
  VECTOR_LOAD(src1_2,inc,19);

  VECTOR_STORE(dst_1,inc,8);
  VECTOR_STORE(dst_2,inc,26);
  ASM("qvfadd 10, 4, 5" ::  );
  ASM("qvfadd 24, 16, 17" ::  );

  VECTOR_LOAD(src0_1,inc,4);
  VECTOR_LOAD(src1_1,inc,5);
  VECTOR_LOAD(src0_2,inc,20);
  VECTOR_LOAD(src1_2,inc,21);

  VECTOR_STORE(dst_1,inc,9);
  VECTOR_STORE(dst_2,inc,27);
  ASM("qvfadd 11, 6, 7" ::  );
  ASM("qvfadd 25, 18, 19" ::  );

  VECTOR_LOAD(src0_1,inc,6);
  VECTOR_LOAD(src1_1,inc,7);
  VECTOR_LOAD(src0_2,inc,22);
  VECTOR_LOAD(src1_2,inc,23);

  VECTOR_STORE(dst_1,inc,10);
  VECTOR_STORE(dst_2,inc,24);
  ASM("qvfadd 8, 0, 1" ::  );
  ASM("qvfadd 26, 20, 21" ::  );

	y-=4;

  }

  VECTOR_LOAD(src0_2,inc,16);
  VECTOR_LOAD(src1_2,inc,17);

  VECTOR_STORE(dst_1,inc,11);
  VECTOR_STORE(dst_2,inc,25);
  ASM("qvfadd 9, 2, 3" ::  );
  ASM("qvfadd 27, 22, 23" ::  );

  VECTOR_LOAD(src0_2,inc,18);
  VECTOR_LOAD(src1_2,inc,19);

  VECTOR_STORE(dst_1,inc,8);
  VECTOR_STORE(dst_2,inc,26);
  ASM("qvfadd 10, 4, 5" ::  );
  ASM("qvfadd 24, 16, 17" ::  );

  VECTOR_LOAD(src0_2,inc,20);
  VECTOR_LOAD(src1_2,inc,21);

  VECTOR_STORE(dst_1,inc,9);
  VECTOR_STORE(dst_2,inc,27);
  ASM("qvfadd 11, 6, 7" ::  );
  ASM("qvfadd 25, 18, 19" ::  );

  VECTOR_LOAD(src0_2,inc,22);
  VECTOR_LOAD(src1_2,inc,23);

  VECTOR_STORE(dst_1,inc,10);
  VECTOR_STORE(dst_2,inc,24);
  ASM("qvfadd 26, 20, 21" ::  );

  VECTOR_STORE(dst_2,inc,25);
  ASM("qvfadd 27, 22, 23" ::  );
  VECTOR_STORE(dst_2,inc,26);
  VECTOR_STORE(dst_2,inc,27);

  return 0;
}
