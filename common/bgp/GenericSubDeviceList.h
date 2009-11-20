/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __common_bgp_GenericSubDeviceList_h__
#define __common_bgp_GenericSubDeviceList_h__

// These are needed in order to use the Collective Network sub-devices.
#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/CNDevice.h"

#ifdef NOT_YET 
#include "components/devices/bgp/memory/MemoryBarrierMsg.h"
#include "components/devices/bgp/memory/LLSCMsg.h"
#endif
#include "components/devices/bgp/global_interrupt/GIBarrierMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceMsg.h"
//#include "components/devices/bgp/collective_network/CNAllreduceShortMsg.h"
#include "components/devices/bgp/collective_network/CNAllreducePPMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceSum2PMsg.h"
#include "components/devices/bgp/collective_network/CNBroadcastMsg.h"

#ifdef NOT_YET 
extern XMI::Device::BGP::MemoryBarrierDev _g_mbarrier_dev;
extern XMI::Device::BGP::LLSCDev _g_llscbarrier_dev;
#endif
extern XMI::Device::BGP::giDevice _g_gibarrier_dev;
extern XMI::Device::BGP::CNAllreduceDevice _g_cnallreduce_dev;
//extern XMI::Device::BGP::CNAllreduceShortDevice _g_cnallreduceshort_dev;
extern XMI::Device::BGP::CNAllreducePPDevice _g_cnallreducepp_dev;
extern XMI::Device::BGP::CNAllreduce2PDevice _g_cnallreduce2p_dev;
extern XMI::Device::BGP::CNBroadcastDevice _g_cnbroadcast_dev;

namespace XMI {
namespace Device {
namespace Generic {

inline void Device::__platform_generic_init(XMI::SysDep &sd,
			XMI::Device::Generic::Device *device) {
#ifdef NOT_YET
	        _g_mbarrier_dev.init(sd, this);
	        _g_llscbarrier_dev.init(sd, this);
#endif
	        _g_gibarrier_dev.init(sd, this);

	        _g_cnallreduce_dev.init(sd, this);
	        //_g_cnallreduceshort_dev.init(sd, this);
	        _g_cnallreducepp_dev.init(sd, this);
	        _g_cnallreduce2p_dev.init(sd, this);
	        _g_cnbroadcast_dev.init(sd, this);
}

inline int Device::__platform_generic_advanceRecv(size_t context) {
	int events = 0;
#ifdef NOT_YET
	events += _g_mbarrier_dev.advanceRecv(__contextId);
	events += _g_llscbarrier_dev.advanceRecv(__contextId);
#endif
	events += _g_gibarrier_dev.advanceRecv(__contextId);

	events += _g_cnallreduce_dev.advanceRecv(__contextId);
	//events += _g_cnallreduceshort_dev.advanceRecv(__contextId);
	events += _g_cnallreducepp_dev.advanceRecv(__contextId);
	events += _g_cnallreduce2p_dev.advanceRecv(__contextId);
	events += _g_cnbroadcast_dev.advanceRecv(__contextId);
	return events;
}

}; // namespace Generic
}; // namespace Device
}; // namespace XMI

#endif // __common_bgp_GenericSubDeviceList_h__
