///
/// \file components/memregion/bgq/BgqMemregion.h
/// \brief XMI BG/Q specific memregion implementation.
///
#ifndef __components_memregion_bgq_BgqMemregion_h__
#define __components_memregion_bgq_BgqMemregion_h__

#include <stdlib.h>
#include <string.h>

#define XMI_MEMREGION_CLASS XMI::MemRegion::BgqMemregion

#include "components/memregion/MemRegion.h"

namespace XMI
{
  namespace MemRegion
  {
    class BgqMemregion : public MemRegion<BgqMemregion>
    {
      public:
        inline BgqMemregion (xmi_context_t context) :
            MemRegion<BgqMemregion> (context),
            _context (context)
        {
        }
        inline xmi_result_t createMemRegion_impl (size_t   * bytes_out,
                                                  size_t     bytes_in,
                                                  void     * base,
                                                  uint64_t   options)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t destroyMemRegion_impl ()
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t getInfo_impl (size_t  * bytes,
                                          void   ** base)
        {
          return XMI_UNIMPL;

        }

        inline void * getBaseVirtualAddress_impl ()
        {
          return NULL;
        }

        inline uint64_t getBasePhysicalAddress ()
        {
          return 0;
        }

      private:

        xmi_context_t _context;

    }; // end XMI::MemRegion::BgqMemregion
  }; // end namespace MemRegion
}; // end namespace XMI

#endif // __components_memregion_bgq_bgqmemregion_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
