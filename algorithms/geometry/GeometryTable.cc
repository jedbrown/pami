///
/// \file algorithms/geometry/GeometryTable.cc
/// \brief ???
///

#include "sys/pami.h"
#include <map>



namespace PAMI
{
  std::map<unsigned, pami_geometry_t> geometry_map;
  std::map<unsigned, pami_geometry_t> cached_geometry;
}
