/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/tree/SmpTreeAllreduce.cc
 * \brief ???
 */
#ifndef __bgp__
#warning DCMF specific protocol - needs work to make it generic
#endif
#include "math/math_coremath.h"
#include "SmpTreeAllreduce.h"
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

          unsigned SmpTreeAllreduce::restart(XMI_CollectiveRequest_t  *request,
                                       XMI_Callback_t           & cb_done,
                                       CCMI_Consistency            consistency,
                                       char                      * srcbuf,
                                       char                      * dstbuf,
                                       size_t                      count,
                                       XMI_Dt                     dtype,
                                       XMI_Op                     op,
                                       size_t                      root) 
          {
	    struct _req {
		XMI_Request_t _msg;
		LL::Topology _root;
		LL::PipeWorkQueue _swq;
		LL::PipeWorkQueue _rwq;
	    } *req = (struct _req *)request;
            // call tree multisend directly	      
            //TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Tree::SmpTreeAllreduce::restart\n", (int)this));

            if((_dt != dtype) || (_count != count))
            {
              reset (dtype, count);
            }

            //_mcombArgs.setConsistency( consistency );
	    req->_swq.configure(NULL, srcbuf, _bytes, _bytes);
	    req->_swq.reset();
	    req->_rwq.configure(NULL, dstbuf, _bytes, 0);
	    req->_rwq.reset();
	    if (root != (size_t)-1) {
	    	new (&req->_root) LL::Topology(root);
	    } else {
	    	new (&req->_root) LL::Topology(_rank);
	    }

            _mcombArgs.setRequestBuffer(&req->_msg, sizeof(req->_msg));
            _mcombArgs.setRoles((unsigned)-1); // perform all roles
            _mcombArgs.setData((XMI_PipeWorkQueue_t *)&req->_swq, _count);
            _mcombArgs.setDataRanks(NULL);
            _mcombArgs.setResults((XMI_PipeWorkQueue_t *)&req->_rwq, _count);
            _mcombArgs.setResultsRanks((XMI_Topology_t *)&req->_root);
            _mcombArgs.setReduceInfo(op, dtype);
            _mcombArgs.setCallback(cb_done.function, cb_done.clientdata);

            ///Inlining the send implementation
            _mcomb->DCMF::Collectives::MultiSend::MulticombineImpl::generate(&_mcombArgs);

            return XMI_SUCCESS;
          }   

      }
    }
  }
} 
