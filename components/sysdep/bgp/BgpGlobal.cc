/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/sysdep/bgp/BgpGlobal.cc
/// \brief Blue Gene/P Global Object implementation
///

#include "config.h"
#include "BgpGlobal.h"
#include "components/topology/bgp/BgpTopology.h" // need to make static members...

XMI::SysDep::BgpGlobal __global;

XMI::Mapping::BgpMapping *XMI::Topology::BgpTopology::mapping = NULL;
xmi_coord_t XMI::Topology::BgpTopology::my_coords;
