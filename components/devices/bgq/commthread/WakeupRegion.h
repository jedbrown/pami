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

#include "sys/xmi.h"
#include "spi/include/l2/atomic.h"
#include "spi/include/kernel/memory.h"

#undef WU_MULTICONTEXT // does WAC region allow separate multiple contexts?

namespace XMI {
namespace Device {
namespace CommThread {

class BgqWakeupRegion {
	// type (size) used for each context. Must be power-of-two.
	typedef uint64_t BgqWakeupRegionBuffer[64]; // probably needs to be larger
public:
	BgqWakeupRegion() :
	_wakeup_region(NULL),
	_wu_region_len(0),
	_wu_memreg(),
#if WU_MULTICONTEXT
	_bytesUsed(NULL)
#else
	_bytesUsed(0)
#endif
	{ }

	~BgqWakeupRegion() { }

	/// \brief Initialize Wakeup Region for client
	///
	/// \param[in] clientid	Client ID being initialized
	/// \param[in] nctx	Number of contexts being created for client
	/// \param[in] mm	MemeryManager
	/// \return	Error code
	///
	inline xmi_result_t init(size_t clientid, size_t nctx, Memory::MemoryManager &mm) {
		int rc;
		size_t mctx = nctx;
		// in order for WAC base/mask values to work, need to ensure alignment
		// is such that power-of-two pairs of (ctx0,mctx) result in viable
		// base/mask values. Also... this is physical address dependent, so
		// does the virtual address even matter?
		while (mctx & (mctx - 1)) ++mctx; // brute force - better way?
		_wu_region_len = mctx * sizeof(*_wakeup_region);
		rc = posix_memalign((void **)&_wakeup_region, _wu_region_len, _wu_region_len);
		if (rc != 0) return XMI_ERROR;
#if WU_MULTICONTEXT
		rc = posix_memalign((void **)&_bytesUsed, 16, mctx * sizeof(*_bytesUsed));
		if (rc != 0) {
			free(_wakeup_region);
			return XMI_ERROR;
		}
		memset(_bytesUsed, 0, mctx * sizeof(*_bytesUsed));
#else
		_bytesUsed = 0;
#endif
		uint32_t krc = Kernel_CreateMemoryRegion(&_wu_memreg, _wakeup_region, _wu_region_len);
		if (krc != 0) {
			free(_wakeup_region);
			_wakeup_region = NULL;
#if WU_MULTICONTEXT
			free(_bytesUsed);
			_bytesUsed = NULL;
#endif
			return XMI_ERROR;
		}
		// assert((_wu_region_len & (_wu_region_len - 1)) == 0); // power of 2
		// assert((_wu_memreg.BasePa & (_wu_region_len - 1)) == 0); // aligned
		return XMI_SUCCESS;
	}

	/// \brief Reserve space in the WAC region
	///
	/// Any change to memory returned will cause a waiting thread to wakeup.
	/// Waiting thread must have used getWURange to program WAC.
	///
	/// must be called from thread-safe code - serialized by caller.
	/// typically called only from init, which is single-threaded.
	///
	/// \param[in] contextid	(not used if WU_MULTICONTEXT)
	/// \param[in] length		Length of memory block desired
	/// \return	Pointer to WAC space.
	///
	inline void *reserveWUSpace(size_t contextid, size_t length) {
		void *v;
		size_t l = length;
		l = (l + 0x07) & ~0x07; // keep uint64_t alignment
#if WU_MULTICONTEXT
		if (_bytesUsed[contextid] + l > _wu_region_len) {
			return NULL;
		}
		v = ((char *)&_wakeup_region[contextid] + _bytesUsed[contextid]);
		_bytesUsed[contextid] += l;
#else
		if (_bytesUsed + l > _wu_region_len) {
			return NULL;
		}
		v = ((char *)&_wakeup_region + _bytesUsed);
		_bytesUsed += l;
#endif
		return v;
	}

	/// \brief Return base phy addr and mask for WAC given context(s)
	///
	/// \param[in] ctx0	(not used if WU_MULTICONTEXT)
	/// \param[in] nctx	(not used if WU_MULTICONTEXT)
	/// \param[out] base	Physical base address of memory block
	/// \param[out] mask	Address bit mask of memory block
	/// \return	Pointer to WAC space.
	///
	inline void getWURange(size_t ctx0, size_t mctx, uint64_t *base, uint64_t *mask) {
#if WU_MULTICONTEXT
		// assert(ctx0 is power-of-two and mctx is power-of-two);
		// these are virtual addresses - WAC needs physical...
		*base = _wu_memreg.BasePa + (ctx0 * sizeof(*_wakeup_region));
		*mask = ~(mctx * sizeof(*_wakeup_region) - 1); // assumes power of two
		// assert((*base & ~*mask) == 0);
#else
		*base = (uint64_t)_wu_memreg.BasePa;
		*mask = ~(_wu_region_len - 1);
#endif
	}
private:
	BgqWakeupRegionBuffer *_wakeup_region;	///< memory for WAC for all contexts
	size_t _wu_region_len;			///< length of total WAC region
	Kernel_MemoryRegion_t _wu_memreg;	///< phy addr of WAC region
#if WU_MULTICONTEXT
	size_t *_bytesUsed;	///< array of per-context space used vars
#else
	size_t _bytesUsed;	///< space used
#endif
}; // class BgqWakeupRegion

}; // namespace CommThread
}; // namespace Device
}; // namespace XMI

#endif // __components_devices_bgq_commthread_WakeupRegion_h__
