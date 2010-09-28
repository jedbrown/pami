/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/bgp/BgpGlobal.cc
/// \brief Blue Gene/P Global Object implementation
///

#include "config.h"
#include "Global.h"
#include "Topology.h" // need to make static members...

PAMI::Global __global;

PAMI::Mapping *PAMI::Topology::mapping = NULL;
pami_coord_t PAMI::Topology::my_coords;

PAMI::Memory::MemoryManager *PAMI::Memory::MemoryManager::heap_mm;
PAMI::Memory::MemoryManager *PAMI::Memory::MemoryManager::shared_mm;
PAMI::Memory::MemoryManager *PAMI::Memory::MemoryManager::shm_mm;

// These are not in Global.h for a couple reasons.
// One is to avoid poluting the namespace there,
// another is that this object should only be accessed by low-level
// code that is specific to the device, not by general code.
//
#include "components/devices/bgp/global_interrupt/GIBarrierMsg.h"
PAMI::Device::BGP::giDevice _g_gibarrier_dev;

#include "components/devices/misc/ProgressFunctionMsg.h"
PAMI::Device::ProgressFunctionDev _g_progfunc_dev;

#include "components/devices/bgp/collective_network/CNDevice.h"
extern PAMI::Device::BGP::CNDevice _g_cncommon_dev;
#include "components/devices/bgp/collective_network/CNAllreduceMsg.h"
PAMI::Device::BGP::CNAllreduceDevice _g_cnallreduce_dev(&_g_cncommon_dev);
#include "components/devices/bgp/collective_network/CNAllreducePPMsg.h"
PAMI::Device::BGP::CNAllreducePPDevice _g_cnallreducepp_dev(&_g_cncommon_dev);
#include "components/devices/bgp/collective_network/CNAllreduceSum2PMsg.h"
PAMI::Device::BGP::CNAllreduce2PDevice _g_cnallreduce2p_dev(&_g_cncommon_dev);
#include "components/devices/bgp/collective_network/CNBroadcastMsg.h"
PAMI::Device::BGP::CNBroadcastDevice _g_cnbroadcast_dev(&_g_cncommon_dev);
