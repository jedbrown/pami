/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/socklinux/GenericSubDeviceList.h
 * \brief ???
 */

#ifndef __common_socklinux_GenericSubDeviceList_h__
#define __common_socklinux_GenericSubDeviceList_h__

// extern XMI::Device::XXX _g_xxx_dev;

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
inline void Device::__platform_generic_init(XMI::SysDep &sd) {
}

inline int Device::__platform_generic_advanceRecv() {
	int events = 0;
	return events;
}

}; // namespace Generic
}; // namespace Device
}; // namespace XMI

#endif // __common_udp_GenericSubDeviceList_h__
