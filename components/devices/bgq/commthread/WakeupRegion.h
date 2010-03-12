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

	inline xmi_result_t init(size_t clientid, size_t num_ctx, Memory::MemoryManager &mm) {
		int rc;
		// alignment requires something else...
		rc = posix_memalign((void **)&_wakeup_region, 16, num_ctx * sizeof(*_wakeup_region));
		if (rc != 0) return XMI_ERROR;
		rc = posix_memalign((void **)&_bytesUsed, 16, num_ctx * sizeof(*_bytesUsed));
		if (rc != 0) return XMI_ERROR;
		memset(_bytesUsed, 0, num_ctx * sizeof(*_bytesUsed));
		return XMI_SUCCESS;
	}

	// must be called from thread-safe code - serialized by caller.
	inline void *reserveWUSpace(size_t contextid, size_t length) {
		void *v;
		if (_bytesUsed[contextid] + length > sizeof(*_wakeup_region)) {
			return NULL;
		}
		v = ((char *)&_wakeup_region[contextid] + _bytesUsed[contextid]);
		_bytesUsed[contextid] += length;
		return v;
	}

	inline void getWURange(size_t ctx0, size_t nctx, uint64_t *base, uint64_t *mask) {
		// this isn't quite right... needs to be aligned properly...
		*base = (uint64_t)&_wakeup_region[ctx0];
		*mask = nctx * sizeof(*_wakeup_region) - 1; // assumes power of two
	}
private:
	BgqWakeupRegionBuffer *_wakeup_region;		///< memory for WAC for all contexts
	size_t *_bytesUsed; ///< array of per-context space used vars
}; // class BgqWakeupRegion

}; // namespace CommThread
}; // namespace Device
}; // namespace XMI

#endif // __components_devices_bgq_commthread_WakeupRegion_h__
