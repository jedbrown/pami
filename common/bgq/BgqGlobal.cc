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

XMI::Global __global;

XMI::Mapping *XMI::Topology::mapping = NULL;
xmi_coord_t XMI::Topology::my_coords;

#include "components/devices/misc/AtomicBarrierMsg.h"
XMI::Device::AtomicBarrierRealDev _g_lmbarrier_dev;

#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
XMI::Device::LocalAllreduceWQRealDevice _g_l_allreducewq_dev;

#include "components/devices/workqueue/LocalBcastWQMessage.h"
XMI::Device::LocalAllreduceWQRealDevice _g_l_bcastwq_dev;

#include "components/devices/workqueue/LocalReduceWQMessage.h"
XMI::Device::LocalAllreduceWQRealDevice _g_l_reducewq_dev;

#include "components/devices/workqueue/WQRingBcastMsg.h"
XMI::Device::WQRingBcastRealDev _g_wqbcast_dev;

#include "components/devices/workqueue/WQRingReduceMsg.h"
XMI::Device::WQRingReduceRealDev _g_wqreduce_dev;

#include "components/devices/misc/ProgressFunctionMsg.h"
XMI::Device::ProgressFunctionRealDev _g_progfunc_dev;

extern "C" void __libxmi_dummy     () { abort(); }
extern "C" void __cxa_pure_virtual  () __attribute__((weak, alias("__libxmi_dummy")));
