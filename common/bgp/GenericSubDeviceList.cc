/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#include "GenericSubDeviceList.h"

// This gets included in a C++ file so it needs to actually instantiate things...

#ifdef NOT_YET 
XMI::Device::BGP::MemoryBarrierDev _g_mbarrier_dev;
XMI::Device::BGP::LLSCDev _g_llscbarrier_dev;
#endif
XMI::Device::BGP::giDevice _g_gibarrier_dev;

// All the CollectiveNetwork message variations share the same hardware (queue)...
XMI::Device::BGP::CNDevice _g_cncommon_dev;
XMI::Device::BGP::CNAllreduceDevice     _g_cnallreduce_dev(&_g_cncommon_dev);
//XMI::Device::BGP::CNAllreduceShortDevice      _g_cnallreduceshort_dev(&_g_cncommon_dev);
XMI::Device::BGP::CNAllreducePPDevice   _g_cnallreducepp_dev(&_g_cncommon_dev);
XMI::Device::BGP::CNAllreduce2PDevice   _g_cnallreduce2p_dev(&_g_cncommon_dev);
XMI::Device::BGP::CNBroadcastDevice     _g_cnbroadcast_dev(&_g_cncommon_dev);

