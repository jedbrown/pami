/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __common_mpi_GenericSubDeviceList_h__
#define __common_mpi_GenericSubDeviceList_h__

#include "components/devices/mpi/MPIBcastMsg.h"

extern XMI::Device::MPIBcastDev _g_mpibcast_dev;

static inline void __platform_generic_init(XMI::SysDep &sd,
				XMI::Device::Generic::Device *device) {
	_g_mpibcast_dev.init(sd, device);
}

static inline int __platform_generic_advanceRecv() {
	int events = 0;

	events += _g_mpibcast_dev.advanceRecv();

	return events;
}

#endif // __common_mpi_GenericSubDeviceList_h__
