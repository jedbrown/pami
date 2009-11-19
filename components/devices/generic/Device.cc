/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/generic/Device.cc
 * \brief Generic Device static implementation components
 *
 * A new device/message is added by:
 * - Add #include for device/message
 * - Add device declaration
 * - Add call from init()
 * - Add call from __advanceRecv()
 */
#include "config.h"
#include "sys/xmi.h"
#include "util/common.h"
#include "components/devices/generic/GenericDevice.h"

#include "GenericSubDeviceList.cc"

// All sub-devices must be instantiated here, but are otherwise unused in
// this particular source file (see GenericDevice.h).
//
XMI::Device::ProgressFunctionDev _g_progfunc_dev;
XMI::Device::AtomicBarrierDev _g_lmbarrier_dev;
XMI::Device::WQRingReduceDev _g_wqreduce_dev;
XMI::Device::WQRingBcastDev _g_wqbcast_dev;
XMI::Device::LocalAllreduceWQDevice _g_l_allreducewq_dev;
XMI::Device::LocalBcastWQDevice _g_l_bcastwq_dev;
XMI::Device::LocalReduceWQDevice _g_l_reducewq_dev;

#if defined(__bgp__) and !defined(__bgq__)
#ifdef NOT_YET
XMI::Device::BGP::MemoryBarrierDev _g_mbarrier_dev;
XMI::Device::BGP::LLSCDev _g_llscbarrier_dev;
#endif
XMI::Device::BGP::giDevice _g_gibarrier_dev;

// All the CollectiveNetwork message variations share the same hardware (queue)...
XMI::Device::BGP::CNDevice _g_cncommon_dev;
XMI::Device::BGP::CNAllreduceDevice	_g_cnallreduce_dev(&_g_cncommon_dev);
//XMI::Device::BGP::CNAllreduceShortDevice	_g_cnallreduceshort_dev(&_g_cncommon_dev);
XMI::Device::BGP::CNAllreducePPDevice	_g_cnallreducepp_dev(&_g_cncommon_dev);
XMI::Device::BGP::CNAllreduce2PDevice	_g_cnallreduce2p_dev(&_g_cncommon_dev);
XMI::Device::BGP::CNBroadcastDevice	_g_cnbroadcast_dev(&_g_cncommon_dev);
#endif // __bgp__ and !__bgq__

namespace XMI {
namespace Device {
namespace Generic {

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */
