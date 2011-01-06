
#ifndef  __components_devices_bgq_mu2_ammulticast__
#define  __components_devices_bgq_mu2_ammulticast__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/MulticastModel.h"
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/msg/InjectAMMulticast.h"

namespace PAMI {
  namespace Device {
    namespace MU {

      class ShortAMMulticastModel: public Interface::AMMulticastModel<ShortAMMulticastModel, MU::Context, sizeof(InjectAMMulticast)>
      {
      protected:
	struct AMMulticastHdr {
	  uint32_t     root;
	  uint16_t     bytes;        /*As this is a short protocol, bytes is 16 bit*/
	  uint16_t     metasize;
	  uint64_t     connection_id; /* Permit a 64 bit connection id */
	};
      public:

	static const unsigned sizeof_msg = sizeof(InjectAMMulticast);

	ShortAMMulticastModel (MU::Context    & context,
			  pami_result_t  & status):
	Interface::AMMulticastModel<ShortAMMulticastModel, MU::Context, sizeof(InjectAMMulticast)> (context, status),
	_context(context)
	  {
	    status = init();
	  }
       
	~ShortAMMulticastModel () {}

	pami_result_t init () 
	{
	  // Zero-out the descriptor models before initialization
	  memset((void *)&_singlepkt, 0, sizeof(_singlepkt));
	  
	  // --------------------------------------------------------------------
	  // Set the common base descriptor fields
	  // --------------------------------------------------------------------
	  MUSPI_BaseDescriptorInfoFields_t base;
	  memset((void *)&base, 0, sizeof(base));

	  base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
	  base.Payload_Address = 0;
	  base.Message_Length  = 0;
	  base.Torus_FIFO_Map  = 0;
	  base.Dest.Destination.Destination = 0;

	  _singlepkt.setBaseFields (&base);
	  
	  // --------------------------------------------------------------------
	  // Set the common point-to-point descriptor fields
	  // --------------------------------------------------------------------
	  MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
	  memset((void *)&pt2pt, 0, sizeof(pt2pt));
	  
	  pt2pt.Hints_ABCD = 0;
	  pt2pt.Skip       = 0;
	  pt2pt.Misc1 =
	    MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
	    MUHWI_PACKET_DO_NOT_DEPOSIT |
	    MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
	  pt2pt.Misc2 =
	    MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;
	  	 
	  _singlepkt.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
	  _singlepkt.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
	  _singlepkt.setPt2PtFields (&pt2pt);
	
	  // --------------------------------------------------------------------
	  // Set the common memory fifo descriptor fields
	  // --------------------------------------------------------------------
	  MUSPI_MemoryFIFODescriptorInfoFields_t memfifo;
	  memset ((void *)&memfifo, 0, sizeof(memfifo));
	  
	  memfifo.Rec_FIFO_Id    = 0;
	  memfifo.Rec_Put_Offset = 0;
	  memfifo.Interrupt      = 0; //MUHWI_DESCRIPTOR_INTERRUPT_ON_PACKET_ARRIVAL;
	  memfifo.SoftwareBit    = 0;
	  
	  _singlepkt.setMemoryFIFOFields (&memfifo);
	  _singlepkt.setMessageUnitPacketType (MUHWI_PACKET_TYPE_FIFO);

	  // --------------------------------------------------------------------
	  // Set the network header information in the descriptor models to
	  // differentiate between a single-packet transfer and a multi-packet
	  // transfer
	  // --------------------------------------------------------------------
	  MemoryFifoPacketHeader * hdr = NULL;
	  
	  hdr = (MemoryFifoPacketHeader *) & _singlepkt.PacketHeader;
	  hdr->setSinglePacket (true);	  	  	  	  

	  return PAMI_SUCCESS;
	}
	
