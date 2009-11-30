/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgp/collective_network/CNDevice.h
 * \brief Collective Network Device interface
 */
#ifndef __components_devices_bgp_collective_network_CNDevice_h__
#define __components_devices_bgp_collective_network_CNDevice_h__

#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/CNPacket.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"

/**
 * \page env_vars Environment Variables
 *
 * - XMI_THREADED_CN -
 *	Bitmask indicating whether Send (1) and/or Recv (2) should
 *	use Comm (helper) Threads.  Note, Comm
 *	threads may not be used in all cases, it will depend on
 *	factors such as run mode, message size, partition size,
 *	data operand, etc.
 *      Possible values:
 *      - 0 - Neither Send nor Recv will use Comm Threads.
 *      - 1 - Only Send will use Comm Threads.
 *      - 2 - Only Recv will use Comm Threads.
 *      - 3 - Both Send and Recv will use Comm Threads.
 *      - Default is 3.
 *
 * - XMI_PERSISTENT_ADVANCE -
 *	Number of cycles to persist in the advance loop waiting for
 *	a (the first) receive packet to arrive.
 *      - Default is a value computed from the partition size
 *        (Collective network depth).
 *
 * - XMI_PERSIST_MAX -
 *	Upper limit on the number of cycles to persist in advance.
 *	This is only used when XMI_PERSISTENT_ADVANCE is computed.
 *	- Default is 5000 cycles.
 *
 * - XMI_PERSIST_MIN -
 *	Lower limit on the number of cycles to persist in advance.
 *	This is only used when XMI_PERSISTENT_ADVANCE is computed.
 *	- Default is 1000 cycles.
 *
 * - XMI_CN_DBLSUM_THRESH -
 *	Number of doubles at which to start using the 2-Pass algorithm.
 *      Special values:
 *      - -1 (minus 1) - Effectively disables the 2-Pass algorithm.
 *      - Default is 2 doubles.
 *
 * - XMI_CN_HELPER_THRESH -
 *	Number of bytes (message size) at which to start using a
 *	helper thread. Ideally this value would be computed based
 *	on network depth and comm thread start-up time.
 *	- Default 16384 bytes.
 *
 * - XMI_CN_VN_DEEP -
 *	Boolean indicating whether to use the "Deep" protocol
 *	for receiving a message in virtual node mode. Currently
 *	not used.
 *      Possible values:
 *      - 0 (false) - The "Deep" protocol is not used.
 *      - 1 (true)  - The "Deep" protocol is used.
 *      - Default is 1.
 */
extern int XMI_THREADED_CN;
extern unsigned XMI_PERSISTENT_ADVANCE;
extern unsigned XMI_PERSIST_MAX;
extern unsigned XMI_PERSIST_MIN;
extern unsigned XMI_CN_DBLSUM_THRESH;
extern unsigned XMI_CN_HELPER_THRESH;
extern int XMI_CN_VN_DEEP;

// This is here to avoid recursive includes
extern "C" size_t _g_num_active_nodes;

namespace XMI {
namespace Device {
namespace BGP {

class CNDevice : public XMI::Device::Generic::CommonQueueSubDevice {
public:
	/**
	 * \brief  A Collective Network device constructor
	 *
	 * \param[in] sd	SysDep object
	 */
	CNDevice() :
	XMI::Device::Generic::CommonQueueSubDevice(),
	_threadRoles(0)
	{
	}

	/**
	 * \brief Tree Device Initialization
	 *
	 * Typically called once, after construction.
	 * Sets up various device parameters for use during
	 * operation. This may include measuring the send-recv
	 * timings for optimizing those parameters.
	 *
	 * All environment variables are sampled at this point.
	 */
	void init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t contextId);

	inline int getMaxThreads() { return _threadRoles; }

private:
	unsigned _threadRoles;
	unsigned __cn_times[2]; // measured cn depth, local and global
}; // class CNDevice

}; // namespace BGP
}; // namespace Device
}; // namespace XMI

#endif /* __components_devices_bgp_cndevice_h__ */
