/**
 * \file components/devices/shmem/bgq_math.h
 * \brief ???
 */
#ifndef __components_devices_shmem_bgq_math_h__
#define __components_devices_shmem_bgq_math_h__
//#define "components/devices/shmem/a2qpx_nway_math.h"

void _pami_core_int_min_2way(int* dst, int* src0, int *src1, unsigned num_ints);
void _pami_core_int_max_2way(int* dst, int* src0, int *src1, unsigned num_ints);
void _pami_core_int_sum_2way(int* dst, int* src0, int *src1, unsigned num_ints);

inline unsigned int_math_2way(int* dst, int* src0, int *src1, unsigned num_ints, pami_op opcode)
{
  switch (opcode)
  {
    case PAMI_SUM:
   _pami_core_int_sum_2way( dst,  src0,  src1, num_ints );
    break;

    case PAMI_MAX:
  _pami_core_int_max_2way( dst,  src0,  src1, num_ints );
      break;

    case PAMI_MIN:
  _pami_core_int_min_2way( dst,  src0,  src1, num_ints );
      break;

    case PAMI_COPY:
    case PAMI_NOOP:
    case PAMI_PROD:
    case PAMI_LAND:
    case PAMI_LOR:
    case PAMI_LXOR:
    case PAMI_BAND:
    case PAMI_BOR:
    case PAMI_BXOR:
    case PAMI_MAXLOC:
    case PAMI_MINLOC:
    case PAMI_OP_COUNT:
      printf("not supported\n");
      exit(0);
    break;
  } 
  return num_ints;
}

void _pami_core_int_min_4way(int* dst, int* src0, int *src1, int* src2, int* src3, unsigned num_ints);
void _pami_core_int_max_4way(int* dst, int* src0, int *src1, int* src2, int* src3, unsigned num_ints);
void _pami_core_int_sum_4way(int* dst, int* src0, int *src1, int* src2, int* src3, unsigned num_ints);
inline unsigned int_math_4way(int* dst, int* src0, int *src1, int* src2, int* src3, unsigned num_ints, pami_op opcode)
{
  switch (opcode )
  {
    case PAMI_SUM:
   _pami_core_int_sum_4way( dst,  src0,  src1, src2,  src3, num_ints );
      break;

    case PAMI_MAX:
   _pami_core_int_max_4way( dst,  src0,  src1, src2,  src3, num_ints );
      break;

    case PAMI_MIN:
   _pami_core_int_min_4way( dst,  src0,  src1, src2,  src3, num_ints );
      break;

    case PAMI_COPY:
    case PAMI_NOOP:
    case PAMI_PROD:
    case PAMI_LAND:
    case PAMI_LOR:
    case PAMI_LXOR:
    case PAMI_BAND:
    case PAMI_BOR:
    case PAMI_BXOR:
    case PAMI_MAXLOC:
    case PAMI_MINLOC:
    case PAMI_OP_COUNT:
      printf("not supported\n");
      exit(0);
    break;
  } 
  return num_ints;

}

void _pami_core_int_min_8way(int* dst, int* src0, int *src1, int* src2, int* src3,
    int* src4, int* src5, int* src6, int* src7, unsigned num_ints);
void _pami_core_int_max_8way(int* dst, int* src0, int *src1, int* src2, int* src3,
    int* src4, int* src5, int* src6, int* src7, unsigned num_ints);
void _pami_core_int_sum_8way(int* dst, int* src0, int *src1, int* src2, int* src3,
    int* src4, int* src5, int* src6, int* src7, unsigned num_ints);

inline unsigned int_math_8way(int* dst, int* src0, int *src1, int* src2, int* src3,
    int* src4, int* src5, int* src6, int* src7, unsigned num_ints, pami_op opcode)
{
  switch (opcode) 
  {
    case PAMI_SUM:
   _pami_core_int_sum_8way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7, num_ints );
      break;

    case PAMI_MAX:
   _pami_core_int_max_8way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7,  num_ints );
      break;

    case PAMI_MIN:
   _pami_core_int_min_8way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7,  num_ints );
      break;

    case PAMI_COPY:
    case PAMI_NOOP:
    case PAMI_PROD:
    case PAMI_LAND:
    case PAMI_LOR:
    case PAMI_LXOR:
    case PAMI_BAND:
    case PAMI_BOR:
    case PAMI_BXOR:
    case PAMI_MAXLOC:
    case PAMI_MINLOC:
    case PAMI_OP_COUNT:
      printf("not supported\n");
      exit(0);
    break;
  } 
  return num_ints;

}

