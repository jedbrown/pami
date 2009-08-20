/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file adaptor/geometry/geometry.cc
 * \brief ???
 */

#include "./Geometry.h"
#include "algorithms/executor/Executor.h"
#include "algorithms/protocols/barrier/BarrierFactory.h"

#include <stdlib.h>

unsigned          CCMI::Adaptor::_ccmi_cached_geometry_comm;
CCMI_Geometry_t * CCMI::Adaptor::_ccmi_cached_geometry;

/// \todo DCMF specifics need to split out of ccmi geometry
#warning DCMF specifics need to split out of ccmi geometry

#ifndef ADAPTOR_NO_TORUS
CCMI::Adaptor::Geometry::Geometry (CCMI::TorusCollectiveMapping *mapping, /// \todo should not be torus
#else // MPI
CCMI::Adaptor::Geometry::Geometry (CCMI::CollectiveMapping *mapping,
#endif
				   unsigned *ranks,
				   unsigned nranks,
				   unsigned comm,
				   unsigned numcolors,
				   bool globalcontext):
_allreduce_iteration(0),
_allreduce_async_mode(1),
_numcolors(numcolors),
_asyncBcastPostQueue(NULL),
_asyncBcastUnexpQueue(NULL)
{
  CCMI_assert (nranks > 0);

  _ranks    =  ranks;
  _numranks =  nranks;
  _commid   =  comm;
  _isGlobalContext = globalcontext;

  // binomial and alltoall schedules should work on all geometries
  _isTorus = true;


  /// \todo This assumes GI and Tree are always available if geom ranks==np
  ///       this is true in BGP though
  if(nranks == mapping->size())
  {
    _isGI = true;
    _isTree = true;
  }
  else
  {
    // Tree and GI are not available for geometries with less nodes
    // than booted
    _isTree = false;
    _isGI = false;
  }

  _barrier_exec = NULL;
  _allreduce[0] = NULL;
  _allreduce_storage[0] = NULL;
  _allreduce[1] = NULL;
  _allreduce_storage[1] = NULL;


  for(int i = 0; i<colorsArray; ++i)
  {
    _collective_exec[i] = NULL;
  }

#ifndef ADAPTOR_NO_TORUS
  /// \brief setup the rectangle
  unsigned coords[CCMI_TORUS_NDIMS];
  unsigned min_coords[CCMI_TORUS_NDIMS];
  unsigned max_coords[CCMI_TORUS_NDIMS] = {0};
 
  mapping->Rank2Torus (&(min_coords[0]), ranks[0]);

  _myidx = -1;  
  //Where am I in the list of ranks
  unsigned count = 0;
  for(count = 0; count < nranks; count ++)
    if(_ranks [count] == mapping->rank())
    {
      _myidx = count;
      break;
    }
#endif
    //Used in alltoalls
  _permutation = NULL;

#ifndef ADAPTOR_NO_TORUS
  DCMF_Hardware_t  hw_info;
  DCMF_Hardware (& hw_info);

  for(count = 0; count < nranks; count ++)
  {
    mapping->Rank2Torus(&(coords[0]), ranks[count]);

    if(coords[CCMI_X_DIM] > max_coords[CCMI_X_DIM])
      max_coords[CCMI_X_DIM] = coords[CCMI_X_DIM];
    if(coords[CCMI_Y_DIM] > max_coords[CCMI_Y_DIM])
      max_coords[CCMI_Y_DIM] = coords[CCMI_Y_DIM];
    if(coords[CCMI_Z_DIM] > max_coords[CCMI_Z_DIM])
      max_coords[CCMI_Z_DIM] = coords[CCMI_Z_DIM];
    if(coords[CCMI_T_DIM] > max_coords[CCMI_T_DIM])
      max_coords[CCMI_T_DIM] = coords[CCMI_T_DIM];

    if(coords[CCMI_X_DIM] < min_coords[CCMI_X_DIM])
      min_coords[CCMI_X_DIM] = coords[CCMI_X_DIM];
    if(coords[CCMI_Y_DIM] < min_coords[CCMI_Y_DIM])
      min_coords[CCMI_Y_DIM] = coords[CCMI_Y_DIM];
    if(coords[CCMI_Z_DIM] < min_coords[CCMI_Z_DIM])
      min_coords[CCMI_Z_DIM] = coords[CCMI_Z_DIM];
    if(coords[CCMI_T_DIM] < min_coords[CCMI_T_DIM])
      min_coords[CCMI_T_DIM] = coords[CCMI_T_DIM];
  }

  if(((unsigned)(max_coords[CCMI_X_DIM] - min_coords[CCMI_X_DIM] + 1) *
      (unsigned)(max_coords[CCMI_Y_DIM] - min_coords[CCMI_Y_DIM] + 1) *
      (unsigned)(max_coords[CCMI_Z_DIM] - min_coords[CCMI_Z_DIM] + 1) *
      (unsigned)(max_coords[CCMI_T_DIM] - min_coords[CCMI_T_DIM] + 1)) == nranks)
  {
    _isRectangle = true;

    _rectangle.x0 = min_coords[CCMI_X_DIM];
    _rectangle.y0 = min_coords[CCMI_Y_DIM];
    _rectangle.z0 = min_coords[CCMI_Z_DIM];
    _rectangle.t0 = min_coords[CCMI_T_DIM];

    _rectangle.xs = max_coords[CCMI_X_DIM] - min_coords[CCMI_X_DIM] + 1;
    _rectangle.ys = max_coords[CCMI_Y_DIM] - min_coords[CCMI_Y_DIM] + 1;
    _rectangle.zs = max_coords[CCMI_Z_DIM] - min_coords[CCMI_Z_DIM] + 1;
    _rectangle.ts = max_coords[CCMI_T_DIM] - min_coords[CCMI_T_DIM] + 1;

    _rectangle.isTorusX = (hw_info.xTorus && _rectangle.xs == hw_info.xSize);
    _rectangle.isTorusY = (hw_info.yTorus && _rectangle.ys == hw_info.ySize);
    _rectangle.isTorusZ = (hw_info.zTorus && _rectangle.zs == hw_info.zSize);
    _rectangle.isTorusT = 1;

    //Some protocols cannot make use of torus links. Cache a mesh for them
    _rectangle_mesh = _rectangle;    
    _rectangle_mesh.isTorusX = 0;
    _rectangle_mesh.isTorusY = 0;
    _rectangle_mesh.isTorusZ = 0;
  }
  else
  {
    _isRectangle = false;

    _rectangle.x0  = 0;
    _rectangle.y0  = 0;
    _rectangle.z0  = 0;
    _rectangle.t0  = 0;

    _rectangle.xs  = 0;
    _rectangle.ys  = 0;
    _rectangle.zs  = 0;
    _rectangle.ts  = 0;

    _rectangle.isTorusX = 0;
    _rectangle.isTorusY = 0;
    _rectangle.isTorusZ = 0;
    _rectangle.isTorusT = 0;

    //Some protocols cannot make use of torus links. Cache a mesh for them
    _rectangle_mesh = _rectangle;
  }
#else // ADAPTOR_NO_TORUS
    _isRectangle = false;

#endif  
}

#define PRIME_A   19
#define PRIME_B   23
#define PRIME_C   113
#define PRIME_D   1217

///
/// \brief The above prime numbers generate good permutations and have
/// measured all-to-all performance.
///
void CCMI::Adaptor::Geometry::generatePermutation ()
{
  if(!_permutation)
  {
    //Memory allocation call in the CCMI, we should find a better way to
    //do this
    _permutation =  (unsigned *) CCMI_Alloc (sizeof (int) * _numranks);

    unsigned count = 0;
    unsigned bits = 0;
    int prime_1=0, prime_2=0;
    if((_numranks % PRIME_A) == 0)
    {
      bits |= 1;
    }
    if((_numranks % PRIME_B) == 0)
    {
      bits |= 2;
    }
    if((_numranks % PRIME_C) == 0)
    {
      bits |= 4;
    }
    if((_numranks % PRIME_D) == 0)
    {
      bits |= 8;
    }
    if(!(bits & 3))
    {
      prime_1 = PRIME_A;
      prime_2 = PRIME_B;
    }
    else if(!(bits & 5))
    {
      prime_1 = PRIME_A;
      prime_2 = PRIME_C;
    }
    else if(!(bits & 9))
    {
      prime_1 = PRIME_A;
      prime_2 = PRIME_D;
    }
    else if(!(bits & 6))
    {
      prime_1 = PRIME_B;
      prime_2 = PRIME_C;
    }
    else if(!(bits & 10))
    {
      prime_1 = PRIME_B;
      prime_2 = PRIME_D;
    }
    else if(!(bits & 12))
    {
      prime_1 = PRIME_C;
      prime_2 = PRIME_D;
    }
    else
    {
      //we apparantly have more than 3.16 million cores
      CCMI_abort();
    }
    for(count = 0; count < _numranks; count++)
    {
      _permutation[count] = ((count + 1) * prime_1 + _myidx * prime_2) % _numranks;
    }
  }
}

void CCMI::Adaptor::Geometry::freePermutation ()
{
  if(_permutation)
    CCMI_Free(_permutation);
}
