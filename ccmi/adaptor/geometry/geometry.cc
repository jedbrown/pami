
#include "./Geometry.h"
#include "../../interfaces/Executor.h"
#include "../protocols/barrier/BarrierFactory.h"

#include <stdlib.h>

unsigned          CCMI::Adaptor::Geometry::_ccmi_cached_geometry_comm;
CCMI_Geometry_t * CCMI::Adaptor::Geometry::_ccmi_cached_geometry;

CCMI::Adaptor::Geometry::Geometry (CCMI::Mapping *mapping,
				   unsigned *ranks,
				   unsigned nranks,
				   unsigned comm,
				   bool globalcontext):
_asyncBcastPostQueue(NULL),
_asyncBcastUnexpQueue(NULL)
{
  CCMI_assert (nranks > 0);

  _ranks    =  ranks;
  _numranks =  nranks;
  _commid   =  comm;
  _isGlobalContext = globalcontext;

  _barrier_exec = NULL;

  _myidx = -1;  
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
    _permutation =  (unsigned *) malloc (sizeof (int) * _numranks);

    unsigned count = 0;
    unsigned bits = 0;
    int prime_1, prime_2;
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
      CCMI_assert (0);
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
    free(_permutation);
}
