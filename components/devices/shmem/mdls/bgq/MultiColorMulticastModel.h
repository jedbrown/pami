
#ifndef __components_devices_shmem_mdls_MultiColorMcstModel_h__
#define __components_devices_shmem_mdls_MultiColorMcstModel_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "Memory.h"

#include "sys/pami.h"
#include "common/bgq/TypeDefs.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/shmem/msgs/BaseMessage.h"

#include "util/trace.h"
#include "math/Memcpy.x.h"

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      static const size_t mcolor_state_bytes = 0;
      template <class T_Device, class T_Atomic>
        class MultiColorMulticastModel : public Interface::MulticastModel < MultiColorMulticastModel<T_Device, T_Atomic>, T_Device, mcolor_state_bytes >
      {
      public:
	static const size_t  sizeof_msg = mcolor_state_bytes;
	static const size_t  NumColors  = 12;

      protected:

	struct MultiColorDescriptor
	{
	  char                  * _buf; 
	  volatile  unsigned      _bytesAvailable;
	  uint16_t                _nconsumers;
	  uint16_t                _root;
	  
	  void initialize(char        * buf,
			  unsigned      nconsumers,
			  unsigned      root,
			  unsigned      bytes_available=0)
	  {
	    _buf            = buf;
	    _bytesAvailable = bytes_available;
	    _root           = root;
	    _nconsumers     = nconsumers;
	  }
	};

	typedef MultiColorDescriptor  *   MCDPtr;

	struct ShmemState {
	  MultiColorDescriptor    _descriptors[NumColors];
	  MCDPtr                  _recvdescriptors[NumColors];
	};

	struct AtomicVec {
	  T_Atomic  _atomics[NumColors];  //Vector of NumColors atomics
	  //Constructor
	  AtomicVec () {}
	};

	enum MultiColorStatus {
	  MULTI_COLOR_DONE = 0,
	  MULTI_COLOR_SEND = 1,
	  MULTI_COLOR_RECV = 2
	};
	
	class MsgState {
	public:
	  int                    _bytes;
	  PipeWorkQueue        * _pwq;
	  char                 * _buf;
	  pami_event_function    _cb_done;
	  void                 * _cookie;
	  
	  MsgState () {
	    _pwq     = NULL;
	    _buf     = NULL;
	    _bytes   = 0;
	    //	    _status  = MULTI_COLOR_DONE;
	    _cb_done = NULL;
	    _cookie  = NULL;
	  }

	  void initialize (MultiColorStatus      status,
			   pami_multicast_t    * mcast) 
	  {	    
	    if (status == MULTI_COLOR_SEND) {
	      _pwq = (PAMI::PipeWorkQueue *) mcast->src;	    
	      _buf     = _pwq->bufferToConsume();
	    }
	    
	    if (status == MULTI_COLOR_RECV) {
	      _pwq = (PAMI::PipeWorkQueue *) mcast->dst;
	      _buf     = _pwq->bufferToProduce();
	    }
	    
	    _bytes   = mcast->bytes;
	    _cb_done = mcast->cb_done.function;
	    _cookie  = mcast->cb_done.clientdata;
	  }
	};

      public:

	//Multicast Model
	MultiColorMulticastModel (pami_client_t         client,
				  pami_context_t        context,
				  T_Device            & device, 
				  pami_result_t       & status) :
	  Interface::MulticastModel < MultiColorMulticastModel<T_Device, T_Atomic>, T_Device, mcolor_state_bytes > (device, status),
	  _polling(false),
	  _advanceFlag(true),
	  _context(context),
	  _mytask(__global.mapping.task()),
	  _localrank(__global.mapping.t()),
	  _device(device),
	  _shmstate(NULL)
	{
          pami_result_t rc;
          char key[PAMI::Memory::MMKEYSIZE];
          //Allocate shared memory
	  char *unique_string = _device.getUniqueString();
          sprintf(key, "/MultiColorMulticastModel-shmstate-%s", 
		  unique_string);

	  size_t tsize = __global.mapping.tSize();
          size_t total_size = tsize * sizeof(ShmemState); 
	  void *shmbuf;
          rc = __global.mm.memalign 
	    (&shmbuf,
	     64,
	     total_size,
	     key,
	     shmem_buf_initialize,
	     NULL);

	  if(rc != PAMI_SUCCESS) {
	    status = PAMI_ERROR;
	    return;
	  }	    
	  
	  _shmstate = (ShmemState *) shmbuf;
	  
	  AtomicVec *atomic_vec;
	  __global.heap_mm->memalign((void**)&atomic_vec, 64, sizeof(AtomicVec) * tsize);
	  size_t c = 0, t = 0;
	  for(t = 0; t < tsize; ++t) {
	    new (&atomic_vec[t]) AtomicVec();
	    sprintf(key, "/MultiColorMulticastModel-atomics-%s-%ld", 
		    unique_string, t);
	    T_Atomic::init(&__global.mm, key, atomic_vec[t]._atomics);
	  }
	  _atomicVec = atomic_vec;

	  new (&_work) PAMI::Device::Generic::GenericThread(advance, this); 
	  for (c = 0; c < NumColors; ++c) {
	    _msgsstatus[c] = MULTI_COLOR_DONE;
	    _msgrstatus[c] = MULTI_COLOR_DONE;
	  }	  
	  status = PAMI_SUCCESS;
        };

	void disableAdvance() { _advanceFlag = false; }	  

	pami_result_t postMulticast_impl(uint8_t (&state)[mcolor_state_bytes],
					 size_t             client,
					 size_t             context,
					 pami_multicast_t * mcast, 
					 void             * devinfo) 
	{
	  PAMI_abort();
	  return PAMI_ERROR;
	}	

	pami_result_t postMulticastImmediate_impl(size_t             client,
						  size_t             context,
						  pami_multicast_t * mcast, 
						  void             * devinfo) 
        {
	  TRACE_FN_ENTER();

	  PAMI::Topology *src_topo = (PAMI::Topology *)mcast->src_participants;
	  unsigned conn_id = mcast->connection_id;
	  PAMI_assert (_localrank < __global.mapping.tSize());
	  PAMI_assert (conn_id < NumColors);
	  MultiColorDescriptor *cur_desc = &(_shmstate[_localrank]._descriptors[conn_id]);

	  //Sending root node
	  if ((src_topo != NULL)  && (src_topo->index2Rank(0) == _mytask))
	    processRoot(mcast, cur_desc);
	  else
	    processPeer(mcast);

	  TRACE_FN_EXIT();

	  return PAMI_SUCCESS;
	}
	  
	void processRoot (pami_multicast_t         * mcast,
			  MultiColorDescriptor     * desc) 
	{
	  TRACE_FN_ENTER();		  

	  PAMI::Topology *dst_topo = (PAMI::Topology *)mcast->dst_participants;
	  size_t ndstranks = 0;
	  int color = mcast->connection_id;

	  PAMI_assert (_msgsstatus[color] == MULTI_COLOR_DONE);
	  _msgsstatus[color] = MULTI_COLOR_SEND;
	  _msgsstate[color].initialize(MULTI_COLOR_SEND, mcast);
	  
	  //Convert local address to node-wide address
	  void* mybuf = _msgsstate[color]._buf;
	  Memregion mregion;
	  size_t bytes_out;
	  mregion.createMemregion(&bytes_out, mcast->bytes, mybuf, 0);
	  PAMI_assert(bytes_out == mcast->bytes);
	  void* phy_addr = (void*)mregion.getBasePhysicalAddress();
	  void * global_vaddr = NULL;
	  uint32_t rc = 0;
	  rc = Kernel_Physical2GlobalVirtual (phy_addr, &global_vaddr);
	  PAMI_assert(rc == 0);	  

	  unsigned bytes_available = 
	    (unsigned)_msgsstate[color]._pwq->bytesAvailableToConsume();
	  //Clear atomics
	  _atomicVec[_localrank]._atomics[color].clear();

	  if (dst_topo->type() == PAMI_AXIAL_TOPOLOGY) {
	    pami_coord_t *ll = NULL;
	    pami_coord_t *ur = NULL;
	    pami_coord_t *ref = NULL;
	    unsigned char *isTorus = NULL;

	    pami_result_t result = PAMI_SUCCESS;
	    result = dst_topo->axial(&ll, &ur, &ref, &isTorus);
	    PAMI_assert(result == PAMI_SUCCESS);
	    size_t start =  ll->u.n_torus.coords[LOCAL_DIM];
	    size_t end   =  ur->u.n_torus.coords[LOCAL_DIM];
	    ndstranks = end - start;
	    desc->initialize ((char*)global_vaddr, 
			      ndstranks, 
			      _localrank, 
			      bytes_available);
	    Memory::sync();

	    for (size_t r = start; r <= end; ++r)
	      if (r != _localrank)
		postDesc(desc, r, mcast->connection_id);
	  }	  
	  else //if (dst_topo->type() == PAMI_LIST_TOPOLOGY)
	  {
	    //Compute local destinations
	    //pami_task_t *ranks = NULL;
	    //dst_topo->rankList(&ranks);
	    pami_task_t ranks[128];
	    size_t nranks;
	    dst_topo->getRankList(128, ranks, &nranks);

	    ndstranks = 0;
	    for (size_t r = 0; r < nranks; ++r) 
	      if (__global.mapping.isPeer(_mytask, ranks[r]))
		ndstranks ++;
	    
	    desc->initialize ((char*)global_vaddr, ndstranks, _localrank); 
	    Memory::sync();
	    
	    for (size_t r =0; r < nranks; ++r) 
	      if (__global.mapping.isPeer(_mytask, ranks[r])) {
		pami_coord_t coords;
		__global.mapping.task2network(ranks[r], &coords,
					      PAMI_N_TORUS_NETWORK);

		postDesc(desc,
			 coords.u.n_torus.coords[LOCAL_DIM], 
			 mcast->connection_id);
	      }	    
	  }
	  //else
	  //PAMI_abort();

	  //Post work to generic device
	  if (!_polling && _advanceFlag) {
	    _polling = true;
	    PAMI::Device::Generic::GenericThread *work = (PAMI::Device::Generic::GenericThread *) &_work;
	    _device.getLocalProgressDevice()->postThread(work);
	  }

	  TRACE_FN_EXIT();		  
	}
	
	void processPeer (pami_multicast_t *mcast) 
	{
	  TRACE_FN_ENTER();	  
	  PAMI_assert (_msgrstatus[mcast->connection_id] == MULTI_COLOR_DONE);
	  _msgrstatus[mcast->connection_id] = MULTI_COLOR_RECV;
	  _msgrstate[mcast->connection_id].initialize(MULTI_COLOR_RECV, mcast);

	  if (!_polling && _advanceFlag) {
	    _polling = true;
	    PAMI::Device::Generic::GenericThread *work = (PAMI::Device::Generic::GenericThread *) &_work;
	    _device.getLocalProgressDevice()->postThread(work);
	  }

	  TRACE_FN_EXIT();		  
	}

	void postDesc (MultiColorDescriptor    *desc,
		       int                      dst,
		       int                      color) 
	{
	  TRACE_FN_ENTER();
	  PAMI_assert (dst  < (int)__global.mapping.tSize());
	  PAMI_assert (color < (int)NumColors);
	  PAMI_assert (_shmstate[dst]._recvdescriptors[color] == NULL);
	  _shmstate[dst]._recvdescriptors[color] = desc;

	  TRACE_FN_EXIT();
	}
	
	int advancePeer (int              color,
			 MsgState       * msg)
	{
	  TRACE_FN_ENTER();		  

	  //The root has not started the multicast
	  PAMI_assert (_localrank  < __global.mapping.tSize());
	  PAMI_assert (color < (int)NumColors);
	  MultiColorDescriptor *desc = (MultiColorDescriptor *)
	    _shmstate[_localrank]._recvdescriptors[color];
	  
	  if (desc == NULL) {
	    TRACE_FN_EXIT();
	    return PAMI_EAGAIN;
	  }

	  PAMI_assert (_msgrstatus[color] == MULTI_COLOR_RECV);
	  int bytes_produced = msg->_pwq->getBytesProduced();
	  int bytes_available = desc->_bytesAvailable;
	  if (bytes_produced < bytes_available) {
	    int bytes = bytes_available - bytes_produced;
	    Core_memcpy(msg->_buf + bytes_produced, 
			(void *)(desc->_buf + bytes_produced),
			bytes);
	    msg->_pwq->produceBytes(bytes);  
	  }

	  if (bytes_available < msg->_bytes) {
	    TRACE_FN_EXIT();
	    return PAMI_EAGAIN;
	  }
	  
	  int root = desc->_root;	  
	  PAMI_assert (_localrank  < __global.mapping.tSize());
	  PAMI_assert (color < (int)NumColors);
	  
	  //reset recv desc slot
	  _shmstate[_localrank]._recvdescriptors[color] = NULL;
	  
	  //Call atomic increment
	  _atomicVec[root]._atomics[color].fetch_and_inc();
	  
	  _msgrstatus[color] = MULTI_COLOR_DONE;
	  
	  //Call completion callback
	  if (msg->_cb_done) {
	    //	    fprintf (stderr, "Calling Done Callback\n");
	    msg->_cb_done (_context, msg->_cookie, PAMI_SUCCESS); 
	  }
	  
	  TRACE_FN_EXIT();	  
	  return PAMI_SUCCESS;	  
	}

	int advanceRoot (int              color,
			 MsgState       * msg) 
	{
	  TRACE_FN_ENTER();
       
	  PAMI_assert (_localrank  < __global.mapping.tSize());
	  PAMI_assert (color < (int)NumColors);
	  MultiColorDescriptor *desc = 
	    &(_shmstate[_localrank]._descriptors[color]);

	  unsigned bytes_available = (unsigned)msg->_pwq->bytesAvailableToConsume();
	  if (bytes_available > desc->_bytesAvailable) {
	    Memory::sync();
	    desc->_bytesAvailable = bytes_available;
	  }

	  if (bytes_available < (unsigned)msg->_bytes) {
	    TRACE_FN_EXIT();
	    return PAMI_EAGAIN;
	  }

	  size_t n_arrived = _atomicVec[_localrank]._atomics[color].fetch();
	  //fprintf (stderr, "Waiting for peers %ld, %d\n", 
	  //   n_arrived, desc->_nconsumers);

	  if (n_arrived == (size_t)desc->_nconsumers)
	  {
	    if (msg->_cb_done) {
	      //fprintf(stderr,"Master calling done\n");
	      msg->_cb_done (_context, msg->_cookie, PAMI_SUCCESS);
	    }
	    
	    _msgsstatus[color] = MULTI_COLOR_DONE;

	    TRACE_FN_EXIT();
	    return PAMI_SUCCESS;	    
	  }	  
	  
	  TRACE_FN_EXIT();
	  return PAMI_EAGAIN;	  
	}

	static pami_result_t advance (pami_context_t     context,
				      void             * cookie) 
	{
	  TRACE_FN_ENTER();
		  
	  MultiColorMulticastModel<T_Device, T_Atomic> *model = 
	    (MultiColorMulticastModel<T_Device, T_Atomic> *) cookie;
	  int active = 0;
	  
	  for (size_t i = 0; i < NumColors; ++i) {
	    MultiColorStatus status = (MultiColorStatus)model->_msgsstatus[i];
	    if (status == MULTI_COLOR_SEND)
	      active |= model->advanceRoot(i, &model->_msgsstate[i]);
	    status = (MultiColorStatus)model->_msgrstatus[i];
	    if (status == MULTI_COLOR_RECV)
	      active |= model->advancePeer(i, &model->_msgrstate[i]);	      
	  }
	  
	  pami_result_t rc = (active)? PAMI_EAGAIN : PAMI_SUCCESS;
	  if (rc == PAMI_SUCCESS)
	    model->_polling = false;
	  
	  TRACE_FN_EXIT();	  
	  return rc;
	}

	static void shmem_buf_initialize (void       * memory,
					  size_t       bytes,
					  const char * key,
					  unsigned     attributes,
					  void       * cookie)
	{
	  memset(memory, 0, bytes);
	}	

      protected:
	
	bool             _polling;
	bool             _advanceFlag;
	pami_context_t   _context;
	pami_task_t      _mytask;
	unsigned         _localrank;		
	T_Device       & _device;
	ShmemState     * _shmstate;
	AtomicVec      * _atomicVec;
	MultiColorStatus _msgrstatus[NumColors];
	MultiColorStatus _msgsstatus[NumColors];
	MsgState         _msgrstate[NumColors];
	MsgState         _msgsstate[NumColors];	  
	pami_work_t      _work;

      };  // PAMI::Device::Shmem::MultiColorMulticastModel class      
    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif 
