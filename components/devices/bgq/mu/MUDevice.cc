/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUDevice.cc
 * \brief ???
 */

#include "components/devices/bgq/mu/MUDevice.h"

#if 0
// #warning Do not use thread local storage .. a device is associated with a context, not a thread.
__thread unsigned   XMI::Device::MU::MUDevice::_p2pSendChannelIndex;
__thread unsigned   XMI::Device::MU::MUDevice::_p2pRecvChannelIndex;
__thread bool       XMI::Device::MU::MUDevice::_colSendChannelFlag;
__thread bool       XMI::Device::MU::MUDevice::_colRecvChannelFlag;
#endif

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

XMI::Device::MU::MUDevice::MUDevice (size_t clientid, size_t ncontexts, size_t contextid) :
    //BaseDevice (),
    Interface::BaseDevice<MUDevice> (),
    Interface::PacketDevice<MUDevice> (),
    sysdep (NULL),
    _contextid (contextid),
    _ncontexts (ncontexts),
    _clientid (clientid),
    _colChannel (NULL),
    _initialized (false)
{
  unsigned i;

  for ( i = 0; i < MAX_NUM_P2P_CHANNELS; i++ ) _p2pChannel[i] = NULL;
};

XMI::Device::MU::MUDevice::~MUDevice() {};

xmi_result_t XMI::Device::MU::MUDevice::init (size_t           clientid,
                                     size_t           contextid,
                                     xmi_client_t     client,
                                     xmi_context_t    context,
                                     SysDep         * sysdep,
                                     XMI::Device::Generic::Device * progress)
{
  int rc = 0;

  this->sysdep  = sysdep;
  _client    = client;
  _context   = context;

  bool isChannelMapped[ MAX_NUM_P2P_CHANNELS ];
  unsigned i;

  for ( i = 0; i < MAX_NUM_P2P_CHANNELS; i++ ) isChannelMapped[i] = 0;

  // find out which channels are mapped for this process

  isChannelMapped[0]    = 1; // \todo asume shm mode and use only a single channel for now
  _firstP2PChannelIndex = 0;
  _numP2PChannels       = 1;
  _p2pSendChannelIndex = _p2pRecvChannelIndex = 0;

  //const int L1D_CACHE_LINE_SIZE = 64;

  // Initialize the dispatch table.
  // There are DISPATCH_SET_COUNT sets of dispatch functions.
  // There are DISPATCH_SET_SIZE  dispatch functions in each dispatch set.
  for ( i = 0; i < DISPATCH_SET_COUNT*DISPATCH_SET_SIZE; i++)
    {
      _dispatch[i].f = noop;
      _dispatch[i].p = (void *) i;
    }

  for ( i = 0; i < MAX_NUM_P2P_CHANNELS; i++ )
    {
      if ( isChannelMapped[i] )
        {
          rc = posix_memalign( (void **) & _p2pChannel[i],
                               L1D_CACHE_LINE_SIZE,
                               sizeof(P2PChannel));

          XMI_assert( rc == 0 );

          new ( _p2pChannel[i] ) P2PChannel();
          rc = _p2pChannel[i]->init( sysdep, _dispatch );
          XMI_assert( rc == 0 );

          if ( rc ) return XMI_ERROR;
        }
    }


  // create the collective channel
#if 0
  rc = posix_memalign( (void **) & _colChannel,
                       L1D_CACHE_LINE_SIZE,
                       sizeof(ColChannel));
  XMI_assert( rc == 0 );

  new ( _colChannel ) ColChannel();
  rc = _colChannel->init( sd );
  XMI_assert( rc == 0 );
#endif
  _colChannel = _p2pChannel[0]; /// \todo temporarily use the p2p channel until resmgr is fixed
#if 0
  TRACE(("MUDEvice ctor p2pChan:%llx resMgr:%llx subgrpPtr:%llx MUsubgrpPtr:%llx\n",
         (unsigned long long)(_p2pChannel[0]),
         (unsigned long long)(&_p2pChannel[0]->_resMgr),
         (unsigned long long)_p2pChannel[0]->_resMgr._injFifoSubGroups[0],
         (unsigned long long)(&_p2pChannel[0]->_resMgr._injFifoSubGroups[0]->_fifoSubGroup)));
#endif
  _initialized = true;

  return XMI_SUCCESS;
};

