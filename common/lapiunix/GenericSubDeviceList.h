/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __common_lapiunix_GenericSubDeviceList_h__
#define __common_lapiunix_GenericSubDeviceList_h__

// extern XMI::Device::XXX _g_xxx_dev;

static inline void __platform_generic_init(XMI::SysDep &sd,
				XMI::Device::Generic::Device *device) {
}

static inline int __platform_generic_advanceRecv() {
	int events = 0;
	return events;
}

#endif // __common_lapiunix_GenericSubDeviceList_h__
