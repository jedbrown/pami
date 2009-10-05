/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUDevice.cc
 * \brief ???
 */

#include "components/devices/bgq/mu/MUDevice.h"


//#define TRACE(x)
//#define TRACE(x) printf x

__thread unsigned   XMI::Device::MU::MUDevice::_p2pSendChannelIndex;
__thread unsigned   XMI::Device::MU::MUDevice::_p2pRecvChannelIndex;
__thread bool       XMI::Device::MU::MUDevice::_colSendChannelFlag;
__thread bool       XMI::Device::MU::MUDevice::_colRecvChannelFlag;


#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

XMI::Device::MU::MUDevice::MUDevice () :
  //BaseDevice (),
  Interface::BaseDevice<MUDevice,SysDep::BgqSysDep> (),
  Interface::MessageDevice<MUDevice> (),
  sysdep (NULL),
  _colChannel (NULL),
  _initialized (false),
  _dispatch_num (0)
{
  unsigned i;
  for ( i = 0; i < MAX_NUM_P2P_CHANNELS; i++ ) _p2pChannel[i] = NULL;
};

XMI::Device::MU::MUDevice::~MUDevice() {};

int XMI::Device::MU::MUDevice::init_impl (SysDep::BgqSysDep * sysdep)
{
  int rc = 0;

  this->sysdep  = sysdep;

  bool isChannelMapped[ MAX_NUM_P2P_CHANNELS ];
  unsigned i;
  for ( i = 0; i < MAX_NUM_P2P_CHANNELS; i++ ) isChannelMapped[i] = 0;

  // find out which channels are mapped for this process

  isChannelMapped[0]    = 1; // \todo asume shm mode and use only a single channel for now
  _firstP2PChannelIndex = 0;
  _numP2PChannels       = 1;
  _p2pSendChannelIndex = _p2pRecvChannelIndex = 0;

  //const int L1D_CACHE_LINE_SIZE = 64;

  // Initialze the dispatch table.
  for ( i = 0; i < 256; i++)
  {
    _dispatch[i].f = noop;
    _dispatch[i].p = (void *) i;
  }

  for ( i = 0; i < MAX_NUM_P2P_CHANNELS; i++ )
    {
      if ( isChannelMapped[i] )
        {
          rc = posix_memalign( (void **)&_p2pChannel[i],
                               L1D_CACHE_LINE_SIZE,
                               sizeof(P2PChannel));

          XMI_assert( rc == 0 );

          new ( _p2pChannel[i] ) P2PChannel();
          rc = _p2pChannel[i]->init( sysdep, _dispatch );
          XMI_assert( rc == 0 );
          if ( rc ) return rc;
        }
    }


  // create the collective channel
#if 0
  rc = posix_memalign( (void **)&_colChannel,
                       L1D_CACHE_LINE_SIZE,
                       sizeof(ColChannel));
  XMI_assert( rc == 0 );

  new ( _colChannel ) ColChannel();
  rc = _colChannel->init( sd );
  XMI_assert( rc == 0 );
#endif
  _colChannel = NULL;
#if 0
  TRACE(("MUDEvice ctor p2pChan:%llx resMgr:%llx subgrpPtr:%llx MUsubgrpPtr:%llx\n",
         (unsigned long long)(_p2pChannel[0]),
         (unsigned long long)(&_p2pChannel[0]->_resMgr),
         (unsigned long long)_p2pChannel[0]->_resMgr._injFifoSubGroups[0],
         (unsigned long long)(&_p2pChannel[0]->_resMgr._injFifoSubGroups[0]->_fifoSubGroup)));
#endif
  _initialized = true;

  return rc;
};

bool XMI::Device::MU::MUDevice::isInit_impl()
{
  return _initialized;
};


bool XMI::Device::MU::MUDevice::requiresRead_impl ()
{
  return false;
};

bool XMI::Device::MU::MUDevice::registerPacketHandler (Interface::RecvFunction_t   function,
                                                void                              * arg,
                                                uint8_t                           & id)
{
  TRACE((stderr, ">> MUDevice::registerPacketHandler(%p, %p, %d), _dispatch = %p\n", function, arg, id, _dispatch));
  if (_dispatch_num >= 255) return false;

  // acquire the lock

  id = _dispatch_num;
  _dispatch[_dispatch_num].f = function;
  _dispatch[_dispatch_num].p = arg;

  _dispatch_num++;

  // release the lock
  TRACE((stderr, "<< MUDevice::registerPacketHandler(%p, %p, %d)\n", function, arg, id));

  return true;
};


/// \see XMI::Device::Interface::RecvFunction_t
int XMI::Device::MU::MUDevice::noop (void   * metadata,
                              void   * payload,
                              size_t   bytes,
                              void   * recv_func_parm)
{
  fprintf (stderr, "Error. Dispatch to unregistered id (%zd).\n", (size_t) recv_func_parm);
  XMI_abort();
  return 0;
};

#undef TRACE
