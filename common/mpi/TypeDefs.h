/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/mpi/TypeDefs.h
 * \brief ???
 */

#ifndef __common_mpi_TypeDefs_h__
#define __common_mpi_TypeDefs_h__


#include "components/devices/mpi/oldmpimulticastmodel.h"
#include "components/devices/mpi/oldmpim2mmodel.h"
#include "components/devices/mpi/mpimessage.h"
#include "components/devices/mpi/mpidevice.h"
#include "common/mpi/NativeInterface.h"


typedef XMI::Device::MPIOldmulticastModel<XMI::Device::MPIDevice,
                                          XMI::Device::MPIMessage> MPIMcastModel;
typedef TSPColl::NBCollManager<MPIMcastModel> XMI_NBCollManager;

typedef XMI::Device::MPIOldm2mModel<XMI::Device::MPIDevice,
                                    XMI::Device::MPIMessage,
                                    size_t> MPIM2MModel;

typedef XMI::Device::MPIDevice               MPIDevice;


#define XMI_COLL_MCAST_CLASS  MPIMcastModel
#define XMI_COLL_M2M_CLASS    MPIM2MModel
#define XMI_COLL_SYSDEP_CLASS XMI::SysDep
#define XMI_NATIVEINTERFACE   MPINativeInterface

#endif
