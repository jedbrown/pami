
#ifndef __components_devices_bgq_mu2_msg_InjectDPutMulticast_h__
#define __components_devices_bgq_mu2_msg_InjectDPutMulticast_h__

#include "spi/include/mu/DescriptorBaseXX.h"
#include "components/devices/bgq/mu2/trace.h"
#include "components/devices/bgq/mu2/InjChannel.h"
#include "components/devices/bgq/mu2/msg/MessageQueue.h"
#include "components/devices/bgq/mu2/trace.h"
#include "common/bgq/Mapping.h"

#define MAX_DIMS  5

#define MESH            0
#define TORUS_POSITIVE  PAMI::Interface::Mapping::TorusPositive
#define TORUS_NEGATIVE  PAMI::Interface::Mapping::TorusNegative

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      
      static const uint64_t    _fifomaps[MAX_DIMS*2]  =  
	{
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP 
	};
      
      static const uint8_t     _hintsABCD[MAX_DIMS*2] = 
	{MUHWI_PACKET_HINT_AM,
	 MUHWI_PACKET_HINT_AP,
	 MUHWI_PACKET_HINT_BM,
	 MUHWI_PACKET_HINT_BP,
	 MUHWI_PACKET_HINT_CM,
	 MUHWI_PACKET_HINT_CP,
	 MUHWI_PACKET_HINT_DM,
	 MUHWI_PACKET_HINT_DP,
	 MUHWI_PACKET_HINT_A_NONE | 
	 MUHWI_PACKET_HINT_B_NONE | 
	 MUHWI_PACKET_HINT_C_NONE | 
	 MUHWI_PACKET_HINT_D_NONE,
	 MUHWI_PACKET_HINT_A_NONE | 
	 MUHWI_PACKET_HINT_B_NONE | 
	 MUHWI_PACKET_HINT_C_NONE | 
	 MUHWI_PACKET_HINT_D_NONE};
      
      static const uint8_t     _hintsE[MAX_DIMS*2] = {
	MUHWI_PACKET_HINT_E_NONE,
	MUHWI_PACKET_HINT_E_NONE,
	MUHWI_PACKET_HINT_E_NONE,
	MUHWI_PACKET_HINT_E_NONE,
	MUHWI_PACKET_HINT_E_NONE,
	MUHWI_PACKET_HINT_E_NONE,
	MUHWI_PACKET_HINT_E_NONE,
	MUHWI_PACKET_HINT_E_NONE,
	MUHWI_PACKET_HINT_EM,
	MUHWI_PACKET_HINT_EP
      };

      ///
      /// \brief Inject one or more descriptors into an inject fifo
      ///
      ///
      class InjectDPutMulticast : public MessageQueue::Element
      {
      public:
	///
	/// \brief Inject descriptor(s) into a specific injection fifo
	///
	/// \param[in] injfifo Specific injection fifo for the descriptor(s)
	///
	InjectDPutMulticast (MU::Context         & context,
			     pami_event_function   fn,
			     void                * cookie,					  
			     MUHWI_Destination_t * me, 
			     pami_coord_t        * ref, 
			     unsigned char       * isTorus, 
			     pami_coord_t        * ll, 
			     pami_coord_t        * ur,
			     PipeWorkQueue       * pwq,
			     uint32_t              length
			     ) :
	  _context (context),
	  _nextdst (0),
	  _ncomplete (0),
	  _ndestinations(0),
	  _length(length),	
	  _consumedBytes(0),
	  _pwq(pwq),
	  _fn (fn),
	  _cookie (cookie),
	  _recPayloadOffset (0),
	  _done (false)
	    {
	      //TRACE_FN_ENTER();
	      
	      setupDestinations (me, ref, isTorus, ll, ur);
	      
	      //TRACE_FN_EXIT();
	    };
	  
	inline ~InjectDPutMulticast () {};
	
	void setRecPutOffset (uint64_t rec_offset) { 
	  _recPayloadOffset = rec_offset; 
	}

	void init() {
	  char *payload = (char*)_pwq->bufferToConsume();
	  PAMI_assert (payload != NULL);
	  //printf ("PWQ payload 0x%lx\n", (uint64_t) payload);
	  
	  // Determine the physical address of the (temporary) payload
	  // buffer from the model state memory.
	  Kernel_MemoryRegion_t memRegion;
	  uint32_t rc;
	  rc = Kernel_CreateMemoryRegion (&memRegion, payload, _length);
	  PAMI_assert ( rc == 0 );
	  uint64_t paddr = (uint64_t)memRegion.BasePa +
	    ((uint64_t)payload - (uint64_t)memRegion.BaseVa);
	  
	  _desc.setPayload (paddr, 0);	      
	}

	void setupDestinations(MUHWI_Destination_t *me, pami_coord_t *ref, unsigned char *isTorus, pami_coord_t *ll, pami_coord_t *ur)
	{
	  unsigned  i;
	  //Process dimensions A B C D E
	  for (i = 0; i < MAX_DIMS; i++ ){ 
	    int nidx = 2*i;
	    int pidx = 2*i+1;
	    int dstidx = (MAX_DIMS - i - 1) * 6;

	    if (isTorus[i] == MESH) //Mesh
	      {
		//positive direction
		if (ur->u.n_torus.coords[i] != ref->u.n_torus.coords[i]) {
		  //printf ("HERE %d %ld \n\n", i, ur->u.n_torus.coords[i]);
		  _destinations[_ndestinations].Destination.Destination = 
		    (me->Destination.Destination & ~(0x3f << dstidx)) | (ur->u.n_torus.coords[i] << dstidx);
		  _fifos[_ndestinations] = pidx;
		  _ndestinations ++;
		}		
	    
		//negative direction
		if (ll->u.n_torus.coords[i] != ref->u.n_torus.coords[i]) {
		  _destinations[_ndestinations].Destination.Destination = 
		    (me->Destination.Destination & ~(0x3f << dstidx))|(ll->u.n_torus.coords[i] << dstidx);
		  _fifos[_ndestinations] = nidx;
		  _ndestinations ++;
		}
	      }                                                                     
	    else if (isTorus[i] == TORUS_POSITIVE)
	      {
		if (ref->u.n_torus.coords[i] == ll->u.n_torus.coords[i])
		  _destinations[_ndestinations].Destination.Destination =
		    (me->Destination.Destination & ~(0x3f << dstidx))|(ur->u.n_torus.coords[i] << dstidx);
		else
		  _destinations[_ndestinations].Destination.Destination =
		    (me->Destination.Destination & ~(0x3f << dstidx))|((ref->u.n_torus.coords[i]-1) << dstidx);
		_fifos[_ndestinations] = pidx;
		_ndestinations ++;
	      }
	    else if (isTorus[i] == TORUS_NEGATIVE)
	      {
		if (ref->u.n_torus.coords[i] == ur->u.n_torus.coords[i])
		  _destinations[_ndestinations].Destination.Destination =
		    (me->Destination.Destination & ~(0x3f << dstidx))|((ll->u.n_torus.coords[i]) << dstidx);
		else
		  _destinations[_ndestinations].Destination.Destination =
		    (me->Destination.Destination & ~(0x3f << dstidx))|((ref->u.n_torus.coords[i]+1) << dstidx);
		_fifos[_ndestinations] = nidx;
	      }
	  }
	  
#if 0
	  for (i = 0; i < _ndestinations; i++)
	    printf ("Sending to 0x%x %d %d %d %d %d\n", 
		    _destinations[i].Destination.Destination,
		    _destinations[i].Destination.A_Destination,
		    _destinations[i].Destination.B_Destination,
		    _destinations[i].Destination.C_Destination,
		    _destinations[i].Destination.D_Destination,
		    _destinations[i].Destination.E_Destination
		    );
#endif
	}  
	
	///
	/// \brief Inject descriptor message virtual advance implementation
	///
	/// The inject descriptor message is complete only after all
	/// descriptors have been injected into the injection fifo.
	///
	/// The number of descriptors to inject is specified as a template
	/// parameter for the class.
	///
	/// \see MU::MessageQueue::Element::advance
	///
	/// \retval true  Message complete
	/// \retval false Message incomplete and must remain on the queue
	///
	virtual bool advance ()
	{
	  //TRACE_FN_ENTER();	  
	  //TRACE_FORMAT("InjectDPutMulticast:  advance:  ndesc=%zu\n",ndesc);

	  if (_done)  return true;
	  
	  uint64_t sequence = 0;
	  uint64_t bytes_available = 0, newbytes = 0;
	  if (_nextdst == 0) {
	    bytes_available = _pwq->bytesAvailableToConsume();
	    if (bytes_available == _consumedBytes)
	      return false;
	    
	    newbytes    = bytes_available - _consumedBytes;
	    _desc.Message_Length = newbytes;
	  }

	  //The is computed when the first descriptor for this round is injected
	  bytes_available = _consumedBytes + _desc.Message_Length;
	  for (; _nextdst < _ndestinations; _nextdst++) {
	    size_t                fnum    = _fifos[_nextdst];
	    uint64_t              map     = _fifomaps[fnum];
	    uint8_t               hints   = _hintsABCD[fnum];
	    uint8_t               hints_e = _hintsE[fnum];
	    MUHWI_Destination_t & dest    = _destinations[_nextdst];

	    InjChannel & channel = _context.injectionGroup.channel[fnum];
	    size_t ndesc = channel.getFreeDescriptorCountWithUpdate ();
	    
	    if (ndesc > 0) {
	      // Clone the message descriptors directly into the injection fifo.
	      MUSPI_DescriptorBase * d = (MUSPI_DescriptorBase *) channel.getNextDescriptor ();
	    
	      _desc.clone (*d);
	      d->setDestination(dest);
	      d->setHints(hints, hints_e);
	      d->Torus_FIFO_Map = map;

	      //TRACE_FORMAT("inject descriptor (%p) from message (%p)", &desc, this);
	      
	      // Advance the injection fifo tail pointer. This will be
	      // moved outside the loop when an "advance multiple" is
	      // available.
	      sequence = channel.injFifoAdvanceDesc ();

	      //last descriptor to that destination
	      if (unlikely(_fn != NULL) && bytes_available == _length)
		{
		  channel.addCompletionEvent (_state, done, this, sequence);
		}
	    }
	  }
	  
	  if (_nextdst == _ndestinations && bytes_available < _length ) {
	    reset();	 
	    newbytes                  = bytes_available - _consumedBytes; 
	    _consumedBytes            = bytes_available; 
	    _desc.setRecPutOffset(_recPayloadOffset + _consumedBytes);
	    _desc.Pa_Payload          = _desc.Pa_Payload + newbytes;
	  }
	  
	  _done = (bytes_available == _length && _nextdst == _ndestinations);
	  //TRACE_FN_EXIT();
	  return _done;
	}
	
	static void done (pami_context_t     context,
			  void             * cookie,
			  pami_result_t      result) 
	  {
	    InjectDPutMulticast *msg = (InjectDPutMulticast *) cookie;
	    msg->_ncomplete ++;
	    if (msg->_ncomplete == msg->_ndestinations) {
	      msg->_pwq->consumeBytes((size_t)msg->_length);
	      msg->_fn (context, msg->_cookie, result);
	    }
	  }

	///
	/// \brief Reset the internal state of the message
	///
	/// \note Only used for message reuse.
	///
	inline void reset () { _nextdst = 0; _ncomplete = 0; }
	
	inline PipeWorkQueue *getPwq() { return _pwq; }

	MUSPI_DescriptorBase     _desc; //The descriptor is setup externally and contains batids, sndbuffer base and msg length
	
      protected:
	
	MU::Context            & _context;
	uint32_t                 _nextdst;
	uint32_t                 _ncomplete;
	uint32_t                 _ndestinations;
	uint32_t                 _length;        //Number of bytes to transfer
	uint64_t                 _consumedBytes;
	PipeWorkQueue          * _pwq;
	pami_event_function      _fn;
	void                   * _cookie;
	uint64_t                 _recPayloadOffset;
	bool                     _done;
	uint8_t                  _fifos[MAX_DIMS*2];
	MUHWI_Destination_t      _destinations[MAX_DIMS*2];
	uint8_t                  _state[InjChannel::completion_event_state_bytes];

      }; // class     PAMI::Device::MU::InjectDPutMulticast     
    };   // namespace PAMI::Device::MU                          
  };     // namespace PAMI::Device           
};       // namespace PAMI                                   

#endif // __components_devices_bgq_mu2_msg_InjectDPutMulticast_h__                     

