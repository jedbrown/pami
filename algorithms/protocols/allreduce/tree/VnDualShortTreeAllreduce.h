/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/tree/VnDualShortTreeAllreduce.h
 * \brief ???
 */

#ifndef __algorithms_protocols_allreduce_tree_VnDualShortTreeAllreduce_h__
#define __algorithms_protocols_allreduce_tree_VnDualShortTreeAllreduce_h__

#include "../BaseComposite.h"
#include "multisend/multisend_impl.h"
#include "interface/Topology.h"
#include "PipeWorkQueue.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      namespace Tree
      {
        ///
        /// \brief Tree allreduce protocol for short msg in VN mode
        ///
        ///
        class VnDualShortTreeAllreduce : public CCMI::Adaptor::Allreduce::BaseComposite
        {

          static const unsigned MaxDataBytes = 4096;

          typedef struct _SharedData
          {
            char dst[MaxDataBytes];
            void * srcs[ MAX_NUM_CORES ];
            char pad[32];

            struct
            {
              volatile unsigned isSrcReady;
              volatile unsigned isDstReady;
              char src[MaxDataBytes];
              char pad[32];
            } client[MAX_NUM_CORES - 1];

          } SharedData __attribute__((__aligned__(32)));

          typedef enum
          {
            OPT_FUN, DBL_SUM, INT_SUM
          } ReduceOpSelect;

        public:

          VnDualShortTreeAllreduce(CCMI::MultiSend::MulticombineInterface * mf,
                                   CCMI::Adaptor::CollectiveProtocolFactory* factory,
                                   CCMI::TorusCollectiveMapping *mapping) :
          BaseComposite(factory),
          _mapping(mapping),
          _mcomb((DCMF::Collectives::MultiSend::MulticombineImpl *)mf),
          _dstbuf(NULL),
          _op(XMI_UNDEFINED_OP),
          _dt(XMI_UNDEFINED_DT),
          _sizeOfType(0),
          _count(0),
          _bytes(0),
          _reduceFunc(NULL)
          {
            CCMI_assert(_shared != NULL);

            //_mcombArgs.setConsistency(CCMI_MATCH_CONSISTENCY);
            _mcombArgs.setCallback(cb_treeRecvDone, this);
            _mcombArgs.setRoles((unsigned)-1); // perform all roles

          }


          static void init(CCMI::TorusCollectiveMapping *mapping);

          static void cb_treeRecvDone(void *me, XMI_Error_t *err)
          {
            VnDualShortTreeAllreduce *a = (VnDualShortTreeAllreduce *)me;

            asm volatile ("mbar" ::: "memory");

            // notify peers that result is ready
            _shared->client[0].isDstReady = 1;
            _shared->client[1].isDstReady = 1;
            _shared->client[2].isDstReady = 1;

            // local completion
            a->done();
          }

          void done()
          {

            TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Tree::VnDualShortTreeAllreduce::done "
                           "dst[0]: %d\n", (int)this,
                           ((int*)_shared->dst)[0]));

            // copy result into user dstbuf ...
            if (_dstbuf != NULL) // ... if we wanted the results
            {
              memcpy(_dstbuf, _rwq_buf, _bytes);
            }
	    // last one must "consume" the bytes...

            // completion callback
            if (_cb_done.function)
            {
              (*_cb_done.function)(_cb_done.clientdata, NULL);
            }
          }

          virtual unsigned restart(XMI_CollectiveRequest_t  *request,
                                       XMI_Callback_t           & cb_done,
                                       CCMI_Consistency            consistency,
                                       char                      * srcbuf,
                                       char                      * dstbuf,
                                       size_t                      count,
                                       XMI_Dt                     dtype,
                                       XMI_Op                     op,
                                       size_t                      root = (size_t)-1);

        private:
#if 0
          int sizeOfType( XMI_Dt dtype )
          {
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
            return size;
          }
#endif

          CCMI::TorusCollectiveMapping                 * _mapping;
          DCMF::Collectives::MultiSend::MulticombineImpl     * _mcomb;
          XMI_Callback_t                _cb_done;
          char                         * _dstbuf;
          XMI_Op                        _op;
          XMI_Dt                        _dt;
          unsigned                       _sizeOfType;
          unsigned                       _count;
          unsigned                       _bytes;
          coremath                _reduceFunc;
          ReduceOpSelect                 _reduceOpSelect;

          static CCMI::MultiSend::CCMI_Multicombine_t _mcombArgs __attribute__ ((__aligned__ (16)));
          static SharedData            * _shared;
	  static char *_swq_buf;
	  static char *_rwq_buf;
	  static XMI::PipeWorkQueue _swq;
	  static XMI::PipeWorkQueue _rwq;
	  static XMI::Topology _root;
          static unsigned _numPeers;
          static unsigned _myPeer;
          static unsigned _isMasterCore;
        }; /* VnDualShortTreeAllreduce */
      }
    }
  }
}

#endif // __ccmi_adaptor_allreduce_vn_dual_short_tree_h__
