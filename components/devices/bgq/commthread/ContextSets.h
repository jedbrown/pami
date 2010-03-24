/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file components/devices/bgq/commthread/ContextSets.h
/// \brief

#ifndef __components_devices_bgq_commthread_ContextSets_h__
#define __components_devices_bgq_commthread_ContextSets_h__

#include "sys/pami.h"
#include "components/atomic/bgq/L2Mutex.h"

namespace PAMI {
namespace Device {
namespace CommThread {

class BgqContextPool {
	static const uint64_t ENABLE = (1ULL << 63);

	typedef PAMI::Mutex::BGQ::L2ProcMutex ContextSetMutex;
public:
	BgqContextPool() :
	_contexts(NULL),
	_ncontexts_total(0),
	_ncontexts(0),
	_mutex(),
	_sets(NULL),
	_notset(0ULL),
	_nsets(0),
	_nactive(0),
	_lastset(0)
	{
	}

	inline void init(size_t clientid, size_t nctx, Memory::MemoryManager *mm) {
		_mutex.init(mm);
		posix_memalign((void **)&_contexts, 16, nctx * sizeof(*_contexts));
		PAMI_assertf(_contexts, "Out of memory for BgqContextPool::_contexts");
		posix_memalign((void **)&_sets, 16, nctx * sizeof(*_sets));
		PAMI_assertf(_contexts, "Out of memory for BgqContextPool::_sets");
		_ncontexts_total = nctx;
	}

	inline pami_result_t addContext(size_t clientid, pami_context_t ctx) {
		if (_ncontexts >= _ncontexts_total) {
			return PAMI_ERROR;
		}
		_contexts[_ncontexts++] = (PAMI::Context *)ctx;
		return PAMI_SUCCESS;
	}

	PAMI::Context *getContext(size_t clientid, size_t contextix) {
		return _contexts[contextix];
	}

	inline uint64_t getContextSet(size_t clientid, size_t threadid) {
		return _sets[threadid] & ~ENABLE;
	}

	inline void joinContextSet(size_t clientid, size_t &threadid) {

		uint64_t m = ENABLE;
		_mutex.acquire();
		threadid = _nactive;
		if (_nactive == 0) {
			// take all? or just a few...
			m |= _notset;
			_notset = 0;
			++_nactive;
		} else {
			++_nactive;
			size_t desired = (_ncontexts + (_nactive - 1)) / _nactive;
			size_t n = 0;

			while (n < desired) {
				uint64_t k = _sets[_lastset];
				if ((k & ENABLE)) {
					size_t x = ffs(k);
					if (x > 0 && x < 64) {
						--x;
						k &= (1ULL << x);
						m |= (1ULL << x);
						++n;
						_sets[_lastset] = k;
					}
				}
				if (++_lastset >= _nsets) _lastset = 0;
			}
		}
		_sets[threadid] = m;
		_mutex.release();
	}

	inline void leaveContextSet(size_t clientid, size_t &threadid) {

		_mutex.acquire();
		--_nactive;
		uint64_t m = _sets[threadid];
		_sets[threadid] = 0;
		size_t x = 0;
		// assert((m & ENABLE) != 0);
		m &= ~ENABLE;
		if (_nactive == 0) {
			_notset |= m;
		} else while (m) {
			if (m & 1) {
				uint64_t n;
				// there must be one other... or else we hang here.
				do {
					n = _sets[_lastset];
					if ((n & ENABLE)) {
						_sets[_lastset] = n | (1ULL << x);
					}
					if (++_lastset >= _nsets) _lastset = 0;
				} while (!(n & ENABLE));
			}
			m >>= 1;
			++x;
		}
		_mutex.release();
		threadid = -1; // only valid while we're joined.
	}
private:
	PAMI::Context **_contexts;
	size_t _ncontexts_total;
	size_t _ncontexts;

	ContextSetMutex _mutex;
	uint64_t *_sets;
	uint64_t _notset;
	size_t _nsets;
	size_t _nactive;
	size_t _lastset;
}; // class BgqContextPool

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

#endif // __components_devices_bgq_commthread_ContextSets_h__