xmi_context_t XMI::Device::MU::MUDevice::getContext_impl ()
{
  return _context;
}

size_t XMI::Device::MU::MUDevice::getContextOffset_impl ()
{
  return _contextid;
}

bool XMI::Device::MU::MUDevice::isInit_impl()
{
  return _initialized;
};


bool XMI::Device::MU::MUDevice::registerPacketHandler (size_t                      dispatch,
                                                       Interface::RecvFunction_t   function,
                                                       void                      * arg,
                                                       uint16_t                  & id)
{
  TRACE((stderr, ">> MUDevice::registerPacketHandler(%zd, %p, %p), _dispatch = %p\n", dispatch, function, arg, _dispatch));

  // There are DISPATCH_SET_COUNT sets of dispatch functions.
  // There are DISPATCH_SET_SIZE  dispatch functions in each dispatch set.
  if (dispatch >= DISPATCH_SET_COUNT) return false;

  unsigned i;

  for (i = 0; i < DISPATCH_SET_SIZE; i++)
    {
      id = dispatch * DISPATCH_SET_SIZE + i;

      if (_dispatch[id].f == noop)
        {
          _dispatch[id].f = function;
          _dispatch[id].p = arg;

          TRACE((stderr, "<< MUDevice::registerPacketHandler(%zd, %p, %p, %d/%X), i = %d\n", dispatch, function, arg, id, id, i));
          return true;
        }
    }

  // release the lock
  TRACE((stderr, "<< MUDevice::registerPacketHandler(%zd, %p, %p, %d/%X), result = false\n", dispatch, function, arg, id, id));

  return false;
};


/// \see XMI::Device::Interface::RecvFunction_t
int XMI::Device::MU::MUDevice::noop (void   * metadata,
                                     void   * payload,
                                     size_t   bytes,
                                     void   * recv_func_parm,
                                     void   * cookie)
{
  fprintf (stderr, "Error. Dispatch to unregistered id (%zd).\n", (size_t) recv_func_parm);
  XMI_abortf("%s<%d>\n",__FILE__,__LINE__);
  return 0;
};