void _pami_core_int_min_16way(int* dst, int* src0, int *src1, int* src2, int* src3,
    int* src4, int* src5, int* src6, int* src7, int* src8, int *src9, int* src10, int* src11,
    int* src12, int* src13, int* src14, int* src15, unsigned num_ints);
void _pami_core_int_max_16way(int* dst, int* src0, int *src1, int* src2, int* src3,
    int* src4, int* src5, int* src6, int* src7, int* src8, int *src9, int* src10, int* src11,
    int* src12, int* src13, int* src14, int* src15, unsigned num_ints);
void _pami_core_int_sum_16way(int* dst, int* src0, int *src1, int* src2, int* src3,
    int* src4, int* src5, int* src6, int* src7, int* src8, int *src9, int* src10, int* src11,
    int* src12, int* src13, int* src14, int* src15, unsigned num_ints);

inline unsigned int_math_16way(int* dst, int* src0, int *src1, int* src2, int* src3,
    int* src4, int* src5, int* src6, int* src7, int* src8, int *src9, int* src10, int* src11,
    int* src12, int* src13, int* src14, int* src15, unsigned num_ints, pami_op opcode)
{

  switch (opcode) 
  {
    case PAMI_SUM:
   _pami_core_int_sum_16way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15,  num_ints );
      break;

    case PAMI_MAX:
   _pami_core_int_max_16way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15,  num_ints );
      break;

    case PAMI_MIN:
   _pami_core_int_min_16way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15,  num_ints );
      break;

    case PAMI_COPY:
    case PAMI_NOOP:
    case PAMI_PROD:
    case PAMI_LAND:
    case PAMI_LOR:
    case PAMI_LXOR:
    case PAMI_BAND:
    case PAMI_BOR:
    case PAMI_BXOR:
    case PAMI_MAXLOC:
    case PAMI_MINLOC:
    case PAMI_OP_COUNT:
      printf("not supported\n");
      exit(0);
    break;
  } 
  return num_ints;

}

void _pami_core_uint_min_2way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned num_ints);
void _pami_core_uint_max_2way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned num_ints);
void _pami_core_uint_sum_2way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned num_ints);

inline unsigned uint_math_2way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned num_ints, pami_op opcode)
{
  switch (opcode)
  {
    case PAMI_SUM:
   _pami_core_uint_sum_2way( dst,  src0,  src1, num_ints );
    break;

    case PAMI_MAX:
  _pami_core_uint_max_2way( dst,  src0,  src1, num_ints );
      break;

    case PAMI_MIN:
  _pami_core_uint_min_2way( dst,  src0,  src1, num_ints );
      break;

    case PAMI_COPY:
    case PAMI_NOOP:
    case PAMI_PROD:
    case PAMI_LAND:
    case PAMI_LOR:
    case PAMI_LXOR:
    case PAMI_BAND:
    case PAMI_BOR:
    case PAMI_BXOR:
    case PAMI_MAXLOC:
    case PAMI_MINLOC:
    case PAMI_OP_COUNT:
      printf("not supported\n");
      exit(0);
    break;
  } 
  return num_ints;
}

