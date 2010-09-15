/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/mpi/Global.cc
/// \brief Global Object implementation
///

#include "config.h"
#include "Global.h"
#include "Topology.h" // need to make static members...

PAMI::Global __global;

PAMI::Mapping *PAMI::Topology::mapping = NULL;
pami_coord_t PAMI::Topology::my_coords;

PAMI::Memory::MemoryManager *PAMI::Memory::MemoryManager::heap_mm = __global.heap_mm;
PAMI::Memory::MemoryManager *PAMI::Memory::MemoryManager::shared_mm = __global.shared_mm;

#include "components/devices/misc/ProgressFunctionMsg.h"
PAMI::Device::ProgressFunctionDev _g_progfunc_dev;

#include "components/devices/mpi/mpidevice.h"
PAMI::Device::MPIDevice _g_mpi_device;