	static int  dispatch_func (void   * metadata,
				   void   * payload,
				   size_t   bytes,
				   void   * recv_func_parm,
				   void   * cookie) 
	{	  
	  ShortAMMulticastModel *model = (ShortAMMulticastModel *) recv_func_parm;
	  AMMulticastHdr *amhdr = (AMMulticastHdr *) metadata;

	  PAMI_Callback_t         cb_done = {0};
	  size_t                  rcvlen;
	  pami_pipeworkqueue_t  * pwqptr;
	  
	  PAMI_assert (model != NULL);
	  PAMI_assert (model->_recv_func != NULL);

	  model->_recv_func (NULL,
			     (const pami_quad_t *) payload,
			     amhdr->metasize,
			     amhdr->connection_id,
			     amhdr->root,
			     amhdr->bytes,
			     model->_async_arg,
			     &rcvlen,
			     &pwqptr,
			     &cb_done);
	  
	  if (rcvlen > 0) {
	    PAMI_assert (pwqptr != NULL);
	    PAMI_assert (rcvlen <= amhdr->bytes);

	    PipeWorkQueue *pwq = (PipeWorkQueue *)pwqptr;
	    char *buf = pwq->bufferToProduce();
	    memcpy(buf, (char *)payload + amhdr->metasize*sizeof(pami_quad_t), rcvlen);
	    pwq->produceBytes(rcvlen);
	  }

	  if (cb_done.function)
	    cb_done.function(NULL, cb_done.clientdata, PAMI_SUCCESS);

	  return PAMI_SUCCESS;
	}


	void prepareDesc (MUSPI_DescriptorBase     * desc,
			  uint64_t                   paddr,
			  unsigned                   bytes,
			  unsigned                   metasize,
			  unsigned                   connection_id) 
	{
	  _singlepkt.clone(*desc);
	  // Set the payload information.
	  desc->setPayload (paddr, metasize*sizeof(pami_quad_t) + bytes);

	  AMMulticastHdr amhdr;
	  amhdr.root    = __global.mapping.task();
	  amhdr.bytes   = bytes;
	  amhdr.metasize = metasize;
	  amhdr.connection_id = connection_id;

	  MemoryFifoPacketHeader *hdr = (MemoryFifoPacketHeader*) &desc->PacketHeader;
	  hdr->setMetaData(&amhdr, sizeof(amhdr));	  
	}
	
	pami_result_t postImmediate (pami_task_t            * ranks,
				     size_t                   nranks,
				     char                   * src,
				     size_t                   bytes,
				     const pami_quad_t      * metadata,
				     size_t                   metasize,
				     pami_event_function      cb_done,
				     void                   * cookie,
				     unsigned                 connection_id) 
	{	  
	  if (metasize*sizeof(pami_quad_t) + bytes > MU::Context::immediate_payload_size)
	    return PAMI_ERROR;
	  
	  PAMI_assert (nranks > 0);	  	  
	  InjChannel & channel = _context.injectionGroup.channel[0];
	  size_t ndesc = channel.getFreeDescriptorCountWithUpdate ();
	  
	  if (ndesc < nranks)
	    return PAMI_ERROR;
	  
	  MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *)channel.getNextDescriptor ();	  
	  void * vaddr;
	  uint64_t paddr;	  
	  //get the payload of the last descriptor
	  channel.getDescriptorPayload (desc + nranks - 1, vaddr, paddr);	  	  
	  prepareDesc (desc, paddr, bytes, metasize, connection_id);
	  if (metasize > 0)
	    memcpy (vaddr, metadata, metasize * sizeof(pami_quad_t));
	  if (bytes > 0) 
	    memcpy ((char*)vaddr + metasize*sizeof(pami_quad_t), src, bytes);	  

	  size_t cidx = 0;
	  MUHWI_Destination_t   dest;
	  uint16_t              rfifo;
	  uint64_t              map;
	  _context.pinFifo (ranks[0],
			    0,
			    dest,
			    rfifo,
			    map);	  
	  
	  // Initialize the injection fifo descriptor in-place.
	  desc->setDestination (dest);
	  desc->setRecFIFOId (rfifo);
	  desc->setTorusInjectionFIFOMap (map);
	  fprintf(stderr, "Sending msg from payload pa %lx\n", paddr);
	  MUSPI_DescriptorDumpHex ((char *)"Immediate Multicast", desc);
	  
	  for (cidx = 1; cidx < nranks; cidx++) {
	    MUSPI_DescriptorBase * memfifo = desc + cidx;
	    _context.pinFifo (ranks[cidx],
			      0,
			      dest,
			      rfifo,
			      map);
	    
	    desc->clone(*memfifo);

            // Initialize the injection fifo descriptor in-place.
	    memfifo->setDestination (dest);
            memfifo->setRecFIFOId (rfifo);
	    memfifo->setTorusInjectionFIFOMap (map);
	    MUSPI_DescriptorDumpHex ((char *)"Immediate Multicast", memfifo);
	  } 
	  
