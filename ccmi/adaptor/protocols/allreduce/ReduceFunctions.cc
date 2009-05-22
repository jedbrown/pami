/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file collectives/bgp/protocols/allreduce/ReduceFunctions.cc
 * \brief Support routines for allreduce operations
 */

#include "../../ccmi_util.h"  // need this first to define throw() before stdlib.h is included (for xlC -noeh)
#include "../../ccmi_internal.h"
#include "../../../executor/AllreduceBase.h"
#include "../../../../math/math_coremath.h"
//#include "protocols/allreduce/Factory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      const unsigned MAX_NSRC = 2;
      template<typename TYPE> void sum(TYPE d, TYPE l, TYPE r) { d = l + r ;}
      template<typename TYPE> void prod(TYPE d, TYPE l, TYPE r) { d = l * r ;}

      template<typename TYPE, void OP(TYPE, TYPE, TYPE)> void math(TYPE *dst, const TYPE **srcs, int nsrc, int count) {
        CCMI_assert_debug(nsrc <= MAX_NSRC);
        CCMI_assert_debug(nsrc >= 2);
        register int n = 0, m;
        TYPE buf0[MAX_NSRC];
        register TYPE rbuffer0;
        register TYPE rbuffer1, rbuffer2, rbuffer3;
        TYPE buf1[MAX_NSRC];
        TYPE buf2[MAX_NSRC];
        TYPE buf3[MAX_NSRC];
        for(; n < count - 3; n += 4)
        {
          for(m = 0; m < nsrc; ++m)
          {
            buf0[m] = srcs[m][n+0];
            buf1[m] = srcs[m][n+1];
            buf2[m] = srcs[m][n+2];
            buf3[m] = srcs[m][n+3];
          }

           OP(rbuffer0,buf0[0],buf0[1]);
           OP(rbuffer1,buf1[0],buf1[1]);
           OP(rbuffer2,buf2[0],buf2[1]);
           OP(rbuffer3,buf3[0],buf3[1]);
          for(m = 2; m < nsrc; ++m)
          {
            OP(rbuffer0,rbuffer0,buf0[m]);
            OP(rbuffer1,rbuffer1,buf1[m]);
            OP(rbuffer2,rbuffer2,buf2[m]);
            OP(rbuffer3,rbuffer3,buf3[m]);
          }

          dst[n+0] = rbuffer0;
          dst[n+1] = rbuffer1;
          dst[n+2] = rbuffer2;
          dst[n+3] = rbuffer3;
        }
        for(; n < count; n++)
        {
          for(m = 0; m < nsrc; ++m)
          {
            buf0[m] = srcs[m][n];
          }
          OP(rbuffer0,buf0[0],buf0[1]);
          for(m = 2; m < nsrc; ++m)
          {
            OP(rbuffer0,rbuffer0,buf0[m]);
          }
          dst[n] = rbuffer0;
        }
      }
      inline void int_sum(int32_t *dst, const int32_t **srcs, int nsrc, int count) 
      {
        int a;
        math<int,sum<int> >(dst, srcs, nsrc, count);
      }
      ///
      ///  \brief Stub in a function for double complex that 
      /// will just call double float sum with count*2.
      /// 
      inline void double_complex_sum(double *dst, const double **srcs, 
                                     int nsrc, int count) 
      {
        math<double,sum<double> >(dst, srcs, nsrc, count*2);
      }

      ///
      ///  \brief Stub in a function for single complex that 
      /// will just call float sum with count*2.
      /// 
      inline void single_complex_sum (float *dst, const float **srcs, 
                                      int nsrc, int count) 
      {
        math<float,sum<float> >(dst, srcs, nsrc, count*2);
      }


