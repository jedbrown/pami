/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/lapiunix/Global.cc
/// \brief Global Object implementation
///

#include "Global.h"
#include "Topology.h" // need to make static members...

PAMI::Global __global;

PAMI::Mapping *PAMI::Topology::mapping = NULL;
pami_coord_t PAMI::Topology::my_coords;

PAMI::Memory::MemoryManager *PAMI::Memory::MemoryManager::heap_mm = &__global.heap_mm;

#include "components/devices/misc/ProgressFunctionMsg.h"
PAMI::Device::ProgressFunctionDev _g_progfunc_dev;
