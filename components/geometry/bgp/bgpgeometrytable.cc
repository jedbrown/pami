///
/// \file components/geometry/bgp/bgpgeometrytable.cc
/// \brief ???
///

#include "sys/xmi.h"
#include <map>



namespace XMI
{
  std::map<unsigned, xmi_geometry_t> geometry_map;
  std::map<unsigned, xmi_geometry_t> cached_geometry;
}
