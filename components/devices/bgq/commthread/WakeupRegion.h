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
        inline pami_result_t init(size_t clientid, size_t nctx, size_t me, size_t lsize, Memory::MemoryManager *mm) {
                size_t mctx = nctx;
                // in order for WAC base/mask values to work, need to ensure alignment
                // is such that power-of-two pairs of (ctx0,mctx) result in viable
                // base/mask values. Also... this is physical address dependent, so
                // does the virtual address even matter?
                while (mctx & (mctx - 1)) ++mctx; // brute force - better way?

		void *virt = NULL;
		size_t esize = mctx * BGQ_WACREGION_SIZE * sizeof(uint64_t);
		size_t size = lsize * esize;
		mm->memalign(&virt, size, size);
		if (virt == NULL) {
			return PAMI_ERROR;
		}
                uint32_t krc = Kernel_CreateMemoryRegion(&_wu_memreg, virt, size);
                if (krc != 0) {
                        //mm->free(virt);
                        return PAMI_ERROR;
                }
		char *v = (char *)virt;
		size_t i;
		for (i = 0; i < lsize; ++i) {
			_wu_mm[i].init(v, esize);
			v += esize;
		}
		_wu_region_me = me;

                // PAMI_assert((size & (size - 1)) == 0); // power of 2
                // PAMI_assert((_wu_memreg.BasePa & (size - 1)) == 0); // aligned
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
			((char *)_wu_mm[_wu_region_me].base() - (char *)_wu_memreg.BaseVa);
                *mask = ~(_wu_mm[_wu_region_me].size() - 1);
        }

	inline PAMI::Memory::MemoryManager *getWUmm(size_t process = (size_t)-1) {
		if (process == (size_t)-1) process = _wu_region_me;
		return &_wu_mm[process];
	}

	inline PAMI::Memory::MemoryManager *getAllWUmm() {
		return &_wu_mm[0];
	}

private:
        typedef uint64_t BgqWakeupRegionBuffer[BGQ_WACREGION_SIZE];

	PAMI::Memory::MemoryManager _wu_mm[PAMI_MAX_PROC_PER_NODE];
	size_t _wu_region_me;	///< local process index into WAC regions
        Kernel_MemoryRegion_t _wu_memreg;	///< phy addr of WAC region
}; // class BgqWakeupRegion

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

#endif // __components_devices_bgq_commthread_WakeupRegion_h__
