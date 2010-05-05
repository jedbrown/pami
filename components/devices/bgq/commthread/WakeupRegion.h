/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file components/devices/bgq/commthread/WakeupRegion.h
/// \brief

#ifndef __components_devices_bgq_commthread_WakeupRegion_h__
#define __components_devices_bgq_commthread_WakeupRegion_h__

#include <pami.h>
#include "spi/include/l2/atomic.h"
#include "spi/include/kernel/memory.h"

#ifndef L1_CACHELINE_SIZE
#define L1_CACHELINE_SIZE	64
#endif // !L1_CACHELINE_SIZE

#ifndef L2_CACHELINE_SIZE
#define L2_CACHELINE_SIZE	128
#endif // !L2_CACHELINE_SIZE

#define WU_CACHELINE_SIZE	L1_CACHELINE_SIZE

namespace PAMI {
namespace Device {
namespace CommThread {

class BgqWakeupRegion {
public:
        // type (size) used for each context. Must be power-of-two.

        BgqWakeupRegion() :
	_wu_mm(),
	_wakeup_region(NULL),
	_wu_region_len(0),
        _wu_memreg()
        { }

        ~BgqWakeupRegion() { }

        /// \brief Initialize Wakeup Region for client
        ///
        /// \param[in] clientid	Client ID being initialized
        /// \param[in] nctx	Number of contexts being created for client
        /// \param[in] mm	L2Atomic/Shmem MemoryManager
        /// \return	Error code
        ///
        inline pami_result_t init(size_t clientid, size_t nctx, Memory::MemoryManager *mm) {
                size_t mctx = nctx;
                // in order for WAC base/mask values to work, need to ensure alignment
                // is such that power-of-two pairs of (ctx0,mctx) result in viable
                // base/mask values. Also... this is physical address dependent, so
                // does the virtual address even matter?
                while (mctx & (mctx - 1)) ++mctx; // brute force - better way?

		void *virt = NULL;
		size_t size = mctx * BGQ_WACREGION_SIZE * sizeof(uint64_t);
		mm->memalign(&virt, size, size);
		if (virt == NULL) {
			return PAMI_ERROR;
		}
                uint32_t krc = Kernel_CreateMemoryRegion(&_wu_memreg, virt, size);
                if (krc != 0) {
                        //mm->free(virt);
                        return PAMI_ERROR;
                }
		_wu_mm.init(virt, size);
		_wakeup_region = virt;
		_wu_region_len = size;

                // assert((_wu_region_len & (_wu_region_len - 1)) == 0); // power of 2
                // assert((_wu_memreg.BasePa & (_wu_region_len - 1)) == 0); // aligned
                return PAMI_SUCCESS;
        }

        /// \brief Return base phy addr and mask for WAC given context(s)
        ///
	/// \todo incorporate memregion into memory manager? also default alignment?
        ///
        /// \param[in] ctx	Bitmap of contexts to get WAC range for (currently not used)
        /// \param[out] base	Physical base address of memory block
        /// \param[out] mask	Address bit mask of memory block
        ///
        inline void getWURange(uint64_t ctx, uint64_t *base, uint64_t *mask) {
                *base = (uint64_t)_wu_memreg.BasePa +
			((char *)_wakeup_region - (char *)_wu_memreg.BaseVa);
                *mask = ~(_wu_region_len - 1);
        }

        inline void touchWURange(uint64_t ctx) {
		volatile uint64_t *addr, *end;
		uint64_t val;

		addr = (volatile uint64_t *)_wakeup_region;
		end = addr + (_wu_region_len - _wu_mm.available()) / sizeof(uint64_t);
		while (addr < end) {
			val = *addr;
			addr = addr + WU_CACHELINE_SIZE / sizeof(uint64_t);
		}
        }

	inline size_t copyWURange(uint64_t *buf) {
		size_t n = _wu_region_len - _wu_mm.available();
		memcpy(buf, _wakeup_region, n);
		return n;
	}

	inline int cmpWURange(uint64_t *buf) {
		return memcmp(buf, _wakeup_region, _wu_region_len - _wu_mm.available());
	}

	PAMI::Memory::MemoryManager _wu_mm;

private:
        typedef uint64_t BgqWakeupRegionBuffer[BGQ_WACREGION_SIZE];

        void *_wakeup_region;	///< memory for WAC for all contexts
        size_t _wu_region_len;			///< length of total WAC region
        Kernel_MemoryRegion_t _wu_memreg;	///< phy addr of WAC region
}; // class BgqWakeupRegion

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

#endif // __components_devices_bgq_commthread_WakeupRegion_h__
