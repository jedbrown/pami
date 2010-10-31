///
/// \file algorithms/geometry/GeometryTable.cc
/// \brief ???
///

#include <pami.h>
#include <map>
#include <algorithms/geometry/Algorithm.h>

namespace PAMI
{
  ///
  /// \brief memory allocator for early arrival barrier messages
  //
  MemoryAllocator <sizeof(PAMI::Geometry::UnexpBarrierQueueElement), 16> _ueb_allocator;

  ///
  /// \brief static match queue to store unexpected barrier messages
  ///
  MatchQueue                                          _ueb_queue;
  
}
