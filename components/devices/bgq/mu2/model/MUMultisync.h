
#ifndef __components_devices_bgq_mu2_model_MUMultisync_h__
#define __components_devices_bgq_mu2_model_MUMultisync_h__

#include <spi/include/kernel/gi.h>
#include <spi/include/mu/GIBarrier.h>

#include "components/devices/bgq/mu2/Context.h"
#include "sys/pami.h"
#include "components/devices/MultisyncModel.h"

//#define MU_BLOCKING_BARRIER 1

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      static const size_t  msync_bytes     = 0;
      static const size_t  NumClassRoutes  = 16;
      static const size_t  cw_classroute   = 0;
      class MUMultisyncModel : public Interface::MultisyncModel<MUMultisyncModel, MU::Context, msync_bytes>
      {
      public:
	static const size_t sizeof_msg = msync_bytes;

	struct CompletionMsg {
	  pami_event_function   cb_done;
	  void                * cookie;
	  MUSPI_GIBarrier_t   * barrier;
	};

	MUMultisyncModel (pami_client_t     client,
			  pami_context_t    context,
			  MU::Context     & mucontext,
			  pami_result_t   & status):
	Interface::MultisyncModel<MUMultisyncModel, MU::Context, msync_bytes> (mucontext, status),
	  _context(context),
	  _mucontext (mucontext),
	  _gdev(*_mucontext.getProgressDevice())
	  {	    
	    //Initialize comm world
	    memset (_inited, 0, sizeof(_inited));
	    int rc = 0;
	    rc = MUSPI_GIBarrierInit ( &_giBarrier[cw_classroute], cw_classroute );
	    PAMI_assert (rc == 0);
	    _inited[cw_classroute] = 1;

	    new (&_work) PAMI::Device::Generic::GenericThread(advance, &_completionmsg);
	  } 
	
	/// \see PAMI::Device::Interface::MultisyncModel::postMultisync
	pami_result_t postMultisyncImmediate_impl(size_t            client,
						  size_t            context, 
						  pami_multisync_t *msync,
						  void             *devinfo = NULL) 
	{
	  size_t classroute = 0;
	  if (devinfo) 
	    classroute = (size_t)devinfo - 1;

	  if (!_inited[classroute]) {
	    int rc = 0;
	    rc = MUSPI_GIBarrierInit ( &_giBarrier[classroute], classroute );
	    PAMI_assert (rc == 0);
	    _inited[classroute] = 1;
	  }

#ifdef MU_BLOCKING_BARRIER
	  MUSPI_GIBarrierEnterAndWait (&_giBarrier[classroute]);
	  if (msync->cb_done.function) 
	    msync->cb_done.function(_context, msync->cb_done.clientdata, PAMI_SUCCESS);
#else	  
	  MUSPI_GIBarrierEnter (&_giBarrier[classroute]);
	  
	  //Crate a work object and post work to generic device
	  _completionmsg.cb_done = msync->cb_done.function;
	  _completionmsg.cookie  = msync->cb_done.clientdata;
	  _completionmsg.barrier = &_giBarrier[classroute];
	  
	  PAMI::Device::Generic::GenericThread *work = (PAMI::Device::Generic::GenericThread *)&_work;
	  _gdev.postThread(work);
#endif

	  return PAMI_SUCCESS;
	}

	pami_result_t postMultisync_impl(uint8_t (&state)[msync_bytes],
					 size_t            client,
					 size_t            context, 
					 pami_multisync_t *msync,
					 void             *devinfo = NULL) 
	{
	  return PAMI_ERROR;
	}

	static pami_result_t advance (pami_context_t     context,
				      void             * cookie)
	{
	  CompletionMsg *msg = (CompletionMsg *) cookie;	  
	  int rc = MUSPI_GIBarrierPoll (msg->barrier);
	  
	  if (rc != 0) 
	    return PAMI_EAGAIN;

	  if (msg->cb_done) 
	    msg->cb_done(context, msg->cookie, PAMI_SUCCESS);
	  
	  return PAMI_SUCCESS;
	}

      protected:
	pami_context_t             _context;
	MU::Context              & _mucontext;
	Generic::Device          & _gdev;
	bool                       _inited [NumClassRoutes];
	MUSPI_GIBarrier_t          _giBarrier[NumClassRoutes];	
	pami_work_t                _work;
	CompletionMsg              _completionmsg;
      };
    };
  };
};

#endif
