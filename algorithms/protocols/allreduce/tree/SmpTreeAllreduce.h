/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/tree/SmpTreeAllreduce.h
 * \brief ???
 */

#ifndef __ccmi_adaptor_allreduce_shm_tree_h__
#define __ccmi_adaptor_allreduce_shm_tree_h__

#include "../BaseComposite.h"
#include "multisend/multisend_impl.h"

extern int dcmf_dt_shift[CM_DT_COUNT];

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      namespace Tree
      {
        ///
        /// \brief Tree allreduce protocol for SMP mode
        ///
        /// 
        class SmpTreeAllreduce : public CCMI::Adaptor::Allreduce::BaseComposite
        {
        public:

          SmpTreeAllreduce (CCMI::MultiSend::MulticombineInterface * mf,
                            CCMI::Adaptor::CollectiveProtocolFactory*          factory,
                            unsigned                                 rank
                           ) :
          BaseComposite( factory ),
          _mcomb       ((DCMF::Collectives::MultiSend::MulticombineImpl *)mf),
          _dt          (CM_UNDEFINED_DT),
          _count       (0),
          _bytes       (0),
          _sizeOfType  (0),
          _rank        (rank)
          {
            _mcombArgs.setCallback( NULL, NULL );
            _mcombArgs.setDataRanks( NULL ); // ???
            _mcombArgs.setReduceInfo( CM_UNDEFINED_OP, CM_UNDEFINED_DT );
          }

          virtual unsigned restart   ( CM_CollectiveRequest_t  * request,
                                       CM_Callback_t           & cb_done,
                                       CCMI_Consistency            consistency,
                                       char                      * srcbuf,
                                       char                      * dstbuf,
                                       size_t                      count,
                                       CM_Dt                     dtype,
                                       CM_Op                     op,
                                       size_t                      root = (size_t)-1);

        private:    
          inline void reset( CM_Dt dtype, unsigned count ) __attribute__((noinline));

          //16 bytes of class vars
          DCMF::Collectives::MultiSend::MulticombineImpl *_mcomb;
          CM_Dt                        _dt;
          unsigned                       _count;
          unsigned                       _bytes;

          //Align structure
          CCMI::MultiSend::CCMI_Multicombine_t      _mcombArgs __attribute__((__aligned__(16)));

          unsigned                       _sizeOfType;

          /// The rank to compare the root with
          unsigned                       _rank;
        }; /* SmpTreeAllreduce */
      }
    }
  }
} 


inline void CCMI::Adaptor::Allreduce::Tree::SmpTreeAllreduce::reset( CM_Dt dtype, unsigned count )
{
  _dt = dtype;
  _bytes = count << dcmf_dt_shift[dtype];
  _count = count;
#if 0
  int size;
  switch(dtype)
  {
  case CM_LOGICAL:
  case CM_SIGNED_INT:
  case CM_UNSIGNED_INT:
    size = sizeof(int);
    break;
  case CM_SIGNED_LONG_LONG:
  case CM_UNSIGNED_LONG_LONG:
    size = sizeof(long long);
    break;
  case CM_SIGNED_SHORT:
  case CM_UNSIGNED_SHORT:
    size = sizeof(short);
    break;
  case CM_UNSIGNED_CHAR:
  case CM_SIGNED_CHAR:
    size = sizeof(char);
    break;
  case CM_FLOAT:
    size = sizeof(float);
    break;
  case CM_DOUBLE:
    size = sizeof(double);
    break;
  case CM_LOC_2INT:
    size = sizeof(int32_int32_t);
    break;
  case CM_LOC_SHORT_INT:
    size = sizeof(int16_int32_t);
    break;
  case CM_LOC_FLOAT_INT:
    size = sizeof(fp32_int32_t);
    break;
  case CM_LOC_DOUBLE_INT:
    size = sizeof(fp64_int32_t);
    break;
  case CM_LOC_2FLOAT:
    size = sizeof(fp32_fp32_t);
    break;
  case CM_LOC_2DOUBLE:
    size = sizeof(fp64_fp64_t);
    break;
  default:
    CCMI_abort();
  }

  _sizeOfType = size;                //sizeOfType( dtype );
  _dt         = dtype;

  _bytes  = count * _sizeOfType;
  _count  = count;
#endif
}


#endif