	  //Advance all the descriptors
	  channel.injFifoAdvanceDescMultiple (nranks);

	  if (cb_done)
	    cb_done (NULL, cookie, PAMI_SUCCESS);
	  
	  return PAMI_SUCCESS;
	}

	pami_result_t registerMcastRecvFunction (int                         dispatch_id,
						 pami_dispatch_multicast_function recv_func,
						 void                       *async_arg) 
	{
	  _dispatch        = dispatch_id;
	  _recv_func       = recv_func;
	  _async_arg       = _async_arg;
	  
	  uint16_t id = 0;
	  if (_context.registerPacketHandler (_dispatch,
					      dispatch_func,
					      this,
					      id))
	    {
	      MemoryFifoPacketHeader * hdr = NULL;	      
	      hdr = (MemoryFifoPacketHeader *) & _singlepkt.PacketHeader;
	      hdr->setDispatchId (id);	 
	      return PAMI_SUCCESS;
	    }
	  
	  fprintf (stderr, "Dispatch Failed\n");
	  
	  return PAMI_ERROR;
	}
	
	pami_result_t postMulticast_impl(uint8_t               (&state)[sizeof_msg],
					 pami_multicast_t    * mcast,
					 void                * devinfo=NULL) 
	{
	  pami_task_t *ranks = NULL;
	  Topology *dst_topology = (Topology *)mcast->dst_participants;
	  dst_topology->rankList(&ranks);
	  size_t nranks = dst_topology->size();
	  
	  //PAMI_assert(ranks[0] == __global.mapping.task());	  
	  PipeWorkQueue *spwq = (PipeWorkQueue *) mcast->src;
	  char *src = spwq->bufferToConsume();
	  uint32_t sbytes = spwq->bytesAvailableToConsume();
	  
	  if (sbytes == mcast->bytes) {
	    int rc = postImmediate (ranks,
				    nranks,
				    src,
				    mcast->bytes,
				    mcast->msginfo,
				    mcast->msgcount,
				    mcast->cb_done.function,	       
				    mcast->cb_done.clientdata,
				    mcast->connection_id);
	    
	    if (rc == PAMI_SUCCESS)
	      return PAMI_SUCCESS;
	  }

	  if (mcast->msgcount*sizeof(pami_quad_t) + mcast->bytes > MU::Context::packet_payload_size)
	    return PAMI_ERROR;

	  fprintf (stderr, "Long Multicast\n");

	  InjectAMMulticast *msg = new (state) InjectAMMulticast (_context,
								  _context.injectionGroup.channel[0],
								  ranks,
								  nranks,
								  spwq,
								  mcast->bytes,
								  mcast->msgcount,
								  mcast->cb_done.function,	       
								  mcast->cb_done.clientdata);

	  MUSPI_DescriptorBase *desc = &msg->_desc;
	  Kernel_MemoryRegion_t memRegion;
	  int rc = Kernel_CreateMemoryRegion (&memRegion, msg->packetBuf(), mcast->bytes);
	  PAMI_assert ( rc == 0 );
	  uint64_t paddr = (uint64_t)memRegion.BasePa + ((uint64_t)src - (uint64_t)memRegion.BaseVa);

	  prepareDesc (desc,  paddr, mcast->bytes, mcast->msgcount, mcast->connection_id);
	  if (mcast->msgcount > 0)
	    memcpy (msg->packetBuf(), mcast->msginfo, mcast->msgcount * sizeof(pami_quad_t));

	  bool done = msg->advance();

	  if (!done) {
	    PAMI::Device::Generic::GenericThread *work = new (msg->workobj()) 
	      PAMI::Device::Generic::GenericThread(advance, msg);
	    _context.getProgressDevice()->postThread(work);
	  }

	  return PAMI_SUCCESS;
	}
	
	static pami_result_t advance (pami_context_t     context,
				      void             * cookie) 
	{
	  InjectAMMulticast *msg = (InjectAMMulticast *) cookie;
	  bool done = msg->advance();

	  if (done) return PAMI_SUCCESS;	      
	  return PAMI_EAGAIN;
	}

      protected:
	MUSPI_DescriptorBase                    _singlepkt;
	MU::Context                           & _context;	
	pami_dispatch_multicast_function        _recv_func;
	void                                  * _async_arg;
	uint32_t                                _dispatch;
      };
    };
  };
};
#endif
