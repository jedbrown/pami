///
/// \file components/memregion/lapiunix/lapiunixmemregion.h
/// \brief XMI LAPI specific memregion implementation.
///
#ifndef __components_memregion_lapiunix_lapiunixmemregion_h__
#define __components_memregion_lapiunix_lapiunixmemregion_h__

#include <stdlib.h>
#include <string.h>

#define XMI_MEMREGION_CLASS XMI::MemRegion::LAPI

#include "components/memregion/MemRegion.h"

namespace XMI
{
  namespace MemRegion
  {
    class LAPI : public MemRegion<XMI::MemRegion::LAPI>
      {
      public:
        inline LAPI (xmi_context_t context) :
          MemRegion<XMI::MemRegion::LAPI> (context),
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
    }; // end XMI::MemRegion::LAPI
  }; // end namespace MemRegion
}; // end namespace XMI

#endif // __xmi_lapi_lapimemregion_h__
