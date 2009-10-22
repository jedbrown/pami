/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/tree/SmpTreeAllreduce.h
 * \brief ???
 */

#ifndef __algorithms_protocols_allreduce_tree_SmpTreeAllreduce_h__
#define __algorithms_protocols_allreduce_tree_SmpTreeAllreduce_h__

#include "../BaseComposite.h"
#include "multisend/multisend_impl.h"

extern int dcmf_dt_shift[XMI_DT_COUNT];

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
          _dt          (XMI_UNDEFINED_DT),
          _count       (0),
          _bytes       (0),
          _sizeOfType  (0),
          _rank        (rank)
          {
            _mcombArgs.setCallback( NULL, NULL );
            _mcombArgs.setDataRanks( NULL ); // ???
            _mcombArgs.setReduceInfo( XMI_UNDEFINED_OP, XMI_UNDEFINED_DT );
          }

          virtual unsigned restart   ( XMI_CollectiveRequest_t  * request,
                                       XMI_Callback_t           & cb_done,
                                       CCMI_Consistency            consistency,
                                       char                      * srcbuf,
                                       char                      * dstbuf,
                                       size_t                      count,
                                       XMI_Dt                     dtype,
                                       XMI_Op                     op,
                                       size_t                      root = (size_t)-1);

        private:
          inline void reset( XMI_Dt dtype, unsigned count ) __attribute__((noinline));

          //16 bytes of class vars
          DCMF::Collectives::MultiSend::MulticombineImpl *_mcomb;
          XMI_Dt                        _dt;
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


inline void CCMI::Adaptor::Allreduce::Tree::SmpTreeAllreduce::reset( XMI_Dt dtype, unsigned count )
{
  _dt = dtype;
  _bytes = count << dcmf_dt_shift[dtype];
  _count = count;
#if 0
  int size;
  switch(dtype)
  {
  case XMI_LOGICAL:
  case XMI_SIGNED_INT:
  case XMI_UNSIGNED_INT:
    size = sizeof(int);
    break;
  case XMI_SIGNED_LONG_LONG:
  case XMI_UNSIGNED_LONG_LONG:
    size = sizeof(long long);
    break;
  case XMI_SIGNED_SHORT:
  case XMI_UNSIGNED_SHORT:
    size = sizeof(short);
    break;
  case XMI_UNSIGNED_CHAR:
  case XMI_SIGNED_CHAR:
    size = sizeof(char);
    break;
  case XMI_FLOAT:
    size = sizeof(float);
    break;
  case XMI_DOUBLE:
    size = sizeof(double);
    break;
  case XMI_LOC_2INT:
    size = sizeof(int32_int32_t);
    break;
  case XMI_LOC_SHORT_INT:
    size = sizeof(int16_int32_t);
    break;
  case XMI_LOC_FLOAT_INT:
    size = sizeof(fp32_int32_t);
    break;
  case XMI_LOC_DOUBLE_INT:
    size = sizeof(fp64_int32_t);
    break;
  case XMI_LOC_2FLOAT:
    size = sizeof(fp32_fp32_t);
    break;
  case XMI_LOC_2DOUBLE:
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
