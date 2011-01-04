///
/// \file components/devices/bgq/mu2/model/CollectiveMulticastDmaModel.h
/// \brief ???
///
#ifndef __components_devices_bgq_mu2_model_CollectiveMulticastDmaModel_h__
#define __components_devices_bgq_mu2_model_CollectiveMulticastDmaModel_h__

#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/CollectiveDmaModelBase.h"
#include "sys/pami.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
#define ZERO_BYTES 65536

      class CollectiveMulticastDmaModel: public CollectiveDmaModelBase, 
	public Interface::MulticastModel < CollectiveMulticastDmaModel, MU::Context, CollectiveDmaModelBase::sizeof_msg > 
	{
	  static char        * _zeroBuf;
	  static uint32_t      _zeroBytes;

	public:
	  static const uint32_t sizeof_msg = CollectiveDmaModelBase::sizeof_msg;
	  
	  CollectiveMulticastDmaModel (MU::Context                 & device, 
				       pami_result_t               & status) : 
	  CollectiveDmaModelBase(device, status),
	    Interface::MulticastModel<CollectiveMulticastDmaModel, MU::Context, CollectiveDmaModelBase::sizeof_msg >  (device, status)
	    {
	      _zeroBytes = ZERO_BYTES;
	      if (_zeroBuf == NULL) {
		_zeroBuf = (char *) malloc (_zeroBytes * sizeof(char));
		memset (_zeroBuf, 0, _zeroBytes);
	      }
	    }
	    
	  /// \see PAMI::Device::Interface::MulticastModel::postMulticast
	  pami_result_t postMulticast_impl(uint8_t (&state)[CollectiveMulticastDmaModel::sizeof_msg],
					   pami_multicast_t *mcast,
					   void             *devinfo = NULL) 
	  {
	    //TRACE_FN_ENTER();
	    // Get the source data buffer/length and validate (assert) inputs	    
	    pami_task_t *ranks = NULL;
	    ((Topology *)mcast->src_participants)->rankList(&ranks);
	    bool isroot = (ranks[0] == __global.mapping.task());

	    unsigned classroute = 0;
	    if (devinfo) 
	      classroute = ((uint32_t)(uint64_t)devinfo) - 1;

	    if (mcast->bytes <= CollectiveDmaModelBase::_collstate._tempSize) {	      
	      if (isroot) {
		PipeWorkQueue *spwq = (PipeWorkQueue *) mcast->src;
		char *src = spwq->bufferToConsume();
		uint32_t sbytes = spwq->bytesAvailableToConsume();
		
		if (sbytes == mcast->bytes) {
		  pami_result_t rc = CollectiveDmaModelBase::postShortCollective (MUHWI_COLLECTIVE_OP_CODE_OR,
										  4,
										  mcast->bytes,
										  src,
										  NULL,
										  mcast->cb_done.function,	       
										  mcast->cb_done.clientdata,
										  classroute);
		  if (rc == PAMI_SUCCESS)
		    return PAMI_SUCCESS;
		}
	      }
	      else {
		pami_result_t rc = CollectiveDmaModelBase::postShortCollective (MUHWI_COLLECTIVE_OP_CODE_OR,
										4,
										mcast->bytes,
										_zeroBuf,
										(PipeWorkQueue*) mcast->dst,
										mcast->cb_done.function,	       
										mcast->cb_done.clientdata,
										classroute);
		if (rc == PAMI_SUCCESS)
		  return PAMI_SUCCESS;
	      }
	    }

	    pami_result_t rc = CollectiveDmaModelBase::postBroadcast (state,
								      mcast->bytes,
								      (PipeWorkQueue *) mcast->src,
								      (PipeWorkQueue *) mcast->dst, 
								      mcast->cb_done.function,	       
								      mcast->cb_done.clientdata,
								      _zeroBuf,
								      _zeroBytes,
								      isroot,
								      classroute);	      
	    return rc;
	  }
	};
    };
  };
};

#endif		  