void _pami_core_uint_min_4way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3, unsigned num_ints);
void _pami_core_uint_max_4way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3, unsigned num_ints);
void _pami_core_uint_sum_4way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3, unsigned num_ints);
inline unsigned uint_math_4way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3, unsigned num_ints, pami_op opcode)
{
  switch (opcode )
  {
    case PAMI_SUM:
   _pami_core_uint_sum_4way( dst,  src0,  src1, src2,  src3, num_ints );
      break;

    case PAMI_MAX:
   _pami_core_uint_max_4way( dst,  src0,  src1, src2,  src3, num_ints );
      break;

    case PAMI_MIN:
   _pami_core_uint_min_4way( dst,  src0,  src1, src2,  src3, num_ints );
      break;

    case PAMI_COPY:
    case PAMI_NOOP:
    case PAMI_PROD:
    case PAMI_LAND:
    case PAMI_LOR:
    case PAMI_LXOR:
    case PAMI_BAND:
    case PAMI_BOR:
    case PAMI_BXOR:
    case PAMI_MAXLOC:
    case PAMI_MINLOC:
    case PAMI_OP_COUNT:
      printf("not supported\n");
      exit(0);
    break;
  } 
  return num_ints;

}

void _pami_core_uint_min_8way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3,
    unsigned* src4, unsigned* src5, unsigned* src6, unsigned* src7, unsigned num_ints);
void _pami_core_uint_max_8way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3,
    unsigned* src4, unsigned* src5, unsigned* src6, unsigned* src7, unsigned num_ints);
void _pami_core_uint_sum_8way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3,
    unsigned* src4, unsigned* src5, unsigned* src6, unsigned* src7, unsigned num_ints);

inline unsigned uint_math_8way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3,
    unsigned* src4, unsigned* src5, unsigned* src6, unsigned* src7, unsigned num_ints, pami_op opcode)
{
  switch (opcode) 
  {
    case PAMI_SUM:
   _pami_core_uint_sum_8way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7, num_ints );
      break;

    case PAMI_MAX:
   _pami_core_uint_max_8way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7,  num_ints );
      break;

    case PAMI_MIN:
   _pami_core_uint_min_8way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7,  num_ints );
      break;

    case PAMI_COPY:
    case PAMI_NOOP:
    case PAMI_PROD:
    case PAMI_LAND:
    case PAMI_LOR:
    case PAMI_LXOR:
    case PAMI_BAND:
    case PAMI_BOR:
    case PAMI_BXOR:
    case PAMI_MAXLOC:
    case PAMI_MINLOC:
    case PAMI_OP_COUNT:
      printf("not supported\n");
      exit(0);
    break;
  } 
  return num_ints;

}

void _pami_core_uint_min_16way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3,
    unsigned* src4, unsigned* src5, unsigned* src6, unsigned* src7, unsigned* src8, unsigned* src9, unsigned* src10, unsigned* src11, unsigned* src12, unsigned* src13, unsigned* src14, unsigned* src15, unsigned num_ints);
void _pami_core_uint_max_16way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3,
    unsigned* src4, unsigned* src5, unsigned* src6, unsigned* src7, unsigned* src8, unsigned* src9, unsigned* src10, unsigned* src11, unsigned* src12, unsigned* src13, unsigned* src14, unsigned* src15, unsigned num_ints);
void _pami_core_uint_sum_16way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3,
    unsigned* src4, unsigned* src5, unsigned* src6, unsigned* src7, unsigned* src8, unsigned* src9, unsigned* src10, unsigned* src11, unsigned* src12, unsigned* src13, unsigned* src14, unsigned* src15, unsigned num_ints);

inline unsigned uint_math_16way(unsigned* dst, unsigned* src0, unsigned* src1, unsigned* src2, unsigned* src3,
    unsigned* src4, unsigned* src5, unsigned* src6, unsigned* src7, unsigned* src8, unsigned* src9, unsigned* src10, unsigned* src11, unsigned* src12, unsigned* src13, unsigned* src14, unsigned* src15, unsigned num_ints, pami_op opcode)
{

  switch (opcode) 
  {
    case PAMI_SUM:
   _pami_core_uint_sum_16way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15,  num_ints );
      break;

    case PAMI_MAX:
   _pami_core_uint_max_16way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15,  num_ints );
      break;

    case PAMI_MIN:
   _pami_core_uint_min_16way( dst,  src0,  src1, src2,  src3, src4, src5, src6, src7, src8, src9, src10, src11, src12, src13, src14, src15,  num_ints );
      break;

    case PAMI_COPY:
    case PAMI_NOOP:
    case PAMI_PROD:
    case PAMI_LAND:
    case PAMI_LOR:
    case PAMI_LXOR:
    case PAMI_BAND:
    case PAMI_BOR:
    case PAMI_BXOR:
    case PAMI_MAXLOC:
    case PAMI_MINLOC:
    case PAMI_OP_COUNT:
      printf("not supported\n");
      exit(0);
    break;
  } 
  return num_ints;

}

