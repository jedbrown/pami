/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/ReduceFunctions.cc
 * \brief Support routines for allreduce operations
 */

#include "util/ccmi_util.h"  // need this first to define throw() before stdlib.h is included (for xlC -noeh)
#include "interface/ccmi_internal.h"
#include "util/ccmi_debug.h"
//#include "algorithms/executor/AllreduceBase.h"
#include "math/math_coremath.h"
//#include "./Factory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      ///
      ///  \brief Stub in a function for double complex that 
      /// will just call double float sum with count*2.
      /// 
      inline void double_complex_sum(double *dst, const double **srcs, 
                                     int nsrc, int count) 
      {
        Core_fp64_sum(dst, srcs, nsrc, count*2);
      }

      ///
      ///  \brief Stub in a function for single complex that 
      /// will just call float sum with count*2.
      /// 
      inline void single_complex_sum (float *dst, const float **srcs, 
                                      int nsrc, int count) 
      {
        Core_fp32_sum(dst, srcs, nsrc, count*2);
      }


///
/// \brief Get the reduce function and datatype size
///
      void getReduceFunction(CM_Dt           dtype,
                             CM_Op           op,
                             unsigned          count,
                             unsigned        & sizeOfType,
                             coremath        & func )
      {
        TRACE_ADAPTOR ((stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X)\n",
                        dtype, op));

        switch(op)
        {
        case CM_SUM:
          switch(dtype)
          {
          case CM_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (coremath) Core_int32_sum;
            break;
          case CM_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_sum;
            break;
          case CM_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_sum;
            break;
          case CM_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (coremath) Core_uint64_sum;
            break;
          case CM_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (coremath) Core_int16_sum;
            break;
          case CM_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_sum;
            break;
          case CM_DOUBLE:
            sizeOfType = sizeof(double);
            func = (coremath) Core_fp64_sum;
            break;
          case CM_FLOAT:
            sizeOfType = sizeof(float);
            func = (coremath) Core_fp32_sum;
            break;
          case CM_LONG_DOUBLE:
            sizeOfType = sizeof(long double);
            func = (coremath) Core_fp128_sum;
            break;
          case CM_DOUBLE_COMPLEX:
            sizeOfType = 2 * sizeof(double);
            func = (coremath) double_complex_sum;
            break;
          case CM_SINGLE_COMPLEX:
            sizeOfType = 2 * sizeof(float);
            func = (coremath) single_complex_sum;
            break;

          case CM_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (coremath) Core_int8_sum;
            break;
          case CM_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_sum;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CM_abort();
          }
          break;
        case CM_MAX:
                  switch(dtype)
                  {
                  case CM_SIGNED_INT:
                    sizeOfType = sizeof(int);
            func = (coremath) Core_int32_max;
                    break;
                  case CM_SIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_max;
                    break;
                  case CM_UNSIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_uint64_max;
                    break;
                  case CM_SIGNED_SHORT:
                    sizeOfType = sizeof(short);
            func = (coremath) Core_int16_max;
                    break;
                  case CM_UNSIGNED_INT:
                    sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_max;
                    break;
                  case CM_UNSIGNED_SHORT:
                    sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_max;
                    break;
                  case CM_FLOAT:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_fp32_max;
                    break;
                  case CM_DOUBLE:
                    sizeOfType = sizeof(double);
            func = (coremath) Core_fp64_max;
                    break;
                  case CM_LONG_DOUBLE:
                    sizeOfType = sizeof(long double);
            func = (coremath) Core_fp128_max;
                    break;
                  case CM_SIGNED_CHAR:
                    sizeOfType = sizeof(char);
            func = (coremath) Core_int8_max;
                    break;
                  case CM_UNSIGNED_CHAR:
                    sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_max;
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_MIN:
                  switch(dtype)
                  {
                  case CM_SIGNED_INT:
                    sizeOfType = sizeof(int);
            func = (coremath) Core_int32_min;
                    break;
                  case CM_SIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_min;
                    break;
                  case CM_UNSIGNED_LONG_LONG:
                    sizeOfType = sizeof(unsigned long long);
            func = (coremath) Core_uint64_min;
                    break;
                  case CM_SIGNED_SHORT:
                    sizeOfType = sizeof(short);
            func = (coremath) Core_int16_min;
                    break;
                  case CM_UNSIGNED_INT:
                    sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_min;
                    break;
                  case CM_UNSIGNED_SHORT:
                    sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_min;
                    break;
                  case CM_FLOAT:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_fp32_min;
                    break;
                  case CM_DOUBLE:
                    sizeOfType = sizeof(double);
            func = (coremath) Core_fp64_min;
                    break;
                  case CM_LONG_DOUBLE:
                    sizeOfType = sizeof(long double);
            func = (coremath) Core_fp128_min;
                    break;
                  case CM_UNSIGNED_CHAR:
                    sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_min;
                    break;
                  case CM_SIGNED_CHAR:
                    sizeOfType = sizeof(char);
            func = (coremath) Core_int8_min;
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_BAND:
                  switch(dtype)
                  {
                  case CM_FLOAT:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_uint32_band;
                    break;
                  case CM_LOGICAL:
                  case CM_UNSIGNED_INT:
                    sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_band;
                    break;
                  case CM_SIGNED_INT:
                    sizeOfType = sizeof(int);
            func = (coremath) Core_int32_band;
                    break;
                  case CM_DOUBLE:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_uint64_band;
                    break;
                  case CM_SIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_band;
                    break;
                  case CM_UNSIGNED_LONG_LONG:
                    sizeOfType = sizeof(unsigned long long);
            func = (coremath) Core_uint64_band;
                    break;
                  case CM_UNSIGNED_SHORT:
                    sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_band;
                    break;
                  case CM_SIGNED_SHORT:
                    sizeOfType = sizeof(short);
            func = (coremath) Core_int16_band;
                    break;
                  case CM_UNSIGNED_CHAR:
                    sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_band;
                    break;
                  case CM_SIGNED_CHAR:
                    sizeOfType = sizeof(char);
            func = (coremath) Core_int8_band;
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_BOR:
                  switch(dtype)
                  {
                  case CM_FLOAT:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_uint32_bor;
                    break;
                  case CM_LOGICAL:
                  case CM_UNSIGNED_INT:
                    sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_bor;
                    break;
                  case CM_SIGNED_INT:
                    sizeOfType = sizeof(int);
            func = (coremath) Core_int32_bor;
                    break;
                  case CM_DOUBLE:
                    sizeOfType = sizeof(double);
            func = (coremath) Core_uint64_bor;
                    break;
                  case CM_SIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_bor;
                    break;
                  case CM_UNSIGNED_LONG_LONG:
                    sizeOfType = sizeof(unsigned long long);
            func = (coremath) Core_uint64_bor;
                    break;
                  case CM_UNSIGNED_SHORT:
                    sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_bor;
                    break;
                  case CM_SIGNED_SHORT:
                    sizeOfType = sizeof(short);
            func = (coremath) Core_int16_bor;
                    break;
                  case CM_UNSIGNED_CHAR:
                    sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_bor;
                    break;
                  case CM_SIGNED_CHAR:
                    sizeOfType = sizeof(char);
            func = (coremath) Core_int8_bor;
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_BXOR:
                  switch(dtype)
                  {
                  case CM_FLOAT:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_uint32_bxor;
                    break;
                  case CM_LOGICAL:
                  case CM_UNSIGNED_INT:
                    sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_bxor;
                    break;
                  case CM_SIGNED_INT:
                    sizeOfType = sizeof(int);
            func = (coremath) Core_int32_bxor;
                    break;
                  case CM_DOUBLE:
                    sizeOfType = sizeof(double);
            func = (coremath) Core_uint64_bxor;
                    break;
                  case CM_SIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_bxor;
                    break;
                  case CM_UNSIGNED_LONG_LONG:
                    sizeOfType = sizeof(unsigned long long);
            func = (coremath) Core_uint64_bxor;
                    break;
                  case CM_UNSIGNED_SHORT:
                    sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_bxor;
                    break;
                  case CM_SIGNED_SHORT:
                    sizeOfType = sizeof(short);
            func = (coremath) Core_int16_bxor;
                    break;
                  case CM_UNSIGNED_CHAR:
                    sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_bxor;
                    break;
                  case CM_SIGNED_CHAR:
                    sizeOfType = sizeof(char);
            func = (coremath) Core_int8_bxor;
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_LAND:
                  switch(dtype)
                  {
                  case CM_FLOAT:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_uint32_land;
                    break;
                  case CM_LOGICAL:
                  case CM_UNSIGNED_INT:
                    sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_land;
                    break;
                  case CM_SIGNED_INT:
                    sizeOfType = sizeof(int);
            func = (coremath) Core_int32_land;
                    break;
                  case CM_DOUBLE:
                    sizeOfType = sizeof(double);
            func = (coremath) Core_uint64_land;
                    break;
                  case CM_SIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_land;
                    break;
                  case CM_UNSIGNED_LONG_LONG:
                    sizeOfType = sizeof(unsigned long long);
            func = (coremath) Core_uint64_land;
                    break;
                  case CM_UNSIGNED_SHORT:
                    sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_land;
                    break;
                  case CM_SIGNED_SHORT:
                    sizeOfType = sizeof(short);
            func = (coremath) Core_int16_land;
                    break;
                  case CM_UNSIGNED_CHAR:
                    sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_land;
                    break;
                  case CM_SIGNED_CHAR:
                    sizeOfType = sizeof(char);
            func = (coremath) Core_int8_land;
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_LOR:
                  switch(dtype)
                  {
                  case CM_FLOAT:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_uint32_lor;
                    break;
                  case CM_LOGICAL:
                  case CM_UNSIGNED_INT:
                    sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_lor;
                    break;
                  case CM_SIGNED_INT:
                    sizeOfType = sizeof(int);
            func = (coremath) Core_int32_lor;
                    break;
                  case CM_DOUBLE:
                    sizeOfType = sizeof(double);
            func = (coremath) Core_uint64_lor;
                    break;
                  case CM_SIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_lor;
                    break;
                  case CM_UNSIGNED_LONG_LONG:
                    sizeOfType = sizeof(unsigned long long);
            func = (coremath) Core_uint64_lor;
                    break;
                  case CM_UNSIGNED_SHORT:
                    sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_lor;
                    break;
                  case CM_SIGNED_SHORT:
                    sizeOfType = sizeof(short);
            func = (coremath) Core_int16_lor;
                    break;
                  case CM_UNSIGNED_CHAR:
                    sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_lor;
                    break;
                  case CM_SIGNED_CHAR:
                    sizeOfType = sizeof(char);
            func = (coremath) Core_int8_lor;
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_LXOR:
                  switch(dtype)
                  {
                  case CM_FLOAT:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_uint32_lxor;
                    break;
                  case CM_LOGICAL:
                  case CM_UNSIGNED_INT:
                    sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_lxor;
                    break;
                  case CM_SIGNED_INT:
                    sizeOfType = sizeof(int);
            func = (coremath) Core_int32_lxor;
                    break;
                  case CM_DOUBLE:
                    sizeOfType = sizeof(double);
            func = (coremath) Core_uint64_lxor;
                    break;
                  case CM_SIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_lxor;
                    break;
                  case CM_UNSIGNED_LONG_LONG:
                    sizeOfType = sizeof(unsigned long long);
            func = (coremath) Core_uint64_lxor;
                    break;
                  case CM_UNSIGNED_SHORT:
                    sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_lxor;
                    break;
                  case CM_SIGNED_SHORT:
                    sizeOfType = sizeof(short);
            func = (coremath) Core_int16_lxor;
                    break;
                  case CM_UNSIGNED_CHAR:
                    sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_lxor;
                    break;
                  case CM_SIGNED_CHAR:
                    sizeOfType = sizeof(char);
            func = (coremath) Core_int8_lxor;
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_MAXLOC:
                  switch(dtype)
                  {
                  case CM_LOC_2INT:
            func = (coremath) Core_int32_int32_maxloc;
                    sizeOfType = sizeof(int32_int32_t);
                    break;
                  case CM_LOC_SHORT_INT:
            func = (coremath) Core_int16_int32_maxloc;
                    sizeOfType = sizeof(int16_int32_t);
                    break;
                  case CM_LOC_FLOAT_INT:
            func = (coremath) Core_fp32_int32_maxloc;
                    sizeOfType = sizeof(fp32_int32_t);
                    break;
                  case CM_LOC_DOUBLE_INT:
            func = (coremath) Core_fp64_int32_maxloc;
                    sizeOfType = sizeof(fp64_int32_t);
                    break;
                  case CM_LOC_2FLOAT:
            func = (coremath) Core_fp32_fp32_maxloc;
                    sizeOfType = sizeof(fp32_fp32_t);
                    break;
                  case CM_LOC_2DOUBLE:
            func = (coremath) Core_fp64_fp64_maxloc;
                    sizeOfType = sizeof(fp64_fp64_t);
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_MINLOC:
                  switch(dtype)
                  {
                  case CM_LOC_2INT:
            func = (coremath) Core_int32_int32_minloc;
                    sizeOfType = sizeof(int32_int32_t);
                    break;
                  case CM_LOC_SHORT_INT:
            func = (coremath) Core_int16_int32_minloc;
                    sizeOfType = sizeof(int16_int32_t);
                    break;
                  case CM_LOC_FLOAT_INT:
            func = (coremath) Core_fp32_int32_minloc;
                    sizeOfType = sizeof(fp32_int32_t);
                    break;
                  case CM_LOC_DOUBLE_INT:
            func = (coremath) Core_fp64_int32_minloc;
                    sizeOfType = sizeof(fp64_int32_t);
                    break;
                  case CM_LOC_2FLOAT:
            func = (coremath) Core_fp32_fp32_minloc;
                    sizeOfType = sizeof(fp32_fp32_t);
                    break;
                  case CM_LOC_2DOUBLE:
            func = (coremath) Core_fp64_fp64_minloc;
                    sizeOfType = sizeof(fp64_fp64_t);
                    break;
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
                case CM_PROD:
                  switch(dtype)
                  {
                  case CM_SIGNED_INT:
                    sizeOfType = sizeof(int);
            func = (coremath) Core_int32_prod;
                    break;
                  case CM_UNSIGNED_INT:
                    sizeOfType = sizeof(unsigned int);
            func = (coremath) Core_uint32_prod;
                    break;
                  case CM_SIGNED_LONG_LONG:
                    sizeOfType = sizeof(long long);
            func = (coremath) Core_int64_prod;
                    break;
                  case CM_UNSIGNED_LONG_LONG:
                    sizeOfType = sizeof(unsigned long long);
            func = (coremath) Core_uint64_prod;
                    break;
                  case CM_SIGNED_SHORT:
                    sizeOfType = sizeof(short);
            func = (coremath) Core_int16_prod;
                    break;
                  case CM_UNSIGNED_SHORT:
                    sizeOfType = sizeof(unsigned short);
            func = (coremath) Core_uint16_prod;
                    break;
                  case CM_DOUBLE:
                    sizeOfType = sizeof(double);
            func = (coremath) Core_fp64_prod;
                    break;
                  case CM_FLOAT:
                    sizeOfType = sizeof(float);
            func = (coremath) Core_fp32_prod;
                    break;
                  case CM_LONG_DOUBLE:
                    sizeOfType = sizeof(long double);
            func = (coremath) Core_fp128_prod;
                    break;
                  case CM_SIGNED_CHAR:
                    sizeOfType = sizeof(char);
            func = (coremath) Core_int8_prod;
                    break;
                  case CM_UNSIGNED_CHAR:
                    sizeOfType = sizeof(unsigned char);
            func = (coremath) Core_uint8_prod;
                    break;
                  case CM_DOUBLE_COMPLEX: // Choose not to optimize this
                  default:
                    fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                            dtype, op, count);
                    CM_abort();
                  }
                  break;
        default:
          fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                  dtype, op, count);
          CM_abort();
        }
        TRACE_ADAPTOR ((stderr, "<          >CCMI::Adaptor::getReduceFunction()"
                        " count %#X, size %#X, function %#X)\n",
                        count, sizeOfType, (int) func));
      }
    }
  }
}
