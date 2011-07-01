/**
 * \file algorithms/protocols/tspcoll/cau_collectives.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_cau_collectives_h__
#define __algorithms_protocols_tspcoll_cau_collectives_h__
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


#ifdef XLPGAS_PAMI_CAU
#include <lapi.h>
#include <pami.h>

namespace xlpgas{

  void cau_fast_allreduce(int64_t* dest, int64_t* src,
			  cau_reduce_op_t& op,
			  int ctxt);
  void cau_init();

  void reduce_op(int64_t *dst, int64_t *src, const cau_reduce_op_t& op);

  cau_reduce_op_t cau_op_dtype(xlpgas_ops_t      op,
			       xlpgas_dtypes_t   dtype);

  template <class T>
  void reduce_fixed_point(T &dst, T &src, unsigned func)
    {
    switch (func) {
    case CAU_NOP: break;
    case CAU_SUM: dst += src; break;
    case CAU_MIN: dst = (src < dst ? src : dst); break;
    case CAU_MAX: dst = (src > dst ? src : dst); break;
    case CAU_AND: dst &= src; break;
    case CAU_XOR: dst ^= src; break;
    case CAU_OR:  dst |= src; break;
    default: assert(!"Bogus fixed-point reduce function");
    }
   }

  template <class T>
   void reduce_floating_point(T &dst, T &src, unsigned func)
    {
      switch (func) {
      case CAU_NOP: break;
      case CAU_SUM: dst += src; break;
      case CAU_MIN: dst = (src < dst ? src : dst); break;
      case CAU_MAX: dst = (src > dst ? src : dst); break;
      default: assert(!"Bogus floating-point reduce function");
      }
    }
  
}//end namespace
#endif // XLPGAS_PAMI_CAU

#endif
