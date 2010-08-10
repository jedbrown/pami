///
/// \file algorithms/geometry/GeometryTable.cc
/// \brief ???
///

#include <pami.h>
#include <map>
#include <algorithms/geometry/Algorithm.h>

namespace PAMI
{
  std::map<unsigned, pami_geometry_t> geometry_map;
  std::map<unsigned, pami_geometry_t> cached_geometry;
  
  ///
  /// \brief memory allocator for early arrival barrier messages
  //
  MemoryAllocator <sizeof(PAMI::Geometry::UnexpBarrierQueueElement), 16> _ueb_allocator;
  
  ///
  /// \brief static match queue to store unexpected barrier messages
  ///
  MatchQueue                                          _ueb_queue;
  
  namespace CollRegistration
  {
    extern const char BarrierString[]        = "PGAS_Barrier";
    extern const char BarrierUEString[]      = "PGAS_UEBarrier";
    extern const char AllgatherString[]      = "PGAS_Allgather";
    extern const char AllgathervString[]     = "PGAS_Allgatherv";
    extern const char ScatterString[]        = "PGAS_Scatter";
    extern const char ScattervString[]       = "PGAS_Scatterv";
    extern const char AllreduceString[]      = "PGAS_Allreduce";
    extern const char ShortAllreduceString[] = "PGAS_ShortAllreduce";
    extern const char BroadcastString[]      = "PGAS_Broadcast";
  }
}
