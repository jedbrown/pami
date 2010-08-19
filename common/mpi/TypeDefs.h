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
#include "algorithms/geometry/Geometry.h"

typedef PAMI::Device::MPIOldmulticastModel<PAMI::Device::MPIDevice,
                                          PAMI::Device::MPIMessage> MPIMcastModel;
//typedef TSPColl::NBCollManager<MPIMcastModel> PAMI_NBCollManager;

typedef PAMI::Device::MPIOldm2mModel<PAMI::Device::MPIDevice,
                                    PAMI::Device::MPIMessage,
                                    size_t> MPIOldM2MModel;

typedef PAMI::Device::MPIDevice               MPIDevice;


#define PAMI_COLL_MCAST_CLASS  MPIMcastModel
#define PAMI_COLL_M2M_CLASS    MPIOldM2MModel
#define PAMI_NATIVEINTERFACE   MPINativeInterface
#define PAMI_GEOMETRY_CLASS    PAMI::Geometry::Common

#endif
