/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/bgp/GenericSubDeviceList.h
 * \brief ???
 */

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

/// \brief Initialize sub-devices specific to this platform
///
/// Called within the C++ object XMI::Device::Generic::Device being
/// initialized (this == Generic::Device *).
///
/// \param[in] first_global	True if first init call ever
/// \param[in] first_client	True if first init call for current client
/// \param[in] sd		XMI::SysDep object
///
inline void Device::__platform_generic_init(bool first_global, bool first_client,
								XMI::SysDep &sd) {
	if (first_global) {
		// These sub-devices only execute one message at a time,
		// and so there is only one instance of each, globally.
#ifdef NOT_YET
	        _g_mbarrier_dev.init(sd, __generics, __contextId);
	        _g_llscbarrier_dev.init(sd, __generics, __contextId);
#endif
	        _g_gibarrier_dev.init(sd, __generics, __contextId);

	        _g_cnallreduce_dev.init(sd, __generics, __contextId);
	        //_g_cnallreduceshort_dev.init(sd, __generics, __contextId);
	        _g_cnallreducepp_dev.init(sd, __generics, __contextId);
	        _g_cnallreduce2p_dev.init(sd, __generics, __contextId);
	        _g_cnbroadcast_dev.init(sd, __generics, __contextId);
	}
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
