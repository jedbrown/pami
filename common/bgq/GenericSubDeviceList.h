/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __common_bgq_GenericSubDeviceList_h__
#define __common_bgq_GenericSubDeviceList_h__

// extern XMI::Device::XXX _g_xxx_dev;

namespace XMI {
namespace Device {
namespace Generic {

inline void Device::__platform_generic_init(XMI::SysDep &sd,
			XMI::Device::Generic::Device *device) {
}

inline int Device::__platform_generic_advanceRecv(size_t context) {
	int events = 0;
	return events;
}

}; // namespace Generic
}; // namespace Device
}; // namespace XMI

#endif // __common_bgq_GenericSubDeviceList_h__
