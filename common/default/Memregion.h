///
/// \file common/default/Memregion.h
/// \brief XMI default memregion implementation.
///
#ifndef __common_default_Memregion_h__
#define __common_default_Memregion_h__

#include <stdlib.h>
#include <string.h>

#include "common/MemregionInterface.h"

namespace XMI
{
  class Memregion : public Interface::Memregion<Memregion>
  {
    public:
      inline Memregion (xmi_context_t context) :
          Interface::Memregion<Memregion> (context),
          _context (context)
      {
      }
      inline xmi_result_t createMemregion (size_t   * bytes_out,
                                           size_t     bytes_in,
                                           void     * base,
                                           uint64_t   options)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t destroyMemregion ()
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

      inline xmi_result_t read_impl (size_t      local_offset,
                                     Memregion * remote_memregion,
                                     size_t      remote_offset,
                                     size_t      bytes)
      {
        return XMI_ERROR;
      }

      inline xmi_result_t write_impl (size_t      local_offset,
                                      Memregion * remote_memregion,
                                      size_t      remote_offset,
                                      size_t      bytes)
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
  }; // end XMI::Memregion
}; // end namespace XMI

#endif
