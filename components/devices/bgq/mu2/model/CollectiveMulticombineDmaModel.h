///
/// \file components/devices/bgq/mu2/model/CollectiveMulticombineDmaModel.h
/// \brief ???
///
#ifndef __components_devices_bgq_mu2_model_CollectiveMulticombineDmaModel_h__
#define __components_devices_bgq_mu2_model_CollectiveMulticombineDmaModel_h__

#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/CollectiveDmaModelBase.h"
#include "components/devices/bgq/mu2/model/MU_Collective_OP_DT_Table.h"
#include "sys/pami.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class CollectiveMulticombineDmaModel: public CollectiveDmaModelBase, 
	public Interface::MulticombineModel < CollectiveMulticombineDmaModel, MU::Context, CollectiveDmaModelBase::sizeof_msg > 
	{
	public:
	  static const uint32_t sizeof_msg = CollectiveDmaModelBase::sizeof_msg;
	  
	  CollectiveMulticombineDmaModel (MU::Context                 & device, 
				       pami_result_t               & status) : 
	  CollectiveDmaModelBase(device, status),
	    Interface::MulticombineModel<CollectiveMulticombineDmaModel, MU::Context, CollectiveDmaModelBase::sizeof_msg>  (device, status)
	    {

	    }
	    
	  /// \see PAMI::Device::Interface::MulticombineModel::postMulticombine
	  pami_result_t postMulticombine_impl(uint8_t (&state)[CollectiveMulticombineDmaModel::sizeof_msg],
					   pami_multicombine_t *mcombine,
					   void             *devinfo = NULL) 
	  {
	    //TRACE_FN_ENTER();
	    // Get the source data buffer/length and validate (assert) inputs
	    unsigned sizeoftype =  mu_collective_size_table[mcombine->dtype];
	    unsigned bytes      =  mcombine->count * sizeoftype;
	    unsigned op = mu_collective_op_table[mcombine->dtype][mcombine->optor];

	    if (op == unsupported_operation)
	      return PAMI_ERROR; //Unsupported operation

	    unsigned classroute = 0;
	    if (devinfo) 
	      classroute = ((uint32_t)(uint64_t)devinfo) - 1;

	    PipeWorkQueue *spwq = (PipeWorkQueue *) mcombine->data;
	    PipeWorkQueue *dpwq = (PipeWorkQueue *) mcombine->results;	    
	    if (bytes <= CollectiveDmaModelBase::_collstate._tempSize) {	      
	      char *src = spwq->bufferToConsume();
	      uint32_t sbytes = spwq->bytesAvailableToConsume();	      
	      
	      if (sbytes == bytes) {
		pami_result_t rc = CollectiveDmaModelBase::postShortCollective 
		  (op,
		   sizeoftype,
		   bytes,
		   src,
		   dpwq,
		   mcombine->cb_done.function,	       
		   mcombine->cb_done.clientdata,
		   classroute);
		if (rc == PAMI_SUCCESS)
		  return PAMI_SUCCESS;
	      }
	    }
	    
	    pami_result_t rc = CollectiveDmaModelBase::postCollective (state,
								       bytes,
								       spwq,
								       dpwq, 
								       mcombine->cb_done.function,	       
								       mcombine->cb_done.clientdata,
								       op,
								       sizeoftype,
								       classroute);
	    return rc;
	  }
	};
    };
  };
};

#endif		  