///
/// \file components/memregion/NoopMemregion.h
/// \brief XMI "noop" memregion implementation.
///
#ifndef __components_memregion_noopmemregion_h__
#define __components_memregion_noopmemregion_h__

#include <stdlib.h>
#include <string.h>

#define XMI_MEMREGION_CLASS XMI::MemRegion::Noop

#include "components/memregion/MemRegion.h"

namespace XMI
{
  namespace MemRegion
  {
    class Noop : public MemRegion<Noop>
    {
      public:
        inline Noop (xmi_context_t context) :
            MemRegion<Noop> (context),
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

        inline xmi_result_t read_impl (size_t   local_offset,
                                       Noop   * remote_memregion,
                                       size_t   remote_offset,
                                       size_t   bytes)
        {
          return XMI_ERROR;
        }

        inline xmi_result_t write_impl (size_t   local_offset,
                                        Noop   * remote_memregion,
                                        size_t   remote_offset,
                                        size_t   bytes)
        {
          return XMI_ERROR;
        }

        inline bool isSharedAddressReadSupported ()
        {
          return shared_address_read_supported;
        }

        inline bool isSharedAddressWriteSupported ()
        {
          return shared_address_write_supported;
        }

        static const bool shared_address_read_supported  = false;
        static const bool shared_address_write_supported = false;

      private:
        xmi_context_t _context;
    }; // end XMI::MemRegion::Noop
  }; // end namespace MemRegion
}; // end namespace XMI

#endif // __components_memregion_noopmemregion_h__
