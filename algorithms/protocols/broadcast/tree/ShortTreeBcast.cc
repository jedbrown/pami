/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/tree/ShortTreeBcast.cc
 * \brief ???
 */

#include "util/ccmi_util.h"
#include "algorithms/ccmi.h"
#include "util/ccmi_debug.h"

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "math/math_coremath.h"
#include "./ShortTreeBcast.h"


#define SHM_FILE "/unique-ccmi-shmem-bcast"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      namespace Tree
      {
        ShortTreeBcast::SharedData * ShortTreeBcast::_shared = (ShortTreeBcast::SharedData *)-1;
        unsigned ShortTreeBcast::_minTCoord = 0;
        unsigned ShortTreeBcast::_numPeers  = 0;

        void ShortTreeBcast::init(CCMI::TorusCollectiveMapping * mapping)
        {
          //CCMI_assert( _shared == NULL );
          if(_shared != ((SharedData *)-1)) return; // this should run only once

#if 1
          int fd = -1, rc = -1;

          // allocate shared memory region
          fd = shm_open( SHM_FILE, O_CREAT | O_RDWR, 0600 );
          CCMI_assert( fd != -1 );

          rc = ftruncate( fd, sizeof(SharedData) );
          CCMI_assert( rc != -1 );

          void * loc = NULL;
          loc = mmap( NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
          CCMI_assert( loc != MAP_FAILED );

          _shared = (SharedData *)loc;
#endif

#if 0
          _shared =  (SharedData *)_g_sysdep->memoryManager().scratchpad_dynamic_area_malloc( sizeof(SharedData) );
#endif

          // find out who is the master core (to access the tree)
          unsigned i;
          _minTCoord = (unsigned)-1;

          unsigned* my_coords = mapping->Coords();
          unsigned coords[CCMI_TORUS_NDIMS];
          CCMI_COPY_COORDS(coords, my_coords);

          unsigned tSize = mapping->GetDimLength(CCMI_T_DIM);
          for(i = 0; i < tSize; i++)
          {
            unsigned peerrank = (unsigned)-1;
            coords[CCMI_T_DIM] = i;

            int rc = mapping->Torus2Rank(coords, &peerrank);

            if((rc == XMI_SUCCESS) && (peerrank != (unsigned)-1))
            {
              _numPeers++;
              if(_minTCoord == (unsigned)-1)
              {
                _minTCoord = i;
              }
            }
          }

          CCMI_assert ( _minTCoord < 4 && _numPeers <= 4 );


          TRACE_ADAPTOR((stderr,"<        >Broadcast::Tree::ShortTreeBcast::init "
                         "minTCoord:%d numPeers:%d t:%d srcs[0]:%#.8X srcs[1]:%#.8X "
                         "srcs[2]:%#.8X srcs[3]:%#.8X\n",
                         _minTCoord,
                         _numPeers,
                         mapping->GetCoord(CCMI_T_DIM),
                         (int)_shared->srcs[0],
                         (int)_shared->srcs[1],
                         (int)_shared->srcs[2],
                         (int)_shared->srcs[3]));

        }

      } /* Tree */
    } /* Broadcast */
  } /* Adaptor */
} /* CCMI */
