///
/// \file common/default/Memregion.h
/// \brief PAMI default memregion implementation.
///
#ifndef __common_default_Memregion_h__
#define __common_default_Memregion_h__

#include <stdlib.h>
#include <string.h>

#include "common/MemregionInterface.h"

namespace PAMI
{
  class Memregion : public Interface::Memregion<Memregion>
  {
    public:
      inline Memregion (pami_context_t context) :
          Interface::Memregion<Memregion> (),
          _context (context)
      {
      }
      inline pami_result_t createMemregion (size_t   * bytes_out,
                                           size_t     bytes_in,
                                           void     * base,
                                           uint64_t   options)
      {
        (void)bytes_out;(void)bytes_in;(void)base;(void)options;
        return PAMI_UNIMPL;
      }

      inline pami_result_t destroyMemregion ()
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t getInfo (size_t * bytes,
                                   void ** base)
      {
        (void)bytes;(void)base;
        return PAMI_UNIMPL;

      }

      inline void * getBaseVirtualAddress ()
      {
        return NULL;
      }

      inline pami_result_t read_impl (size_t      local_offset,
                                     Memregion * remote_memregion,
                                     size_t      remote_offset,
                                     size_t      bytes)
      {
        (void)local_offset;(void)remote_memregion;(void)remote_offset;(void)bytes;
        return PAMI_ERROR;
      }

      inline pami_result_t write_impl (size_t      local_offset,
                                      Memregion * remote_memregion,
                                      size_t      remote_offset,
                                      size_t      bytes)
      {
        (void)local_offset;(void)remote_memregion;
        (void)remote_offset;(void)bytes;
        return PAMI_ERROR;
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
      pami_context_t _context;
  }; // end PAMI::Memregion
}; // end namespace PAMI

#endif
