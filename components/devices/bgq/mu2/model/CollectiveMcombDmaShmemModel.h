
#ifndef __collective_multicombine_dma_model__
#define __collective_multicombine_dma_model__

#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/CollectiveDmaModelBase.h"
#include "components/devices/bgq/mu2/model/MU_Collective_OP_DT_Table.h"
#include "components/devices/shmemcoll/ShmemCollDevice.h"
#include "components/devices/shmemcoll/ShmemCollDesc.h"

#include "sys/pami.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class CollectiveShmemMcombDmaModel: public CollectiveDmaModelBase, 
	public Interface::MulticombineModel < CollectiveShmemMcombDmaModel, MU::Context, CollectiveDmaModelBase::sizeof_msg > 
	{
	public:
	  static const uint32_t sizeof_msg = CollectiveDmaModelBase::sizeof_msg;
	  
	  CollectiveShmemMcombDmaModel (MU::Context                 & device, 
                                        ShmemCollDevice             & shmemcoll,                          
				       pami_result_t               & status) : 
	  CollectiveDmaModelBase(device, status),
	    Interface::MulticombineModel<CollectiveShmemMcombDmaModel, MU::Context, CollectiveDmaModelBase::sizeof_msg>  (device, status),
          _shmemcoll(shmemcoll)
	    { }
	    
	  /// \see PAMI::Device::Interface::MulticombineModel::postMulticombine
	  pami_result_t postMulticombine_impl(uint8_t (&state)[CollectiveShmemMcombDmaModel::sizeof_msg],
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

            Shmem::ShemCollDesc *my_desc = NULL, *master_desc = NULL; 
            pami_result_t res = _device.getShmemWorldDesc(&my_desc, &master_desc, 0);//(local_root=0)

            //assert(res == PAMI_SUCCESS);
            while (res != PAMI_SUCCESS)
            {
              res = _device.getShmemWorldDesc(&my_desc, &master_desc, 0);//(local_root = 0)
              _device.advance();
            }
                

	    PipeWorkQueue *spwq = (PipeWorkQueue *) mcombine->data;
	    PipeWorkQueue *dpwq = (PipeWorkQueue *) mcombine->results;

	    if (bytes <= CollectiveDmaModelBase::_collstate._tempSize) {	      
	      char *src = spwq->bufferToConsume();
	      uint32_t sbytes = spwq->bytesAvailableToConsume();	      
	      char *dst = dpwq->bufferToProduce();
	      
	      if (sbytes == bytes) {
		pami_result_t rc = CollectiveDmaModelBase::postShortCollective 
		  (op,
		   sizeoftype,
		   bytes,
		   src,
		   dst,
		   mcombine->cb_done.function,	       
		   mcombine->cb_done.clientdata);
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
								       sizeoftype
								       );
	    return rc;
	  }

        private:
          ShmemCollDevice   &_shmemcoll;

	};
    };
  };
};

#endif		  