inline unsigned bgq_math_16way(char* dst, char* src0, char *src1, char* src2, char* src3,
    char* src4, char* src5, char* src6, char* src7, char* src8, char *src9, char* src10, char* src11,
    char* src12, char* src13, char* src14, char* src15, unsigned bytes, pami_op opcode, pami_dt dt)
{

  if (dt == PAMI_DOUBLE)
  {
    return quad_double_math_16way((double*) dst, (double*) src0, (double*)src1, (double*) src2, (double*) src3,
    (double*) src4, (double*) src5, (double*) src6, (double*) src7, (double*) src8, (double*)src9, (double*) src10, (double*) src11,
    (double*) src12, (double*) src13, (double*) src14, (double*) src15, bytes/sizeof(double), opcode);
  }
  else if (dt == PAMI_SIGNED_INT)
  {
    return int_math_16way((int*) dst, (int*) src0, (int*)src1, (int*) src2, (int*) src3,
    (int*) src4, (int*) src5, (int*) src6, (int*) src7, (int*) src8, (int*)src9, (int*) src10, (int*) src11,
    (int*) src12, (int*) src13, (int*) src14, (int*) src15, bytes/sizeof(int), opcode);
  }
  else
  {
    return uint_math_16way((unsigned*) dst, (unsigned*) src0, (unsigned*)src1, (unsigned*) src2, (unsigned*) src3,
    (unsigned*) src4, (unsigned*) src5, (unsigned*) src6, (unsigned*) src7, (unsigned*) src8, (unsigned*)src9, (unsigned*) src10, 
    (unsigned*) src11, (unsigned*) src12, (unsigned*) src13, (unsigned*) src14, (unsigned*) src15, bytes/sizeof(unsigned), opcode);
  }

}

inline unsigned bgq_math_8way(char* dst, char* src0, char *src1, char* src2, char* src3,
    char* src4, char* src5, char* src6, char* src7, unsigned bytes, pami_op opcode, pami_dt dt)
{

  if (dt == PAMI_DOUBLE)
  {
    return quad_double_math_8way((double*) dst, (double*) src0, (double*)src1, (double*) src2, (double*) src3,
    (double*) src4, (double*) src5, (double*) src6, (double*) src7, bytes/sizeof(double), opcode);
  }
  else if (dt == PAMI_SIGNED_INT)
  {
    return int_math_8way((int*) dst, (int*) src0, (int*)src1, (int*) src2, (int*) src3,
    (int*) src4, (int*) src5, (int*) src6, (int*) src7, bytes/sizeof(int), opcode);
  }
  else
  {
    return uint_math_8way((unsigned*) dst, (unsigned*) src0, (unsigned*)src1, (unsigned*) src2, (unsigned*) src3,
    (unsigned*) src4, (unsigned*) src5, (unsigned*) src6, (unsigned*) src7, bytes/sizeof(unsigned), opcode);
  }

}

inline unsigned bgq_math_4way(char* dst, char* src0, char *src1, char* src2, char* src3, unsigned bytes, pami_op opcode, pami_dt dt)
{

  if (dt == PAMI_DOUBLE)
  {
   return  quad_double_math_4way((double*) dst, (double*) src0, (double*)src1, (double*) src2, (double*) src3, bytes/sizeof(double), opcode);
  }
  else if (dt == PAMI_SIGNED_INT)
  {
    return int_math_4way((int*) dst, (int*) src0, (int*)src1, (int*) src2, (int*) src3, bytes/sizeof(int), opcode);
  }
  else
  {
    return uint_math_4way((unsigned*) dst, (unsigned*) src0, (unsigned*)src1, (unsigned*) src2, (unsigned*) src3, bytes/sizeof(unsigned), opcode);
  }

}
#endif
