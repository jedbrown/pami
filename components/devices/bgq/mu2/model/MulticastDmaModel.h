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
#include "util/trace.h"
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/msg/InjectDPutMulticast.h"

#include "Memory.h"

#define MAX_COUNTERS 12
#define MAX_VEC_SIZE 16

#ifndef L1_DCACHE_LINE_SIZE
#define L1_DCACHE_LINE_SIZE   64
#endif

#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"

#undef DO_TRACE_ENTEREXIT 
#undef DO_TRACE_DEBUG     
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
      ///
      /// \brief MulticastDmaModel to inject dput p2p multicasts on the network
      //
      class MulticastDmaModel : public Interface::MulticastModel < MulticastDmaModel, MU::Context, 0 > {
      public:
	struct MulticastDmaRecv {	  	  
	  void initialize (pami_multicast_t *mcast) {
	    TRACE_FN_ENTER();	    
	    _connid = mcast->connection_id;
	    _inuse = 1;
	    _pwq = (PAMI::PipeWorkQueue *)mcast->dst;
	    _fn = mcast->cb_done.function;
	    _cookie = mcast->cb_done.clientdata;
	    TRACE_FN_EXIT();		    
	  }

	  void produceBytes (int bytes) {
	    TRACE_FN_ENTER();
	    _pwq->produceBytes (bytes);
	    TRACE_FN_EXIT();		    
	  }
	  
	  unsigned                _connid;
	  unsigned                _inuse;
	  PipeWorkQueue         * _pwq;
	  pami_event_function     _fn;
	  void                  * _cookie; 
	};

	static const size_t sizeof_msg = 0;

	MulticastDmaModel (pami_client_t    client,
			   pami_context_t   context,
			   MU::Context                 & device, 
			   pami_result_t               & status) : 
	  Interface::MulticastModel<MulticastDmaModel, MU::Context, 0> (device, status), 
	  _mucontext(device),
	  _nActiveRecvs(0),
	  _nActiveSends(0),
	  _nRecvsComplete(0),
	  _nSendsComplete(0),
	  _callConsumeBytes (true),
	  _localMulticast(true),
	  _polling(false),
	  _mytask(__global.mapping.task())	    
	  //_curBaseAddress(0)
	  {	    
	    TRACE_FN_ENTER();
	    char * buf;
	    //buf = (char*) malloc (sizeof(InjectDPutMulticast) * MAX_COUNTERS);
	    __global.heap_mm->memalign((void**)&buf, 64, sizeof(InjectDPutMulticast) * MAX_COUNTERS);  
	    memset(buf, 0, sizeof(InjectDPutMulticast) * MAX_COUNTERS);

	    for (int i = 0; i < MAX_COUNTERS; ++i) {
	      char *sbuf = buf + sizeof(InjectDPutMulticast) * i;
	      new (sbuf) InjectDPutMulticast(device,context);
	    }
	    _sends = (InjectDPutMulticast *)buf;

	    //_recvs = (MulticastDmaRecv *) malloc (sizeof(MulticastDmaRecv) * MAX_COUNTERS);
	    __global.heap_mm->memalign((void**)&_recvs, 64, sizeof(MulticastDmaRecv) * MAX_COUNTERS); 
	    memset(_recvs, 0, sizeof(MulticastDmaRecv) * MAX_COUNTERS);	    

	    memset ((void *)_counterVec, 0, sizeof(_counterVec));
	    memset (_counterShadowVec, 0, sizeof(_counterShadowVec));
	    
	    if (__global.mapping.tSize() == 1)
	      _localMulticast = false;

	    ///// Get the BAT IDS ///////////////
	    //// Setup CounterVec in BAT 
	    int32_t rcBAT = 0;
	    rcBAT = _mucontext.allocateBatIds (MAX_COUNTERS, _b_batids);  
	    if (rcBAT == -1)
	      {
		status = PAMI_ERROR;
	    TRACE_FN_EXIT();
		return;
	      }

	    uint16_t batid = 0;
	    rcBAT = _mucontext.allocateBatIds (1, &batid);  
	    if (rcBAT == -1)
	      {
		status = PAMI_ERROR;
	    TRACE_FN_EXIT();
		return;
	      }
	    _c_batid = batid;

	    //printf ("Get Bat Ids %d %d\n", _batids[0], _batids[1]);

	    int rc = 0;
	    Kernel_MemoryRegion_t memRegion;
	    rc = Kernel_CreateMemoryRegion (&memRegion, (void *)_counterVec, sizeof(_counterVec));
	    PAMI_assert ( rc == 0 );
	    uint64_t paddr = (uint64_t)memRegion.BasePa +
	      ((uint64_t)(void *)_counterVec - (uint64_t)memRegion.BaseVa);
	    
	    uint64_t atomic_address = MUSPI_GetAtomicAddress(paddr, MUHWI_ATOMIC_OPCODE_STORE_ADD);
	    _mucontext.setBatEntry (_c_batid, atomic_address);

	    _me = *_mucontext.getMuDestinationSelf();
	    new (&_work) PAMI::Device::Generic::GenericThread(advance, this);

	    initModels ();	    
	    status = PAMI_SUCCESS;
	    TRACE_FN_EXIT();
	  }

	  void setLocalMulticast (bool val) {
	    _localMulticast = val;
	  }
	  
	  void callConsumeBytesOnMaster (bool val) {
	    _callConsumeBytes = val;
	  }

	  void initModels() {
	    TRACE_FN_ENTER();
	    buildP2pDputModelDescriptor (_modeldesc, _b_batids[0], _c_batid);
	    _modeldesc.setDeposit (MUHWI_PACKET_DEPOSIT);	    
	    
	    for (int i = 0; i < MAX_COUNTERS; i++) {
	      MUSPI_DescriptorBase * dput = &_sends[i]._desc;
	      _modeldesc.clone (*dput);
	    }	    

	    TRACE_FN_EXIT();		    
	  };
	  
	  ~MulticastDmaModel () 
	    {
	      TRACE_FN_ENTER();
	      TRACE_FN_EXIT();		    
	    }
	  
	  
	  void processSend (pami_multicast_t *mcast,
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
	    size_t idx = _nActiveSends++;
	    MUSPI_DescriptorBase * dput = &_sends[idx]._desc;
	    _sends[idx].initialize ( mcast->cb_done.function,
				     mcast->cb_done.clientdata,					  
				     &_me, 
				     ref, 
				     isTorus, 
				     ll,
				     ur,
				     pwq,
				     length,
				     _localMulticast );
	    
	    // Clone the single-packet model descriptor into the injection fifo
	    PAMI_assert (mcast->connection_id < MAX_COUNTERS);
	    dput->PacketHeader.messageUnitHeader.Packet_Types.Direct_Put.Counter_Offset = mcast->connection_id * sizeof(uint64_t);
	    dput->PacketHeader.messageUnitHeader.Packet_Types.Direct_Put.Rec_Payload_Base_Address_Id = _b_batids[mcast->connection_id];

	    bool done = _sends[idx].advance();	    
	    if (!done && !_polling) {
	      _polling = true;
	      PAMI::Device::Generic::GenericThread *work = (PAMI::Device::Generic::GenericThread *)&_work;
	      _mucontext.getProgressDevice()->postThread(work);
	    }
	    else if (done) {
	      _nActiveSends --;
	    }	      
	    TRACE_FN_EXIT();		    
	  }

	  void processRecv (pami_multicast_t *mcast,
			    void             *devinfo = NULL) 
	  {
	    TRACE_FN_ENTER();
	    int connid = mcast->connection_id;
	    PAMI_assert(connid < MAX_COUNTERS);	
	    
	    int idx = _nActiveRecvs ++;
	    PAMI_assert (_nActiveRecvs <= MAX_COUNTERS);
	    PAMI_assert(_recvs[idx]._inuse == 0);

	    _counterVec[connid] = mcast->bytes;
	    _counterShadowVec[idx] = mcast->bytes;
	    _recvs[idx].initialize(mcast);

	    Kernel_MemoryRegion_t memRegion;
	    char *payload = (char *) ((PAMI::PipeWorkQueue *)mcast->dst)->bufferToProduce();
	    uint rc = 0;
	    rc = Kernel_CreateMemoryRegion (&memRegion, payload, mcast->bytes);
	    PAMI_assert ( rc == 0 );
	    uint64_t paddr = (uint64_t)memRegion.BasePa +
	      ((uint64_t)payload - (uint64_t)memRegion.BaseVa);
	    _mucontext.setBatEntry (_b_batids[connid], paddr);

	    if (!_polling) {
	      _polling = true;
	      PAMI::Device::Generic::GenericThread *work = (PAMI::Device::Generic::GenericThread *)&_work;
	      _mucontext.getProgressDevice()->postThread(work);
	    }
	    TRACE_FN_EXIT();		    
	  }
  
	  pami_result_t postMulticastImmediate_impl(size_t                client,
						    size_t                context, 
						    pami_multicast_t    * mcast,
						    void                * devinfo=NULL) 
	  {
	    TRACE_FN_ENTER();
	    // Get the source data buffer/length and validate (assert) inputs
	    PAMI::Topology *root_topo = (PAMI::Topology*)mcast->src_participants;
	    if ((root_topo != NULL)  && (root_topo->index2Rank(0) == _mytask))
	      processSend (mcast, devinfo);
	    else
	      processRecv (mcast, devinfo);
	    TRACE_FN_EXIT();
	    return PAMI_SUCCESS;
	  }

	  /// \see PAMI::Device::Interface::MulticastModel::postMulticast
	  pami_result_t postMulticast_impl(uint8_t (&state)[MulticastDmaModel::sizeof_msg],
					   size_t            client,
					   size_t            context,
					   pami_multicast_t *mcast,
					   void             *devinfo = NULL) 
	  {
			TRACE_FN_ENTER();
			TRACE_FN_EXIT();		    	    
	    return PAMI_SUCCESS;
	  }

	  static pami_result_t advance (pami_context_t     context,
					void             * cookie) 
	    {
	      MulticastDmaModel *model = (MulticastDmaModel *)cookie;	      
	      for (int i =0; i < 4; ++i) {
		size_t events = model->advance_recvs(context);		
		model->advance_sends (context);
		
		if (events == 0 && model->_nActiveRecvs > 0)
		  break;
		
		if (model->_nActiveRecvs == model->_nRecvsComplete) {
		  model->reset_recvs(context);
		  break;
		}
	      }
	      
	      if (model->_nActiveRecvs == 0 && model->_nActiveSends == 0) {
		model->_polling = false;
		return PAMI_SUCCESS;
	      }
	      
	      return PAMI_EAGAIN;
	    }

	  size_t advance_recvs (pami_context_t     context)
	  {
	    TRACE_FN_ENTER();
	    size_t events = 0;
	    unsigned i = 0;
	    for (i=0; i < _nActiveRecvs; i++)  {
	      MulticastDmaRecv *recv = &_recvs[i];
	      if (recv->_inuse) {
		uint32_t cid = recv->_connid;
		uint64_t cc = _counterVec[cid];
		uint64_t bytes = _counterShadowVec[i] - cc;
		if (bytes > 0) {
		  Memory::sync();
		  recv->produceBytes (bytes);
		  _counterShadowVec[i] = cc;
		  
		  //Trigger a send check on counter i??
		  if (cc == 0) {
		    recv->_inuse = 0;
		    _nRecvsComplete++;	 
		  }
		  events = 1;
		}
	      }	 
	    }
	    TRACE_FN_EXIT();		    	    
	    return events;
	  }

	  void reset_recvs (pami_context_t context) {	    
	    unsigned nrecvs = _nActiveRecvs;
	    _nActiveRecvs = 0;
	    _nRecvsComplete = 0;
	    
	    unsigned i = 0;
	    for (i=0; i < nrecvs; i++)  {
	      MulticastDmaRecv *recv = &_recvs[i];
	      _mucontext.setBatEntry (_b_batids[_recvs[i]._connid], 0);	      
	      if (recv->_fn)
		recv->_fn (context, recv->_cookie, PAMI_SUCCESS);
	    }
	  }	  	  

	  void advance_sends (pami_context_t     context)
	  {
	    TRACE_FN_ENTER();
	    unsigned i = 0; 
	    for (i=0; i < _nActiveSends; i++)  {
	      if (!_sends[i].done()) {
		bool done = _sends[i].advance();
		if (done) 
		  _nSendsComplete++;
	      }
	    }
	    
	    if (_nSendsComplete == _nActiveSends) {
	      _nSendsComplete = 0;
	      _nActiveSends = 0;
	    }
	    TRACE_FN_EXIT();
	  }

	  
      protected:
	  MU::Context                              & _mucontext;
	  unsigned                                   _nActiveRecvs;
	  unsigned                                   _nActiveSends;
	  unsigned                                   _nRecvsComplete;
	  unsigned                                   _nSendsComplete;
	  bool                                       _callConsumeBytes;
	  bool                                       _localMulticast;
	  bool                                       _polling;
	  pami_task_t                                _mytask;
	  uint32_t                                   _c_batid;                 /// The base address table id for counter
	  uint16_t                                   _b_batids[MAX_VEC_SIZE];  /// The base address table id for payload
	  MUHWI_Destination_t                        _me;
	  uint64_t                                   _counterShadowVec[MAX_VEC_SIZE]; //A list of shadow counters
	  MulticastDmaRecv                         * _recvs; 
	  InjectDPutMulticast                      * _sends;
	  pami_work_t                                _work;	  
	  MUSPI_DescriptorBase                       _modeldesc;         /// Model descriptor
	  

	  ///These counters are indexed by the connetion id known to the remote node
	  volatile uint64_t      _counterVec[MAX_VEC_SIZE] __attribute__((__aligned__(L1_DCACHE_LINE_SIZE)));

      } __attribute__((__aligned__(L1_DCACHE_LINE_SIZE)));
    };
  };
};

#undef DO_TRACE_ENTEREXIT 
#undef DO_TRACE_DEBUG     

#endif
