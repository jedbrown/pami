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

#include "config.h"
#include "Global.h"
#include "Topology.h" // need to make static members...

XMI::Global __global;

XMI::Mapping *XMI::Topology::mapping = NULL;
xmi_coord_t XMI::Topology::my_coords;

#include "components/devices/misc/AtomicBarrierMsg.h"
XMI::Device::AtomicBarrierDev _g_lmbarrier_dev;

#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
XMI::Device::LocalAllreduceWQDevice _g_l_allreducewq_dev;

#include "components/devices/workqueue/LocalBcastWQMessage.h"
XMI::Device::LocalBcastWQDevice _g_l_bcastwq_dev;

#include "components/devices/workqueue/LocalReduceWQMessage.h"
XMI::Device::LocalReduceWQDevice _g_l_reducewq_dev;

#include "components/devices/workqueue/WQRingBcastMsg.h"
XMI::Device::WQRingBcastDev _g_wqbcast_dev;

#include "components/devices/workqueue/WQRingReduceMsg.h"
XMI::Device::WQRingReduceDev _g_wqreduce_dev;

#include "components/devices/misc/ProgressFunctionMsg.h"
XMI::Device::ProgressFunctionDev _g_progfunc_dev;