///
/// \brief Get the reduce function and datatype size
///
      void getReduceFunction(CCMI_Dt           dtype,
                             CCMI_Op           op,
                             unsigned          count,
                             unsigned        & sizeOfType,
                             CCMI_ReduceFunc & func )
      {
        TRACE_ADAPTOR ((stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X)\n",
                        dtype, op));

        switch(op)
        {
        case CCMI_SUM:
          switch(dtype)
          {
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) int_sum;
            break;
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) int_sum;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_sum;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_uint64_sum;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_sum;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_sum;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(double);
            func = (CCMI_ReduceFunc) Core_fp64_sum;
            break;
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_fp32_sum;
            break;
          case CCMI_LONG_DOUBLE:
            sizeOfType = sizeof(long double);
            func = (CCMI_ReduceFunc) Core_fp128_sum;
            break;
          case CCMI_DOUBLE_COMPLEX:
            sizeOfType = 2 * sizeof(double);
            func = (CCMI_ReduceFunc) double_complex_sum;
            break;
          case CCMI_SINGLE_COMPLEX:
            sizeOfType = 2 * sizeof(float);
            func = (CCMI_ReduceFunc) single_complex_sum;
            break;

          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_sum;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_sum;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_MAX:
          switch(dtype)
          {
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) Core_int32_max;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_max;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_uint64_max;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_max;
            break;
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) Core_uint32_max;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_max;
            break;
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_fp32_max;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(double);
            func = (CCMI_ReduceFunc) Core_fp64_max;
            break;
          case CCMI_LONG_DOUBLE:
            sizeOfType = sizeof(long double);
            func = (CCMI_ReduceFunc) Core_fp128_max;
            break;
          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_max;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_max;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_MIN:
          switch(dtype)
          {
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) Core_int32_min;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_min;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(unsigned long long);
            func = (CCMI_ReduceFunc) Core_uint64_min;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_min;
            break;
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) Core_uint32_min;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_min;
            break;
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_fp32_min;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(double);
            func = (CCMI_ReduceFunc) Core_fp64_min;
            break;
          case CCMI_LONG_DOUBLE:
            sizeOfType = sizeof(long double);
            func = (CCMI_ReduceFunc) Core_fp128_min;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_min;
            break;
          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_min;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_BAND:
          switch(dtype)
          {
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_uint32_band;
            break;
          case CCMI_LOGICAL:
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) Core_uint32_band;
            break;
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) Core_int32_band;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_uint64_band;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_band;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(unsigned long long);
            func = (CCMI_ReduceFunc) Core_uint64_band;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_band;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_band;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_band;
            break;
          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_band;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_BOR:
          switch(dtype)
          {
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_uint32_bor;
            break;
          case CCMI_LOGICAL:
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) Core_uint32_bor;
            break;
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) Core_int32_bor;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(double);
            func = (CCMI_ReduceFunc) Core_uint64_bor;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_bor;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(unsigned long long);
            func = (CCMI_ReduceFunc) Core_uint64_bor;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_bor;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_bor;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_bor;
            break;
          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_bor;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_BXOR:
          switch(dtype)
          {
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_uint32_bxor;
            break;
          case CCMI_LOGICAL:
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) Core_uint32_bxor;
            break;
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) Core_int32_bxor;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(double);
            func = (CCMI_ReduceFunc) Core_uint64_bxor;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_bxor;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(unsigned long long);
            func = (CCMI_ReduceFunc) Core_uint64_bxor;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_bxor;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_bxor;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_bxor;
            break;
          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_bxor;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_LAND:
          switch(dtype)
          {
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_uint32_land;
            break;
          case CCMI_LOGICAL:
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) Core_uint32_land;
            break;
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) Core_int32_land;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(double);
            func = (CCMI_ReduceFunc) Core_uint64_land;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_land;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(unsigned long long);
            func = (CCMI_ReduceFunc) Core_uint64_land;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_land;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_land;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_land;
            break;
          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_land;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_LOR:
          switch(dtype)
          {
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_uint32_lor;
            break;
          case CCMI_LOGICAL:
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) Core_uint32_lor;
            break;
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) Core_int32_lor;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(double);
            func = (CCMI_ReduceFunc) Core_uint64_lor;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_lor;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(unsigned long long);
            func = (CCMI_ReduceFunc) Core_uint64_lor;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_lor;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_lor;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_lor;
            break;
          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_lor;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_LXOR:
          switch(dtype)
          {
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_uint32_lxor;
            break;
          case CCMI_LOGICAL:
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) Core_uint32_lxor;
            break;
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) Core_int32_lxor;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(double);
            func = (CCMI_ReduceFunc) Core_uint64_lxor;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_lxor;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(unsigned long long);
            func = (CCMI_ReduceFunc) Core_uint64_lxor;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_lxor;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_lxor;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_lxor;
            break;
          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_lxor;
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_MAXLOC:
          switch(dtype)
          {
          case CCMI_LOC_2INT:
            func = (CCMI_ReduceFunc) Core_int32_int32_maxloc;
            sizeOfType = sizeof(int32_int32_t);
            break;
          case CCMI_LOC_SHORT_INT:
            func = (CCMI_ReduceFunc) Core_int16_int32_maxloc;
            sizeOfType = sizeof(int16_int32_t);
            break;
          case CCMI_LOC_FLOAT_INT:
            func = (CCMI_ReduceFunc) Core_fp32_int32_maxloc;
            sizeOfType = sizeof(fp32_int32_t);
            break;
          case CCMI_LOC_DOUBLE_INT:
            func = (CCMI_ReduceFunc) Core_fp64_int32_maxloc;
            sizeOfType = sizeof(fp64_int32_t);
            break;
          case CCMI_LOC_2FLOAT:
            func = (CCMI_ReduceFunc) Core_fp32_fp32_maxloc;
            sizeOfType = sizeof(fp32_fp32_t);
            break;
          case CCMI_LOC_2DOUBLE:
            func = (CCMI_ReduceFunc) Core_fp64_fp64_maxloc;
            sizeOfType = sizeof(fp64_fp64_t);
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_MINLOC:
          switch(dtype)
          {
          case CCMI_LOC_2INT:
            func = (CCMI_ReduceFunc) Core_int32_int32_minloc;
            sizeOfType = sizeof(int32_int32_t);
            break;
          case CCMI_LOC_SHORT_INT:
            func = (CCMI_ReduceFunc) Core_int16_int32_minloc;
            sizeOfType = sizeof(int16_int32_t);
            break;
          case CCMI_LOC_FLOAT_INT:
            func = (CCMI_ReduceFunc) Core_fp32_int32_minloc;
            sizeOfType = sizeof(fp32_int32_t);
            break;
          case CCMI_LOC_DOUBLE_INT:
            func = (CCMI_ReduceFunc) Core_fp64_int32_minloc;
            sizeOfType = sizeof(fp64_int32_t);
            break;
          case CCMI_LOC_2FLOAT:
            func = (CCMI_ReduceFunc) Core_fp32_fp32_minloc;
            sizeOfType = sizeof(fp32_fp32_t);
            break;
          case CCMI_LOC_2DOUBLE:
            func = (CCMI_ReduceFunc) Core_fp64_fp64_minloc;
            sizeOfType = sizeof(fp64_fp64_t);
            break;
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        case CCMI_PROD:
          switch(dtype)
          {
          case CCMI_SIGNED_INT:
            sizeOfType = sizeof(int);
            func = (CCMI_ReduceFunc) Core_int32_prod;
            break;
          case CCMI_UNSIGNED_INT:
            sizeOfType = sizeof(unsigned int);
            func = (CCMI_ReduceFunc) Core_uint32_prod;
            break;
          case CCMI_SIGNED_LONG_LONG:
            sizeOfType = sizeof(long long);
            func = (CCMI_ReduceFunc) Core_int64_prod;
            break;
          case CCMI_UNSIGNED_LONG_LONG:
            sizeOfType = sizeof(unsigned long long);
            func = (CCMI_ReduceFunc) Core_uint64_prod;
            break;
          case CCMI_SIGNED_SHORT:
            sizeOfType = sizeof(short);
            func = (CCMI_ReduceFunc) Core_int16_prod;
            break;
          case CCMI_UNSIGNED_SHORT:
            sizeOfType = sizeof(unsigned short);
            func = (CCMI_ReduceFunc) Core_uint16_prod;
            break;
          case CCMI_DOUBLE:
            sizeOfType = sizeof(double);
            func = (CCMI_ReduceFunc) Core_fp64_prod;
            break;
          case CCMI_FLOAT:
            sizeOfType = sizeof(float);
            func = (CCMI_ReduceFunc) Core_fp32_prod;
            break;
          case CCMI_LONG_DOUBLE:
            sizeOfType = sizeof(long double);
            func = (CCMI_ReduceFunc) Core_fp128_prod;
            break;
          case CCMI_SIGNED_CHAR:
            sizeOfType = sizeof(char);
            func = (CCMI_ReduceFunc) Core_int8_prod;
            break;
          case CCMI_UNSIGNED_CHAR:
            sizeOfType = sizeof(unsigned char);
            func = (CCMI_ReduceFunc) Core_uint8_prod;
            break;
          case CCMI_DOUBLE_COMPLEX: // Choose not to optimize this
          default:
            fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                    dtype, op, count);
            CCMI_abort();
          }
          break;
        default:
          fprintf(stderr, "<          >CCMI::Adaptor::getReduceFunction(dtype %#X,op %#X, count %#X)\n",
                  dtype, op, count);
          CCMI_abort();
        }
        TRACE_ADAPTOR ((stderr, "<          >CCMI::Adaptor::getReduceFunction()"
                        " count %#X, size %#X, function %#X)\n",
                        count, sizeOfType, (int) func));
      }
    }
  }
}
