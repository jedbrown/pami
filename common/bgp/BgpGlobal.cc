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

XMI::Global __global;

XMI::Mapping *XMI::Topology::mapping = NULL;
xmi_coord_t XMI::Topology::my_coords;

// These are not in Global.h for a couple reasons.
// One is to avoid poluting the namespace there,
// another is that this object should only be accessed by low-level
// code that is specific to the device, not by general code.
//
#include "components/devices/bgp/global_interrupt/GIBarrierMsg.h"
XMI::Device::BGP::giRealDevice _g_gibarrier_dev;

extern "C" void __libxmi_dummy     () { abort(); }
extern "C" void __cxa_pure_virtual  () __attribute__((weak, alias("__libxmi_dummy")));
