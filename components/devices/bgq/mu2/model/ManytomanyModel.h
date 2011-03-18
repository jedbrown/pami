
#ifndef __components_devices_bgq_mu2_ManytomanyModel_h__
#define __components_devices_bgq_mu2_ManytomanyModel_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/msg/InjectAMMulticast.h"
#include "math/Memcpy.x.h"

#include <pami.h>
#include "components/devices/ManytomanyModel.h"
#include "components/devices/bgq/mu2/msg/InjectAMManytomany.h"
#include "M2MPipeWorkQueue.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      static const size_t m2m_state_bytes = sizeof(InjectAMManytomany<size_t, 0>); 
      template <typename T_Int, bool T_Single>
      class ManytomanyModel : public Interface::ManytomanyModel<ManytomanyModel<T_Int, T_Single>, MU::Context, m2m_state_bytes>
      {
      protected:
	static const uint64_t AnyFIFOMap =    
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM |
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP | 
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM |
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP | 
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM |
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP |
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM |
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP |
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM |
	  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP;
	
	class M2mRecv {
	public:
	  M2MPipeWorkQueueT<T_Int, T_Single> * _pwq;
	  pami_callback_t                      _cb_done;
	  bool                                 _init; 
	  uint32_t                             _ncomplete;

	  M2mRecv (): _init(false) {}

	  bool initialized () { return _init; }

	  void initialize (pami_context_t                       ctxt, 	
			   pami_dispatch_manytomany_function    recv_func, 
			   void                               * async_arg, 
			   unsigned                             connid);

	  void processPacket (uint32_t         srcidx,
			      size_t           poffset,
			      size_t           pbytes,
			      void           * payload,
			      pami_context_t   ctxt) 
	  {
	    size_t bytes_produced = _pwq->getBytesProduced(srcidx);
	    //Get the start of the buffer and then add packet offset to it
	    char * buf = (char *)_pwq->bufferToProduce(srcidx) + poffset - bytes_produced;
	    size_t bytes_available = _pwq->bytesAvailableToProduce(srcidx);
	    //Compute the total bytes expected for this src idx
	    size_t total_bytes = bytes_produced + bytes_available;
	    //Is this the last packet?
	    size_t is_complete = (bytes_available - pbytes - 1) >> 63;
	    _ncomplete  += is_complete;
	    //Compute how many bytes can be copied 
	    size_t bytes = ((pbytes + poffset) <= total_bytes) ? pbytes : (total_bytes - poffset);
	    _pwq->produceBytes(srcidx, bytes);	    	    
	    //Core_memcpy (buf, payload, bytes);
	    
	    if ( (bytes == 512) && (((uint64_t)buf & 0x1F) == 0) )
	      quad_copy_512(buf, (char *)payload);
	    else
	      _int64Cpy(buf, (char *)payload, bytes);

	    //printf ("Process Packet from src %d bytes avail %ld packet bytes %ld\n", srcidx, bytes_available, pbytes);
	    if (unlikely(_ncomplete == _pwq->numActive())) { 
	      _init = false;
	      if ( likely(_cb_done.function != NULL) )
		_cb_done.function (ctxt, _cb_done.clientdata, PAMI_SUCCESS);  
	    }
	  }
	  
	};
      
      public:
	static const size_t sizeof_msg = m2m_state_bytes;

	ManytomanyModel (pami_client_t client, pami_context_t context, MU::Context &device, pami_result_t &status) :	
	Interface::ManytomanyModel<ManytomanyModel<T_Int, T_Single>, MU::Context, m2m_state_bytes> (device, status),
	  _mucontext(device),
	  _cached_connid ((uint32_t)-1),
	  _cached_recv (NULL),
	  _client(client),
	  _ctxt(context)
	    {
	      initDesc();
	      status = PAMI_SUCCESS;
	    }
	
	~ManytomanyModel<T_Int, T_Single> ()
          {
          }
	
	void initDesc () {
	  // Zero-out the descriptor models before initialization
	  memset((void *)&_modeldesc, 0, sizeof(MUSPI_DescriptorBase));
	
	  // --------------------------------------------------------------------
	  // Set the common base descriptor fields
	  // --------------------------------------------------------------------
	  MUSPI_BaseDescriptorInfoFields_t base;
	  memset((void *)&base, 0, sizeof(base));

	  base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
	  base.Payload_Address = 0;
	  base.Message_Length  = 0;
	  base.Torus_FIFO_Map  = AnyFIFOMap;
	  base.Dest.Destination.Destination = 0;

	  _modeldesc.setBaseFields (&base);
	  
	  // --------------------------------------------------------------------
	  // Set the common point-to-point descriptor fields
	  // --------------------------------------------------------------------
	  MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
	  memset((void *)&pt2pt, 0, sizeof(pt2pt));
	  
	  pt2pt.Hints_ABCD = 0;
	  pt2pt.Skip       = 0;
	  pt2pt.Misc1 =
	    MUHWI_PACKET_USE_DYNAMIC_ROUTING |
	    MUHWI_PACKET_DO_NOT_DEPOSIT |
	    MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
	  pt2pt.Misc2 =
	    MUHWI_PACKET_VIRTUAL_CHANNEL_DYNAMIC | MUHWI_PACKET_ZONE_ROUTING_1;
	  	 
	  _modeldesc.setPt2PtFields (&pt2pt);
	  _modeldesc.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
	  _modeldesc.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
	
	  // --------------------------------------------------------------------
	  // Set the common memory fifo descriptor fields
	  // --------------------------------------------------------------------
	  MUSPI_MemoryFIFODescriptorInfoFields_t memfifo;
	  memset ((void *)&memfifo, 0, sizeof(memfifo));
	  
	  memfifo.Rec_FIFO_Id    = 0;
	  memfifo.Rec_Put_Offset = 0;
	  memfifo.Interrupt      = 0; //MUHWI_DESCRIPTOR_INTERRUPT_ON_PACKET_ARRIVAL;
	  memfifo.SoftwareBit    = 0;
	  
	  _modeldesc.setMemoryFIFOFields (&memfifo);
	  _modeldesc.setMessageUnitPacketType (MUHWI_PACKET_TYPE_FIFO);

	  // --------------------------------------------------------------------
	  // Set the network header information in the descriptor models to
	  // differentiate between a single-packet transfer and a multi-packet
	  // transfer
	  // --------------------------------------------------------------------
	  MemoryFifoPacketHeader * hdr = NULL;	  
	  hdr = (MemoryFifoPacketHeader *) & _modeldesc.PacketHeader;
	  hdr->setSinglePacket (false);	

	  //MUSPI_DescriptorDumpHex ((char *)"Model", &_modeldesc);
	} 
	
	void *getRecvFromMap (uint32_t connid)  __attribute__((__noinline__, weak));

	M2mRecv *getRecvObject (uint32_t connid) {
	  if (unlikely(connid != _cached_connid)) {
	    _cached_connid = connid;
	    _cached_recv   = (M2mRecv *)getRecvFromMap(connid);
	  }	 
	  
	  PAMI_assert (_cached_recv != NULL);
	  return _cached_recv;
	}

	pami_result_t registerManytomanyRecvFunction_impl(int dispatch_id,
							  pami_dispatch_manytomany_function recv_func,
							  void  *async_arg)
	{
	  // Implementation
	  _dispatch        = dispatch_id;
	  _recv_func       = recv_func;
	  _async_arg       = async_arg;
	  
	  uint16_t id = 0;
	  if (_mucontext.registerPacketHandler (_dispatch,
						dispatch_func,
						this,
						id))
	  {
	    MemoryFifoPacketHeader * hdr = NULL;	      
	    hdr = (MemoryFifoPacketHeader *) & _modeldesc.PacketHeader;
	    hdr->setDispatchId (id);	 
	    return PAMI_SUCCESS;
	  }
	  
	  return PAMI_ERROR;
	}	
	
	static int  dispatch_func (void   * metadata,
				   void   * payload,
				   size_t   bytes,
				   void   * recv_func_parm,
				   void   * cookie) 
	{
	  ManytomanyModel<T_Int, T_Single> *model = (ManytomanyModel<T_Int, T_Single> *) recv_func_parm;
	  M2mHdr *amhdr = (M2mHdr *) metadata;
	  
	  unsigned connid    =  amhdr->connid;
	  M2mRecv *recv =  model->getRecvObject(connid); 

	  //PAMI_assert (model != NULL);
	  //PAMI_assert (model->_recv_func != NULL);
	  
	  if (unlikely(!recv->initialized())) 
	    recv->initialize (model->_ctxt, model->_recv_func, model->_async_arg, connid);
	  
	  MUHWI_PacketHeader_t  * hdr = (MUHWI_PacketHeader_t *) ((char*)cookie - 32);
	  //We only support an alltoall 2GB in size per destination
	  size_t offset = hdr->messageUnitHeader.Packet_Types.Memory_FIFO.Put_Offset_LSB;

	  recv->processPacket (amhdr->srcidx, offset, bytes, payload, model->_ctxt);	  
	  return PAMI_SUCCESS;
	}	  
	
	pami_result_t postManytomany_impl(uint8_t (&state)[m2m_state_bytes],
					  pami_manytomany_t *m2minfo,
					  void              *devinfo)
	  
	{
	  // Implementation
	  //Check size and construct shortm2m msg	  
	  // else
	  // call multipacket alltoall msg
	  Topology *topo = m2minfo->send.participants;	  

	  uint32_t srcidx = topo->rank2Index(__global.mapping.task());
	  InjectAMManytomany<T_Int, T_Single> *msg = new (state) InjectAMManytomany<T_Int, T_Single> 
	    (_mucontext, 
	     topo,
	     (M2MPipeWorkQueueT<T_Int, T_Single> *)m2minfo->send.buffer, 
	     m2minfo->cb_done.function, 
	     m2minfo->cb_done.clientdata,
	     m2minfo->connection_id, 
	     _modeldesc,
	     srcidx,
	     _ctxt);
	  
	  //Advance message
	  bool done = msg->advance();
	  if (!done) {
	    //Post work to generic device
	    pami_work_t *wobj = msg->workobj();
	    PAMI::Device::Generic::GenericThread *work = new (wobj) PAMI::Device::Generic::GenericThread (advance, msg);
	    _mucontext.getProgressDevice()->postThread(work);
	  }
	  
	  return PAMI_SUCCESS;
	}

	static pami_result_t advance (pami_context_t     context,
				      void             * cookie)
	{
	  InjectAMManytomany<T_Int, T_Single> *msg = (InjectAMManytomany<T_Int, T_Single> *) cookie;	  	  
	  if (msg->advance()) 
	    return PAMI_SUCCESS;
	  
	  return PAMI_EAGAIN;
	}
	
      protected:
	MUSPI_DescriptorBase                    _modeldesc;
	MU::Context                           & _mucontext;	
	uint32_t                                _cached_connid;
	uint32_t                                _dispatch;
	M2mRecv                               * _cached_recv;
	pami_dispatch_manytomany_function       _recv_func;
	void                                  * _async_arg;
	pami_client_t                           _client;
	pami_context_t                          _ctxt;
	std::map<int, M2mRecv>                  _recvmap;	
      }; // class ManytomanyModel

      template <typename T_Int, bool T_Single>
      void *ManytomanyModel<T_Int, T_Single>::getRecvFromMap(uint32_t connid) {
	return &_recvmap[connid];
      }

      template <typename T_Int, bool T_Single>
	void ManytomanyModel<T_Int, T_Single>::M2mRecv::initialize (pami_context_t                       ctxt, 	
								    pami_dispatch_manytomany_function    recv_func, 
								    void                               * async_arg, 
								    unsigned                             connid) 
      {
	_init = true;
	_ncomplete = 0;	    
	pami_manytomanybuf_t *rbuf;
	recv_func (ctxt,
		   async_arg,
		   connid,
		   NULL,
		   0,
		   &rbuf,
		   &_cb_done);	  
	_pwq = (M2MPipeWorkQueueT<T_Int, T_Single> *) rbuf->buffer;
      }
      
    };
  };
};
#endif // __components_device_manytomanymodel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
