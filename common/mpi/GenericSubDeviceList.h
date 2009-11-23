/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/mpi/GenericSubDeviceList.h
 * \brief ???
 */

#ifndef __common_mpi_GenericSubDeviceList_h__
#define __common_mpi_GenericSubDeviceList_h__

#include "components/devices/mpi/MPIBcastMsg.h"

extern XMI::Device::MPIBcastDev _g_mpibcast_dev;

namespace XMI {
namespace Device {
namespace Generic {

/// \brief Initialize sub-devices specific to this platform
///
/// Called within the C++ object XMI::Device::Generic::Device being
/// initialized (this == Generic::Device *).
///
/// \param[in] first_global     True if first init call ever
/// \param[in] first_client     True if first init call for current client
/// \param[in] sd               XMI::SysDep object
///
inline void Device::__platform_generic_init(bool first_global, bool first_client,
								XMI::SysDep &sd) {
	if (first_global) {
		// These sub-devices only execute one message at a time,
		// and so there is only one instance of each, globally.
		_g_mpibcast_dev.init(sd, __generics, __contextId);
	}
}

inline int Device::__platform_generic_advanceRecv(size_t context) {
	int events = 0;
	events += _g_mpibcast_dev.advanceRecv(context);
	return events;
}

}; // namespace Generic
}; // namespace Device
}; // namespace XMI

#endif // __common_mpi_GenericSubDeviceList_h__
