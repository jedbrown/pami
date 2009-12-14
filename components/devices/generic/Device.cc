/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/generic/Device.cc
 * \brief Generic Device static implementation components
 *
 * A new device/message is added by:
 * - Add #include for device/message
 * - Add device declaration
 * - Add call from init()
 * - Add call from __advanceRecv()
 */
#include "config.h"
#include "sys/xmi.h"
#include "util/common.h"
#include "components/devices/generic/GenericDevice.h"

#include "GenericSubDeviceList.cc"

// All sub-devices must be instantiated here, but are otherwise unused in
// this particular source file (see GenericDevice.h).
//
XMI::Device::ProgressFunctionDev _g_progfunc_dev;
XMI::Device::AtomicBarrierDev _g_lmbarrier_dev;
XMI::Device::WQRingReduceDev _g_wqreduce_dev;
XMI::Device::WQRingBcastDev _g_wqbcast_dev;
XMI::Device::LocalAllreduceWQDevice _g_l_allreducewq_dev;
XMI::Device::LocalBcastWQDevice _g_l_bcastwq_dev;
XMI::Device::LocalReduceWQDevice _g_l_reducewq_dev;

namespace XMI {
namespace Device {
namespace Generic {

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */
