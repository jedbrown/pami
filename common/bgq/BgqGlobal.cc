/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/bgq/BgqGlobal.cc
/// \brief Blue Gene/Q Global Object implementation
///

#include "Global.h"
#include "Topology.h" // need to make static members...

PAMI::Global __global;

PAMI::Mapping *PAMI::Topology::mapping = NULL;
pami_coord_t PAMI::Topology::my_coords;

#include "components/devices/misc/AtomicBarrierMsg.h"
PAMI::Device::AtomicBarrierDev _g_lmbarrier_dev;

#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
PAMI::Device::LocalAllreduceWQDevice _g_l_allreducewq_dev;

#include "components/devices/workqueue/LocalBcastWQMessage.h"
PAMI::Device::LocalBcastWQDevice _g_l_bcastwq_dev;

#include "components/devices/workqueue/LocalReduceWQMessage.h"
PAMI::Device::LocalReduceWQDevice _g_l_reducewq_dev;

#include "components/devices/workqueue/WQRingBcastMsg.h"
PAMI::Device::WQRingBcastDev _g_wqbcast_dev;

#include "components/devices/workqueue/WQRingReduceMsg.h"
PAMI::Device::WQRingReduceDev _g_wqreduce_dev;

#include "components/devices/misc/ProgressFunctionMsg.h"
PAMI::Device::ProgressFunctionDev _g_progfunc_dev;

extern "C" void __libpami_dummy     () { abort(); }
extern "C" void __cxa_pure_virtual  () __attribute__((weak, alias("__libpami_dummy")));
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
