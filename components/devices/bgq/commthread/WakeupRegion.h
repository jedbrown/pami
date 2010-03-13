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

#warning Need Kernel_VirtualToPhysical
#define vtop(v)		((uint64_t)v)

namespace XMI {
namespace Device {
namespace CommThread {

class BgqWakeupRegion {
	typedef uint64_t BgqWakeupRegionBuffer[64]; // probably needs to be larger
public:
	BgqWakeupRegion() :
	_wakeup_region(NULL)
	{ }

	~BgqWakeupRegion() { }

	inline xmi_result_t init(size_t clientid, size_t nctx, Memory::MemoryManager &mm) {
		int rc;
		size_t mctx = nctx;
		// in order for WAC base/mask values to work, need to ensure alignment
		// is such that power-of-two pairs of (ctx0,mctx) result in viable
		// base/mask values. Also... this is physical address dependent, so
		// does the virtual address even matter?
		while (mctx & (mctx - 1)) ++mctx; // brute force - better way?
		size_t s = mctx * sizeof(*_wakeup_region);
		rc = posix_memalign((void **)&_wakeup_region, s, s);
		if (rc != 0) return XMI_ERROR;
		rc = posix_memalign((void **)&_bytesUsed, 16, mctx * sizeof(*_bytesUsed));
		if (rc != 0) return XMI_ERROR;
		memset(_bytesUsed, 0, mctx * sizeof(*_bytesUsed));
		return XMI_SUCCESS;
	}

	// must be called from thread-safe code - serialized by caller.
	inline void *reserveWUSpace(size_t contextid, size_t length) {
		void *v;
		size_t l = length;
		l = (l + 0x07) & ~0x07; // keep uint64_t alignment
		if (_bytesUsed[contextid] + l > sizeof(*_wakeup_region)) {
			return NULL;
		}
		v = ((char *)&_wakeup_region[contextid] + _bytesUsed[contextid]);
		_bytesUsed[contextid] += l;
		return v;
	}

	inline void getWURange(size_t ctx0, size_t mctx, uint64_t *base, uint64_t *mask) {
		// assert(ctx0 is power-of-two and mctx is power-of-two);
		// these are virtual addresses - WAC needs physical...
		*base = vtop(&_wakeup_region[ctx0]);
		*mask = ~(mctx * sizeof(*_wakeup_region) - 1); // assumes power of two
		// assert((*base & ~*mask) == 0);
	}
private:
	BgqWakeupRegionBuffer *_wakeup_region;	///< memory for WAC for all contexts
	size_t *_bytesUsed; ///< array of per-context space used vars
}; // class BgqWakeupRegion

}; // namespace CommThread
}; // namespace Device
}; // namespace XMI

#endif // __components_devices_bgq_commthread_WakeupRegion_h__
