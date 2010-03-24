///
/// \file common/bgq/Memregion.h
/// \brief PAMI BG/Q specific memregion implementation.
///
#ifndef __common_bgq_Memregion_h__
#define __common_bgq_Memregion_h__

#include <stdlib.h>
#include <string.h>

#include <spi/include/kernel/memory.h>

#include "common/MemregionInterface.h"
#include "util/common.h"

namespace PAMI
{
  class Memregion : public Interface::Memregion<Memregion>
  {
    public:
      inline Memregion (pami_context_t context) :
          Interface::Memregion<Memregion> (context),
          _context (context)
      {
      }

      inline pami_result_t createMemregion_impl (size_t   * bytes_out,
                                                size_t     bytes_in,
                                                void     * base,
                                                uint64_t   options)
      {
        PAMI_assert(base != NULL);

        // Determine the physical address of the source buffer.
        uint32_t rc;
        rc = Kernel_CreateMemoryRegion (&_memregion, base, bytes_in);
        PAMI_assert ( rc == 0 );

        _offset = (uint64_t)base - (uint64_t)_memregion.BaseVa;
        fprintf(stderr, "DmaMemregionBgqCnk::createDmaMemregion_impl() .. base = %p, _memregion.BaseVa = %p, _offset = %zd\n", base, _memregion.BaseVa, _offset);

        *bytes_out = bytes_in;

        rc = Kernel_CreateGlobalMemoryRegion (&_memregion, &_globmemregion);
        printf("in create, my global va:%p\n", _globmemregion.BaseVa);

        return PAMI_SUCCESS;

      }

      inline pami_result_t destroyMemregion_impl ()
      {
        return PAMI_SUCCESS;;
      }

      inline pami_result_t getInfo_impl (size_t  * bytes,
                                        void   ** base)
      {
        *bytes = _memregion.Bytes - _offset;
        *base  = (void *)((uint64_t)_memregion.BaseVa + _offset);
        return PAMI_SUCCESS;;
      }

      inline void * getBaseVirtualAddress_impl ()
      {
        return (void *)((uint64_t)_memregion.BaseVa + _offset);
      }

      inline void* getBasePhysicalAddress ()
      {
        return (void *)((uint64_t)_memregion.BasePa + _offset);
      }

      inline void * getBaseGlobalVirtualAddress ()
      {
        return (void *)((uint64_t)_globmemregion.BaseVa + _offset);
      }

      inline pami_result_t read_impl (size_t         local_offset,
                                     Memregion * remote_memregion,
                                     size_t         remote_offset,
                                     size_t         bytes)
      {
        void * remote_vaddr = (void *)((uint64_t)remote_memregion->getBaseGlobalVirtualAddress() + remote_offset);
        void * local_vaddr  = (void *)((uint8_t *)getBaseVirtualAddress() + local_offset);
        memcpy (local_vaddr, remote_vaddr, bytes);
        ppc_msync();

        return PAMI_SUCCESS;
      }

      inline pami_result_t write_impl (size_t         local_offset,
                                      Memregion * remote_memregion,
                                      size_t         remote_offset,
                                      size_t         bytes)
      {
        void * remote_vaddr = (void *)((uint64_t)remote_memregion->getBaseGlobalVirtualAddress() + remote_offset);
        void * local_vaddr  = (void *)((uint8_t *)getBaseVirtualAddress() + local_offset);
        memcpy (remote_vaddr, local_vaddr, bytes);
        ppc_msync();

        return PAMI_SUCCESS;
      }

      inline bool isSharedAddressReadSupported ()
      {
        return shared_address_read_supported;
      }

      inline bool isSharedAddressWriteSupported ()
      {
        return shared_address_write_supported;
      }

      static const bool shared_address_read_supported  = true;
      static const bool shared_address_write_supported = true;

    private:
      Kernel_MemoryRegion_t _memregion; // Memory region associated with the buffer.
      Kernel_MemoryRegion_t _globmemregion; // Memory region associated with the buffer.
      size_t _offset;

      pami_context_t _context;

  }; // end PAMI::Memregion::Memregion
}; // end namespace PAMI

#endif // __components_memregion_bgq_bgqmemregion_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
