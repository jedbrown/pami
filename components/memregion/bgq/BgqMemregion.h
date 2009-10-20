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
		   XMI_assert(base!=NULL);

          // Determine the physical address of the source buffer.
          uint32_t rc;
          rc = Kernel_CreateMemoryRegion (&_memregion, base, bytes_in);
          XMI_assert ( rc == 0 );

          _offset = (uint64_t)base - (uint64_t)_memregion.BaseVa;
          fprintf(stderr, "DmaMemregionBgqCnk::createDmaMemregion_impl() .. base = %p, _memregion.BaseVa = %p, _offset = %zd\n", base, _memregion.BaseVa, _offset);

          *bytes_out = bytes_in;

          rc = Kernel_CreateGlobalMemoryRegion (&_memregion, &_globmemregion);
     	   printf("in create, my global va:%p\n",_globmemregion.BaseVa);

          return XMI_SUCCESS;
	
        }

        inline xmi_result_t destroyMemRegion_impl ()
        {
          return XMI_SUCCESS;;
        }

        inline xmi_result_t getInfo_impl (size_t  * bytes,
                                          void   ** base)
        {
		 *bytes = _memregion.Bytes - _offset;
          *base  = (void *)((uint64_t)_memregion.BaseVa + _offset);
          return XMI_SUCCESS;;
        }

        inline void * getBaseVirtualAddress_impl ()
		{
			return (void *)((uint64_t)_memregion.BaseVa + _offset);
		}

        inline uint64_t getBasePhysicalAddress ()
        {
			 return (void *)((uint64_t)_memregion.BasePa + _offset);
        }
		
		  /// \see XMI::CDI::DMA::Memregion::getBaseVirtualAddress
        inline void * getBaseGlobalVirtualAddress ()
        {
          	return (void *)((uint64_t)_globmemregion.BaseVa + _offset);
        }


      private:
		Kernel_MemoryRegion_t _memregion; // Memory region associated with the buffer.
        Kernel_MemoryRegion_t _globmemregion; // Memory region associated with the buffer.
        size_t _offset;

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
