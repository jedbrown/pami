///
/// \file components/memregion/mpi/mpimemregion.h
/// \brief XMI MPI specific memregion implementation.
///
#ifndef __components_memregion_mpi_mpimemregion_h__
#define __components_memregion_mpi_mpimemregion_h__

#include <stdlib.h>
#include <string.h>

#define XMI_MEMREGION_CLASS XMI::MemRegion::MPI

#include "components/memregion/MemRegion.h"

namespace XMI
{
  namespace MemRegion
  {
    class MPI : public MemRegion<XMI::MemRegion::MPI>
      {
      public:
        inline MPI (xmi_context_t context) :
          MemRegion<XMI::MemRegion::MPI> (context),
          _context (context)
        {
        }
          inline xmi_result_t createMemRegion (size_t   * bytes_out,
                                               size_t     bytes_in,
                                               void     * base,
                                               uint64_t   options)
          {
              return XMI_UNIMPL;
          }

          inline xmi_result_t destroyMemRegion ()
          {
              return XMI_UNIMPL;
          }

          inline xmi_result_t getInfo (size_t * bytes,
                                       void ** base)
          {
              return XMI_UNIMPL;

          }

          inline void * getBaseVirtualAddress ()
          {
              return NULL;
          }



      private:
        xmi_context_t _context;
    }; // end XMI::MemRegion::MPI
  }; // end namespace MemRegion
}; // end namespace XMI

#endif // __xmi_mpi_mpimemregion_h__
