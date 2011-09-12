
#ifndef __components_devices_bgq_mu2_msg_InjectDPutListMulticast_h__
#define __components_devices_bgq_mu2_msg_InjectDPutListMulticast_h__

#include "components/devices/bgq/mu2/msg/InjectDPutBase.h"

#define NUM_FIFOS   10

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {      
      ///
      /// \brief Inject one or more descriptors into an inject fifo
      ///
      ///
      class InjectDPutListMulticast : public InjectDPutBase
      {
      public:	
	/// \brief Constructor
	/// \param[in] context the MU context for this message
	InjectDPutListMulticast (MU::Context &context):
	  InjectDPutBase(),
	  _context (context),
	  _nextdst (0),
	  _ndestinations(0),
	  _processInjection (false),
	  _localMulticast(1),
	  _length(0),	
	  _consumedBytes(0),
	  _pwq(NULL),
	  _fn (NULL),
	  _cookie (NULL),
	  _ranks(NULL),
	  _nranks(0)
	    {
	      //Default constructor
	    }

	///
	/// \brief Inject descriptor(s) into a specific injection fifo
	///
	/// \param[in] fn  completion event fn
	/// \param[in] cookie callback cookie
	/// \param[in] me  my coordinates as an MUHWI_Destination_t 
	/// \param[in] ref  the ref of the axial topology
	/// \param[in] istorus istorus bits
	/// \param[in] ll  lower left of the axial
	/// \param[in] ur  upper right of the axial
	/// \param[in] pwq pipeworkqueue that has data to be consumed
	/// \param[in] length the totaly number of bytes to be transfered
	/// \param[in] localMultcast : should this message do local sends
	///
	void initialize (pami_event_function   fn,
			 void                * cookie,	    
			 pami_task_t         * ranks,
			 size_t                nranks,
			 PipeWorkQueue       * pwq,
			 uint64_t              length,
			 uint32_t              localMulticast) 
	{
	  TRACE_FN_ENTER();	 
	  _nextdst          = 0;
	  _ndestinations    = nranks;
	  _processInjection = true;
	  this->_doneCompletion   = false;
	  _length           = length;	
	  _consumedBytes    = 0;
	  _pwq              = pwq;
	  _fn               = fn;
	  _cookie           = cookie;
	  _ranks            = ranks;
	  _nranks           = nranks;

	  for (unsigned fnum = 0; fnum < NUM_FIFOS; fnum++)
	    _lastCompletionSeqNo[fnum] = UNDEFINED_SEQ_NO;
	  
	  // Determine the physical address of the (temporary) payload
	  // buffer from the model state memory.
	  char *payload = (char*)pwq->bufferToConsume();
	  Kernel_MemoryRegion_t  memRegion;
	  uint32_t rc;
	  rc = Kernel_CreateMemoryRegion (&memRegion, payload, length);
	  PAMI_assert ( rc == 0 );
	  uint64_t paddr = (uint64_t)memRegion.BasePa +
	    ((uint64_t)payload - (uint64_t)memRegion.BaseVa);
	  
	  _desc.setPayload (paddr, 0); 
	  _desc.setRecPutOffset(0);

	  //_localMulticast = localMulticast;
	  TRACE_FN_EXIT();
	};
	  
	inline ~InjectDPutListMulticast () {};
	
	virtual bool advance ()
	{
	  //TRACE_FN_ENTER();
	  //printf("InjectDPutMulticast:  advance:  nextdst=%u ndst=%u\n",_nextdst, _ndestinations);
	  uint64_t sequence = 0;
	  uint64_t bytes_available = 0;
	  unsigned done = 1;

	  if (likely(_processInjection)) {
	    //The is computed when the first descriptor for this round is injected
	    if (likely(_nextdst == 0)) {  //When nextdst == 0
	      bytes_available = _pwq->bytesAvailableToConsume();
	      if (bytes_available == _consumedBytes)
		return false;
	      
	      uint64_t newbytes    = bytes_available - _consumedBytes;
	      _desc.Message_Length = newbytes;
	    }
	    else 
	      bytes_available = _consumedBytes + _desc.Message_Length;	    

	    if (_nextdst < _ndestinations) 
	    {
	      register double fp0 asm("fr0");
	      register double fp1 asm("fr1");	      
	      VECTOR_LOAD_NU (&_desc,  0, fp0);
	      VECTOR_LOAD_NU (&_desc, 32, fp1);		  	    

	      MUHWI_MessageUnitHeader_t *muh_model = &(_desc.PacketHeader.messageUnitHeader);
	      uint pid = muh_model->Packet_Types.Direct_Put.Rec_Payload_Base_Address_Id;
	      uint cid = muh_model->Packet_Types.Direct_Put.Rec_Counter_Base_Address_Id;	      
	      
	      do { 
		bool islocal = __global.mapping.isPeer(__global.mapping.task(), _ranks[_nextdst]);
		if (!islocal || _localMulticast) {
		  MUHWI_Destination_t   dest;
		  uint64_t              map;
		  size_t                tcoord;
		  uint32_t              fnum;
		  fnum = _context.pinFifo (_ranks[_nextdst],
					   0,
					   dest,
					   tcoord,
					   map);
		  
		  InjChannel & channel = _context.injectionGroup.channel[fnum];
		  bool flag = channel.hasFreeSpaceWithUpdate ();
		  
		  if (likely(flag)) {
		    // Clone the message descriptors directly into the injection fifo.
		    MUSPI_DescriptorBase * d = (MUSPI_DescriptorBase *) channel.getNextDescriptor ();
		    //_desc.clone (*d);
		    VECTOR_STORE_NU (d,  0, fp0);
		    VECTOR_STORE_NU (d, 32, fp1);	
		    d->setDestination(dest);
		    d->Torus_FIFO_Map = map;
		    
		    MUHWI_MessageUnitHeader_t *muh_dput  = &(d->PacketHeader.messageUnitHeader);		    
		    muh_dput->Packet_Types.Direct_Put.Rec_Payload_Base_Address_Id = _context.pinBatId(tcoord, pid); 
		    muh_dput->Packet_Types.Direct_Put.Rec_Counter_Base_Address_Id = _context.pinBatId(tcoord, cid);
		    
		    sequence = channel.injFifoAdvanceDesc ();	      
		    _lastCompletionSeqNo[fnum] = sequence;
		  }
		  //No descriptor slots available, so come back and try later
		  else return false;
		}
		_nextdst++;
	      } while(_nextdst < _ndestinations);
	    }

	    if (bytes_available < _length) {
	      reset(bytes_available);	 
	      return false;
	    }
	  } //We have completed processing all injections
	  
	  _processInjection = false;
	  //printf ("Completed injection\n");

	  for (unsigned fnum = 0; fnum < NUM_FIFOS; fnum++) 	    
	    if (_lastCompletionSeqNo[fnum] != UNDEFINED_SEQ_NO) {
	      InjChannel & channel = _context.injectionGroup.channel[fnum];
	      unsigned rc = channel.checkDescComplete(_lastCompletionSeqNo[fnum]);	      
	      _lastCompletionSeqNo[fnum] |= seqno_table[rc];
	      done &=  rc;
	    }
	  this->_doneCompletion = done;	 

	  if (done && _fn) {
	    //printf ("Done completion \n");
	    _fn (NULL, _cookie, PAMI_SUCCESS);
	  }
	  
	  //TRACE_FN_EXIT();
	  return this->_doneCompletion;
	}
      
	///
	/// \brief Reset the internal state of the message
	///
	/// \note Only used for message reuse.
	///
	inline void reset (uint64_t bytes_available) { 
	  _nextdst = 0; 
	  uint64_t newbytes         = bytes_available - _consumedBytes; 
	  _consumedBytes            = bytes_available; 
	  _desc.setRecPutOffset(/*_recPayloadOffset +*/ _consumedBytes);
	  _desc.Pa_Payload          = _desc.Pa_Payload + newbytes;	    
	}
	
	inline PipeWorkQueue *getPwq() { return _pwq; }

	inline uint32_t ndestinations() { return _ndestinations; }

	//The descriptor is setup externally and contains batids, sndbuffer base and msg length
	MUSPI_DescriptorBase     _desc __attribute__((__aligned__(32))); 

      protected:	
	MU::Context            & _context;
	uint32_t                 _nextdst;
	uint32_t                 _ndestinations;
	bool                     _processInjection;
	bool                     _localMulticast;
	uint64_t                 _length;        //Number of bytes to transfer
	uint64_t                 _consumedBytes;
	PipeWorkQueue          * _pwq;
	pami_event_function      _fn;
	void                   * _cookie;
	pami_task_t            * _ranks;
	size_t                   _nranks;	  
	uint8_t                  _fifos[NUM_FIFOS];
	uint32_t                 _destinations[NUM_FIFOS];
	uint64_t                 _lastCompletionSeqNo[NUM_FIFOS];
      } __attribute__((__aligned__(32))); //class PAMI::Device::MU::InjectDPutListMulticast     

    };   // namespace PAMI::Device::MU                          
  };     // namespace PAMI::Device           
};       // namespace PAMI                                   

#endif // __components_devices_bgq_mu2_msg_InjectDPutListMulticast_h__                    