void XMI::Device::MU::dumpHexData(const char * pstring, const uint32_t *buffer, size_t n_ints)
{
  fprintf(stderr, "dumphex:%s:%p:%zd:\n\n", pstring, &buffer, n_ints);
  unsigned nChunks = n_ints / 8;
  if (!buffer || !n_ints) return;
  for (unsigned i = 0; i < nChunks; i++)
  {
    fprintf(stderr,
            "<%p>: %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X \n",
            buffer+(i*8),
            *(buffer+(i*8)+0),
            *(buffer+(i*8)+1),
            *(buffer+(i*8)+2),
            *(buffer+(i*8)+3),
            *(buffer+(i*8)+4),
            *(buffer+(i*8)+5),
            *(buffer+(i*8)+6),
            *(buffer+(i*8)+7)
           );
  }
  if (n_ints % 8)
  {
    unsigned lastChunk = nChunks * 8;
    fprintf(stderr,
            "<%p>: %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X \n",
            buffer+lastChunk,
            lastChunk+0<n_ints?*(buffer+lastChunk+0):0xDEADDEAD,
            lastChunk+1<n_ints?*(buffer+lastChunk+1):0xDEADDEAD,
            lastChunk+2<n_ints?*(buffer+lastChunk+2):0xDEADDEAD,
            lastChunk+3<n_ints?*(buffer+lastChunk+3):0xDEADDEAD,
            lastChunk+4<n_ints?*(buffer+lastChunk+4):0xDEADDEAD,
            lastChunk+5<n_ints?*(buffer+lastChunk+5):0xDEADDEAD,
            lastChunk+6<n_ints?*(buffer+lastChunk+6):0xDEADDEAD,
            lastChunk+7<n_ints?*(buffer+lastChunk+7):0xDEADDEAD
           );
    lastChunk = 0; // gets rid of an annoying warning when not tracing the buffer
  }
}
void XMI::Device::MU::dumpDescriptor(const char* string, const MUHWI_Descriptor_t *desc)
{
  fprintf(stderr, "dumpDescriptor:%s:%p:\n\n", string, desc);
  fprintf(stderr, "0x%08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n",
          (uint32_t)(*(((uint32_t*)desc) + 0)),
          (uint32_t)(*(((uint32_t*)desc) + 1)),
          (uint32_t)(*(((uint32_t*)desc) + 2)),
          (uint32_t)(*(((uint32_t*)desc) + 3)),
          (uint32_t)(*(((uint32_t*)desc) + 4)),
          (uint32_t)(*(((uint32_t*)desc) + 5)),
          (uint32_t)(*(((uint32_t*)desc) + 6)),
          (uint32_t)(*(((uint32_t*)desc) + 7)),
          (uint32_t)(*(((uint32_t*)desc) + 8)),
          (uint32_t)(*(((uint32_t*)desc) + 9)),
          (uint32_t)(*(((uint32_t*)desc) + 10)),
          (uint32_t)(*(((uint32_t*)desc) + 11)),
          (uint32_t)(*(((uint32_t*)desc) + 12)),
          (uint32_t)(*(((uint32_t*)desc) + 13)),
          (uint32_t)(*(((uint32_t*)desc) + 14)),
          (uint32_t)(*(((uint32_t*)desc) + 15)));

  fprintf(stderr,"desc->Half_Word0.Prefetch_Only             %#X\n", desc->Half_Word0.Prefetch_Only);
  fprintf(stderr,"desc->Half_Word1.Interrupt                 %#X\n", desc->Half_Word1.Interrupt    );
  fprintf(stderr,"desc->Pa_Payload                           %#lX\n", desc->Pa_Payload              );
  fprintf(stderr,"desc->Message_Length                       %#lX\n", desc->Message_Length          );
  fprintf(stderr,"desc->Torus_FIFO_Map                       %#lX\n", desc->Torus_FIFO_Map          );

  MUHWI_CollectiveNetworkHeader_t *chdr = (MUHWI_CollectiveNetworkHeader_t *)& desc->PacketHeader;

  fprintf(stderr,"hdr->Data_Packet_Type                      %#X\n", chdr->Data_Packet_Type                     );
  if (chdr->Data_Packet_Type == MUHWI_COLLECTIVE_DATA_PACKET_TYPE)
  {
    fprintf(stderr,  "COLLECTIVE Data_Packet_Type\n");
    fprintf(stderr,"hdr->Byte1.OpCode                          %#X\n", chdr->Byte1.OpCode                         );
    fprintf(stderr,"hdr->Byte1.Word_Length                     %#X\n", chdr->Byte1.Word_Length                    );
    fprintf(stderr,"hdr->Byte2.Class_Route                     %#X\n", chdr->Byte2.Class_Route                    );
    fprintf(stderr,"hdr->Byte2.Interrupt                       %#X\n", chdr->Byte2.Interrupt                      );
    fprintf(stderr,"hdr->Byte3.Virtual_channel                 %#X\n", chdr->Byte3.Virtual_channel                );
    fprintf(stderr,"hdr->Byte3.Collective_Type                 %#X\n", chdr->Byte3.Collective_Type                );
    fprintf(stderr,"hdr->Destination.Destination.A_Destination %#X\n", chdr->Destination.Destination.A_Destination);
    fprintf(stderr,"hdr->Destination.Destination.B_Destination %#X\n", chdr->Destination.Destination.B_Destination);
    fprintf(stderr,"hdr->Destination.Destination.C_Destination %#X\n", chdr->Destination.Destination.C_Destination);
    fprintf(stderr,"hdr->Destination.Destination.D_Destination %#X\n", chdr->Destination.Destination.D_Destination);
    fprintf(stderr,"hdr->Destination.Destination.E_Destination %#X\n", chdr->Destination.Destination.E_Destination);
    fprintf(stderr,"hdr->Byte8.Packet_Type                     %#X\n", chdr->Byte8.Packet_Type                    );
    fprintf(stderr,"hdr->Byte8.Size                            %#X\n", chdr->Byte8.Size                           );
    fprintf(stderr,"hdr->Injection_Info.Skip                   %#X\n", chdr->Injection_Info.Skip                  );
    if (chdr->Byte8.Packet_Type == MUHWI_PACKET_TYPE_FIFO)
    {
      fprintf(stderr,  "FIFO Packet_Type\n"); 
      MemoryFifoPacketHeader_t * hdr = (MemoryFifoPacketHeader_t *) & desc->PacketHeader;
      fprintf(stderr,"hdr->dev.issingle                          %#X\n", hdr->dev.issingle   );
      //  fprintf(stderr,"hdr->dev.multipkt                          %#X\n", hdr->dev.multipkt   );
      fprintf(stderr,"hdr->dev.singleonly                        %#X\n", hdr->dev.singleonly );
      fprintf(stderr,"hdr->dev.singlepkt                         %#.2X%.2X%.2X%.2X %#.2X%.2X%.2X%.2X %#.2X%.2X%.2X%.2X %#.2X%.2X%.2X%.2X\n", 
              hdr->dev.singlepkt.metadata[0], hdr->dev.singlepkt.metadata[1], hdr->dev.singlepkt.metadata[2], hdr->dev.singlepkt.metadata[3],
              hdr->dev.singlepkt.metadata[4], hdr->dev.singlepkt.metadata[5], hdr->dev.singlepkt.metadata[6], hdr->dev.singlepkt.metadata[7],
              hdr->dev.singlepkt.metadata[8], hdr->dev.singlepkt.metadata[9], hdr->dev.singlepkt.metadata[10], hdr->dev.singlepkt.metadata[11],
              hdr->dev.singlepkt.metadata[12], hdr->dev.singlepkt.metadata[13], hdr->dev.singlepkt.metadata[14], hdr->dev.singlepkt.metadata[15]);
      fprintf(stderr,"hdr->dev.dispatch_id                       %#X\n", hdr->dev.dispatch_id);
    }
    else if (chdr->Byte8.Packet_Type == MUHWI_PACKET_TYPE_PUT) fprintf(stderr,  "PUT Packet_Type\n"); // \todo expand
    else if (chdr->Byte8.Packet_Type == MUHWI_PACKET_TYPE_GET) fprintf(stderr,  "GET Packet_Type\n"); // \todo expand
    else if (chdr->Byte8.Packet_Type == MUHWI_PACKET_TYPE_PACED_GET) fprintf(stderr,  "PACED GET Packet_Type\n"); // \todo expand
    else
      fprintf(stderr,  "unknown Packet_Type\n");
  }
  else if (chdr->Data_Packet_Type == MUHWI_PT2PT_DATA_PACKET_TYPE)
  {
    fprintf(stderr,  "PT2PT Data_Packet_Type\n"); // \todo expand
    MUHWI_Pt2PtNetworkHeader_t *phdr = (MUHWI_Pt2PtNetworkHeader_t *)& desc->PacketHeader;

    fprintf(stderr,"hdr->Hints                                 %#X\n", phdr->Hints                                    );
    fprintf(stderr,"hdr->Byte2.Hint_E_plus                     %#X\n", phdr->Byte2.Hint_E_plus                        );
    fprintf(stderr,"hdr->Byte2.Hint_E_minus                    %#X\n", phdr->Byte2.Hint_E_minus                       );
    fprintf(stderr,"hdr->Byte2.Route_To_IO_Node                %#X\n", phdr->Byte2.Route_To_IO_Node                   );
    fprintf(stderr,"hdr->Byte2.Return_From_IO_Node             %#X\n", phdr->Byte2.Return_From_IO_Node                );
    fprintf(stderr,"hdr->Byte2.Dynamic                         %#X\n", phdr->Byte2.Dynamic                            );
    fprintf(stderr,"hdr->Byte2.Deposit                         %#X\n", phdr->Byte2.Deposit                            );
    fprintf(stderr,"hdr->Byte2.Interrupt                       %#X\n", phdr->Byte2.Interrupt                          );
    fprintf(stderr,"hdr->Byte3.Virtual_channel                 %#X\n", phdr->Byte3.Virtual_channel                    );
    fprintf(stderr,"hdr->Byte3.Zone_Routing_Id                 %#X\n", phdr->Byte3.Zone_Routing_Id                    );
    fprintf(stderr,"hdr->Byte3.Stay_On_Bubble                  %#X\n", phdr->Byte3.Stay_On_Bubble                     );
    fprintf(stderr,"hdr->Destination.Destination.A_Destination %#X\n", phdr->Destination.Destination.A_Destination);
    fprintf(stderr,"hdr->Destination.Destination.B_Destination %#X\n", phdr->Destination.Destination.B_Destination);
    fprintf(stderr,"hdr->Destination.Destination.C_Destination %#X\n", phdr->Destination.Destination.C_Destination);
    fprintf(stderr,"hdr->Destination.Destination.D_Destination %#X\n", phdr->Destination.Destination.D_Destination);
    fprintf(stderr,"hdr->Destination.Destination.E_Destination %#X\n", phdr->Destination.Destination.E_Destination);
    fprintf(stderr,"hdr->Byte8.Packet_Type                     %#X\n", phdr->Byte8.Packet_Type                        );
    fprintf(stderr,"hdr->Byte8.Size                            %#X\n", phdr->Byte8.Size                               );
    fprintf(stderr,"hdr->Injection_Info.Skip                   %#X\n", phdr->Injection_Info.Skip                      );
    if (phdr->Byte8.Packet_Type == MUHWI_PACKET_TYPE_FIFO)
    {
      fprintf(stderr,  "FIFO Packet_Type\n"); 
      MemoryFifoPacketHeader_t * hdr = (MemoryFifoPacketHeader_t *) & desc->PacketHeader;
      fprintf(stderr,"hdr->dev.issingle                          %#X\n", hdr->dev.issingle   );
      //  fprintf(stderr,"hdr->dev.multipkt                          %#X\n", hdr->dev.multipkt   );
      fprintf(stderr,"hdr->dev.singleonly                        %#X\n", hdr->dev.singleonly );
      fprintf(stderr,"hdr->dev.singlepkt                         %#.2X%.2X%.2X%.2X %#.2X%.2X%.2X%.2X %#.2X%.2X%.2X%.2X %#.2X%.2X%.2X%.2X\n", 
              hdr->dev.singlepkt.metadata[0], hdr->dev.singlepkt.metadata[1], hdr->dev.singlepkt.metadata[2], hdr->dev.singlepkt.metadata[3],
              hdr->dev.singlepkt.metadata[4], hdr->dev.singlepkt.metadata[5], hdr->dev.singlepkt.metadata[6], hdr->dev.singlepkt.metadata[7],
              hdr->dev.singlepkt.metadata[8], hdr->dev.singlepkt.metadata[9], hdr->dev.singlepkt.metadata[10], hdr->dev.singlepkt.metadata[11],
              hdr->dev.singlepkt.metadata[12], hdr->dev.singlepkt.metadata[13], hdr->dev.singlepkt.metadata[14], hdr->dev.singlepkt.metadata[15]);
      fprintf(stderr,"hdr->dev.dispatch_id                       %#X\n", hdr->dev.dispatch_id);
    }
    else if (phdr->Byte8.Packet_Type == MUHWI_PACKET_TYPE_PUT) fprintf(stderr,  "PUT Packet_Type\n"); // \todo expand
    else if (phdr->Byte8.Packet_Type == MUHWI_PACKET_TYPE_GET) fprintf(stderr,  "GET Packet_Type\n"); // \todo expand
    else if (phdr->Byte8.Packet_Type == MUHWI_PACKET_TYPE_PACED_GET) fprintf(stderr,  "PACED GET Packet_Type\n"); // \todo expand
    else
      fprintf(stderr,  "unknown Packet_Type\n");
  }
  else
    fprintf(stderr,  "unknown Data_Packet_Type\n");

}


#undef TRACE
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
