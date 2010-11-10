/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/model/MulticastDmaModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_model_MulticastDmaModel_h__
#define __components_devices_bgq_mu2_model_MulticastDmaModel_h__

#include "components/devices/MulticastModel.h"
#include "components/memory/MemoryAllocator.h"
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/msg/InjectDPutMulticast.h"


#define MAX_COUNTERS 12

#ifndef L1_DCACHE_LINE_SIZE
#define L1_DCACHE_LINE_SIZE   64
#endif

#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"
#include "util/trace.h"

#ifdef CCMI_TRACE_ALL
  #define DO_TRACE_ENTEREXIT 1
  #define DO_TRACE_DEBUG     1
#else
  #define DO_TRACE_ENTEREXIT 0
  #define DO_TRACE_DEBUG     0
#endif

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class MulticastDmaModel : public Interface::MulticastModel < MulticastDmaModel, MU::Context, sizeof(InjectDPutMulticast) > {
      public:
	class MulticastDmaRecv {	  	  
	public:
	  
	  MulticastDmaRecv (pami_multicast_t *mcast) {
	    TRACE_FN_ENTER();
	    
	    _pwq = (PAMI::PipeWorkQueue *)mcast->dst;
	    _payload = (char*)_pwq->bufferToProduce();
	    //bytes_available = _pwq->bytesAvailableToProduce();

	    _fn = mcast->cb_done.function;
	    _cookie = mcast->cb_done.clientdata;
	    TRACE_FN_EXIT();		    
	  }

	  void produceBytes (int bytes) {
	    TRACE_FN_ENTER();
	    _pwq->produceBytes (bytes);
	    TRACE_FN_EXIT();		    
	  }
	  
	  PipeWorkQueue         * _pwq;
	  pami_event_function     _fn;
	  void                  * _cookie; 
	  char                  * _payload;
	};

	static const unsigned sizeof_msg = sizeof(InjectDPutMulticast);
	static const unsigned POLLING_SENDS = 0x1;
	static const unsigned POLLING_RECVS = 0x2;
	
	MulticastDmaModel (MU::Context                 & device, 
			   pami_result_t               & status) : 
	  Interface::MulticastModel<MulticastDmaModel, MU::Context, sizeof(InjectDPutMulticast)> (device, status), 
	  _mucontext(device),
	  _nActiveRecvs(0),
	  _nActiveSends(0),
	  _flags(0) //,
	  //_curBaseAddress(0)
	  {	    
	    TRACE_FN_ENTER();
	    memset(_sends, 0, sizeof(_sends));
	    memset(_recvs, 0, sizeof(_recvs));
	    memset ((void *)_counterVec, 0, sizeof(_counterVec));
	    memset (_counterShadowVec, 0, sizeof(_counterShadowVec));
	    memset (_recvIdVec, 0, sizeof(_recvIdVec));
	    
	    ///// Get the BAT IDS ///////////////
	    //// Setup CounterVec in BAT 
	    int32_t rcBAT = 0;
	    rcBAT = _mucontext.allocateBatIds (MAX_COUNTERS, _b_batids);  
	    if (rcBAT == -1)
	      {
		status = PAMI_ERROR;
		return;
	      }

	    rcBAT = _mucontext.allocateBatIds (1, &_c_batid);  
	    if (rcBAT == -1)
	      {
		status = PAMI_ERROR;
		return;
	      }

	    //printf ("Get Bat Ids %d %d\n", _batids[0], _batids[1]);
	    uint rc = 0;

	    Kernel_MemoryRegion_t memRegion;
	    rc = Kernel_CreateMemoryRegion (&memRegion, (void *)_counterVec, sizeof(_counterVec));
	    PAMI_assert ( rc == 0 );
	    uint64_t paddr = (uint64_t)memRegion.BasePa +
	      ((uint64_t)(void *)_counterVec - (uint64_t)memRegion.BaseVa);
	    
	    uint64_t atomic_address = MUSPI_GetAtomicAddress(paddr, MUHWI_ATOMIC_OPCODE_STORE_ADD);
	    _mucontext.setBatEntry (_c_batid, atomic_address);

	    _me = *_mucontext.getMuDestinationSelf();

	    initModels ();
	    TRACE_FN_EXIT();
	  }

	  void initModels() {
	    TRACE_FN_ENTER();
	    // Zero-out the descriptor models before initialization
	    memset((void *)&_modeldesc, 0, sizeof(_modeldesc));
	    
	    // --------------------------------------------------------------------
	    // Set the common base descriptor fields
	    // 
	    // For the remote get packet, send it using the high priority torus
	    // fifo map.  Everything else uses non-priority torus fifos, pinned
	    // later based on destination.  This is necessary to avoid deadlock
	    // when the remote get fifo fills.  Note that this is in conjunction
	    // with using the high priority virtual channel (set elsewhere).
	    // --------------------------------------------------------------------
	    MUSPI_BaseDescriptorInfoFields_t base;
	    memset((void *)&base, 0, sizeof(base));
	    
	    base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
	    _modeldesc.setBaseFields (&base);
	    
	    // --------------------------------------------------------------------
	    // Set the common point-to-point descriptor fields
	    //
	    // For the remote get packet, send it on the high priority virtual
	    // channel.  Everything else is on the deterministic virtual channel.
	    // This is necessary to avoid deadlock when the remote get fifo fills.
	    // Note that this is in conjunction with setting the high priority
	    // torus fifo map (set elsewhere).
	    // --------------------------------------------------------------------
	    MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
	    memset((void *)&pt2pt, 0, sizeof(pt2pt));
	    
	    pt2pt.Misc1 =
	      MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
	      MUHWI_PACKET_DO_NOT_DEPOSIT |
	      MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
	    pt2pt.Misc2 =
	      MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;
	    
	    _modeldesc.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
	    _modeldesc.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
	    _modeldesc.setPt2PtFields (&pt2pt);
	    
	    // --------------------------------------------------------------------                                                      
	    // Set the direct put descriptor fields                                                                                      
	    // --------------------------------------------------------------------                                                      
	    MUSPI_DirectPutDescriptorInfoFields dput;
	    memset((void *)&dput, 0, sizeof(dput));
	    
	    dput.Rec_Payload_Base_Address_Id = _b_batids[0];
	    dput.Rec_Payload_Offset          = 0;
	    dput.Rec_Counter_Base_Address_Id = _c_batid;
	    dput.Rec_Counter_Offset          = 0;
	    dput.Pacing                      = MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED;
	    
	    _modeldesc.setDirectPutFields (&dput);
	    _modeldesc.setMessageUnitPacketType (MUHWI_PACKET_TYPE_PUT);  
	    _modeldesc.setDeposit (MUHWI_PACKET_DEPOSIT);	    
	    TRACE_FN_EXIT();		    
	  };
	  
	  ~MulticastDmaModel () 
	    {
	      TRACE_FN_ENTER();
	      TRACE_FN_EXIT();		    
	    }
	  
	  
	  void processSend (uint8_t (&state)[MulticastDmaModel::sizeof_msg],
			    pami_multicast_t *mcast,
			    void             *devinfo = NULL) 
	  {
	    TRACE_FN_ENTER();
	    size_t length = mcast->bytes;
	    PAMI::Topology *dst_topology = (PAMI::Topology *)mcast->dst_participants;
	    PAMI_assert (dst_topology->type() == PAMI_AXIAL_TOPOLOGY);

	    pami_coord_t *ll=NULL;
	    pami_coord_t *ur=NULL;
	    pami_coord_t *ref=NULL;
	    unsigned char *isTorus=NULL;
	    
	    pami_result_t result = PAMI_SUCCESS;
	    result = dst_topology->axial(&ll, &ur, &ref, &isTorus);
	    PAMI_assert(result == PAMI_SUCCESS);

	    PipeWorkQueue *pwq = (PAMI::PipeWorkQueue *)mcast->src;
	    InjectDPutMulticast *msg = new (&state) InjectDPutMulticast (_mucontext, 
									 mcast->cb_done.function,
									 mcast->cb_done.clientdata,					  
									 &_me, 
									 ref, 
									 isTorus, 
									 ll,
									 ur,
									 pwq,
									 length);
	    	    	    
	    _sends[_nActiveSends] = msg;
	    
	    // Clone the single-packet model descriptor into the injection fifo
	    MUSPI_DescriptorBase * dput = &msg->_desc;
	    _modeldesc.clone (*dput);
	    
	    msg->init();
	    PAMI_assert (mcast->connection_id < MAX_COUNTERS);
	    dput->PacketHeader.messageUnitHeader.Packet_Types.Direct_Put.Counter_Offset = mcast->connection_id * sizeof(uint64_t);
	    dput->PacketHeader.messageUnitHeader.Packet_Types.Direct_Put.Rec_Payload_Base_Address_Id = _b_batids[mcast->connection_id];

	    //Passing the put offset
	    //if (_nActiveSends != 0) 
	    //msg->setRecPutOffset ((uint64_t)pwq->bufferToConsume() - (uint64_t)_curBaseAddress);
	    //else //_nActiveSends == 0
	    //_curBaseAddress = (uint64_t)pwq->bufferToConsume();

	    _nActiveSends ++;
	    bool done = msg->advance();

	    if (!done && !(_flags & POLLING_SENDS)) {
	      _flags |= POLLING_SENDS;	      
	      PAMI::Device::Generic::GenericThread *work = new (&_swork) PAMI::Device::Generic::GenericThread(advance_sends, this);
	      _mucontext.getProgressDevice()->postThread(work);
	    }
	    else if (done) {
	      _nActiveSends --;
	      _sends[_nActiveSends] = NULL;
	      //if (_nActiveSends == 0)
	      //_curBaseAddress = 0;
	    }	      
	    TRACE_FN_EXIT();		    
	  }

	  void processRecv (uint8_t (&state)[MulticastDmaModel::sizeof_msg],
			    pami_multicast_t *mcast,
			    void             *devinfo = NULL) 
	  {
	    TRACE_FN_ENTER();
	    int connid = mcast->connection_id;
	    PAMI_assert(connid < MAX_COUNTERS);	
	    
	    int idx = _nActiveRecvs ++;
	    PAMI_assert (_nActiveRecvs < MAX_COUNTERS);
	    PAMI_assert(_recvs[idx] == NULL);

	    //fprintf(stderr, "Processing recv for idx %d connid %d\n", idx, connid);

	    _recvIdVec[idx] = connid;
	    _counterVec[connid] = mcast->bytes;
	    _counterShadowVec[idx] = mcast->bytes;
	    _recvs[idx] = new (state) MulticastDmaRecv(mcast);
	    
	    //if (idx == 0) {
	    char *payload = _recvs[idx]->_payload;
	    Kernel_MemoryRegion_t memRegion;
	    uint rc = 0;
	    rc = Kernel_CreateMemoryRegion (&memRegion, payload, mcast->bytes);
	    PAMI_assert ( rc == 0 );
	    uint64_t paddr = (uint64_t)memRegion.BasePa +
	      ((uint64_t)payload - (uint64_t)memRegion.BaseVa);
	    //fprintf (stderr, "Setting BAT entry\n");
	    _mucontext.setBatEntry (_b_batids[connid], paddr);
	    //}

	    if ((_flags & POLLING_RECVS) == 0) {
	      _flags |= POLLING_RECVS;	      	      
	      PAMI::Device::Generic::GenericThread *work = new (&_rwork) PAMI::Device::Generic::GenericThread(advance_recvs, this);
	      _mucontext.getProgressDevice()->postThread(work);
	    }
	    TRACE_FN_EXIT();		    
	  }
  
	  /// \see PAMI::Device::Interface::MulticastModel::postMulticast
	  pami_result_t postMulticast_impl(uint8_t (&state)[MulticastDmaModel::sizeof_msg],
					   pami_multicast_t *mcast,
					   void             *devinfo = NULL) 
	  {
	    TRACE_FN_ENTER();
	    // Get the source data buffer/length and validate (assert) inputs
	    if (mcast->src_participants)
	      processSend (state, mcast, devinfo);
	    else
	      processRecv (state, mcast, devinfo);
	    TRACE_FN_EXIT();		    
	    return PAMI_SUCCESS;
	  }

	  static pami_result_t advance_recvs (pami_context_t     context,
					      void             * cookie)
	    {
	      TRACE_FN_ENTER();
	      unsigned i = 0, ncomplete = 0;
	      MulticastDmaModel *model = (MulticastDmaModel *)cookie;

	      for (i=0; i < model->_nActiveRecvs; i++)  {
		if (model->_recvs[i] != NULL) {
		  uint8_t cid = model->_recvIdVec[i];
		  uint64_t cc = model->_counterVec[cid];
		  if (cc != model->_counterShadowVec[i]) {
		    mem_sync();

		    //printf("Advancing counter %d connid %d\n", i, cid);

		    model->_recvs[i]->produceBytes (model->_counterShadowVec[i] - cc);
		    model->_counterShadowVec[i] = cc;
		  }
		  
		  //Trigger a send check on counter i??
		  if (cc == 0) {
		    ncomplete++;		      
		    if (model->_recvs[i]->_fn)
		      model->_recvs[i]->_fn (context, model->_recvs[i]->_cookie, PAMI_SUCCESS);
		    model->_recvs[i] = NULL;
		    model->_mucontext.setBatEntry (model->_b_batids[cid], 0);
		  }
		}
		else {
		  ncomplete++;
		}
	      }

	      if (model->_nActiveRecvs == ncomplete) {
		//printf ("All recvs completed\n");

		//All connections of this collective have completed
		model->_nActiveRecvs = 0;
		model->_flags &= ~(POLLING_RECVS);
		//model->_mucontext.setBatEntry (model->_batids[0], 0);
		
		TRACE_FN_EXIT();		    
		return PAMI_SUCCESS;
	      }
	      
	      TRACE_FN_EXIT();		    
	      return PAMI_EAGAIN;
	    }
	  

	  static pami_result_t advance_sends (pami_context_t     context,
					      void             * cookie)
	    {
	      TRACE_FN_ENTER();
	      MulticastDmaModel *model = (MulticastDmaModel *)cookie;
	      unsigned i = 0, ncomplete = 0;	      
	      for (i=0; i < model->_nActiveSends; i++)  {
		if (model->_sends[i] != NULL) {
		  bool done = model->_sends[i]->advance();
		  if (done) {
		    model->_sends[i] = NULL;
		    ncomplete++;
		  }
		}
		else
		  ncomplete ++;
	      }

	      if (ncomplete == model->_nActiveSends) {
		model->_nActiveSends = 0;
		model->_flags &= ~(POLLING_SENDS);
		//model->_curBaseAddress = 0;

		TRACE_FN_EXIT();		    
		return PAMI_SUCCESS;
	      }

	      TRACE_FN_EXIT();	 
	      return PAMI_EAGAIN;
	    }
	  
      protected:
	  MUSPI_DescriptorBase                       _modeldesc;         /// Model descriptor
	  uint16_t                                   _b_batids[MAX_COUNTERS];  /// The base address table id for payload
	  uint16_t                                   _c_batid;             /// The base address table id for counter
	  MUHWI_Destination_t                        _me;
	  MU::Context                              & _mucontext;
	  unsigned                                   _nActiveRecvs;
	  unsigned                                   _nActiveSends;
	  unsigned                                   _flags;
	  uint8_t                                    _recvIdVec[MAX_COUNTERS];
	  //uint64_t                                   _curBaseAddress;
	  uint64_t                                   _counterShadowVec[MAX_COUNTERS]; //A list of shadow counters
	  MulticastDmaRecv                         * _recvs[MAX_COUNTERS];
	  InjectDPutMulticast                      * _sends[MAX_COUNTERS];
	  pami_work_t                                _swork;	  
	  pami_work_t                                _rwork;

	  ///These counters are indexed by the connetion id known to the remote node
	  volatile uint64_t      _counterVec[MAX_COUNTERS] __attribute__((__aligned__(L1_DCACHE_LINE_SIZE)));

      } __attribute__((__aligned__(L1_DCACHE_LINE_SIZE)));
    };
  };
};

#endif
