///
/// \file algorithms/geometry/GeometryTable.cc
/// \brief ???
///

#include <pami.h>
#include <map>

namespace PAMI
{
  std::map<unsigned, pami_geometry_t> geometry_map;
  std::map<unsigned, pami_geometry_t> cached_geometry;
  namespace CollRegistration
  {
    extern const char BarrierString[]        = "PGAS_Barrier";
    extern const char AllgatherString[]      = "PGAS_Allgather";
    extern const char AllgathervString[]     = "PGAS_Allgatherv";
    extern const char ScatterString[]        = "PGAS_Scatter";
    extern const char ScattervString[]       = "PGAS_Scatterv";
    extern const char AllreduceString[]      = "PGAS_Allreduce";
    extern const char ShortAllreduceString[] = "PGAS_ShortAllreduce";
    extern const char BroadcastString[]      = "PGAS_Broadcast";
  }
}
