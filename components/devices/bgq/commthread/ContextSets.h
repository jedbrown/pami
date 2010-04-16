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

	// lock is held by caller...
	inline uint64_t __getOneContext(uint64_t &ref) {
		uint64_t k = ref;
		uint64_t m;
		size_t x = ffs(k);
		if (x > 0 && x < 64) {
			--x;
			m = (1ULL << x);
			k &= ~m;
			ref = k;
			return m;
		}
		// should never happen...
		return 0;
	}
	// lock is held by caller...
	// must ensure we complete, even if no contexts found.
	// right now, assumes caller's _sets[] is zero... (!ENABLE)
	inline uint64_t __rebalanceContexts(uint64_t initial) {
		uint64_t m = 0;
		size_t desired = (_ncontexts + (_nactive - 1)) / _nactive;
		size_t n = 0;
		size_t ni = 0;

		while (n < desired) {
			if (++_lastset >= _nsets) {
				_lastset = 0;
				if (++ni >= 2) break;
#if 0 // this is not right, yet...
				if (_notset) {
					m |= __getOneContext(k);
					++n;
					continue;
				}
#endif
			}
			uint64_t k = _sets[_lastset];
			if ((k & ENABLE)) {
				k &= ~ENABLE;
				if ((k & initial)) {
					++n; // for now, assume 1 bit set in "initial"...
					_sets[_lastset] = (k & ~initial) | ENABLE;
				} else if (k) {
					// do not take their last context...
					uint64_t mm = __getOneContext(k);
					if (k && mm) {
						m |= mm;
						++n;
						_sets[_lastset] = (k | ENABLE);
					}
				}
			}
		}
		m |= initial; // we REALLY want this context...
		return m;
	}

	// lock is held by caller...
	inline void __giveupContexts(uint64_t ctxs) {
		uint64_t m = ctxs;
                size_t x = 0;
		while (m) {
			// give away each context in a round-robin fashion.
                        if (m & 1) {
                                uint64_t n;
                                // there must be one other... or else we hang here.
                                do {
                                        if (++_lastset >= _nsets) _lastset = 0;
                                        n = _sets[_lastset];
                                        if ((n & ENABLE)) {
                                                _sets[_lastset] = n | (1ULL << x);
                                        }
                                } while (!(n & ENABLE));
                        }
                        m >>= 1;
                        ++x;
		}
	}

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
		memset(_sets, 0, nctx * sizeof(*_sets));
                _ncontexts_total = nctx;
                _nsets = nctx;
        }

	// caller promises to be single-threaded (?)
        inline uint64_t addContext(size_t clientid, pami_context_t ctx) {
                if (_ncontexts >= _ncontexts_total) {
                        return 0ULL;
                }
                _mutex.acquire();
		size_t x = _ncontexts++;
                _contexts[x] = (PAMI::Context *)ctx;
		_notset |= (1ULL << x);
                _mutex.release();
		// presumably, a new comm thread is about to call joinContextSet(),
		// but can we guarantee timing? May need a way to ensure that
		// "_notset" will be checked.
                return (1ULL << x);
        }

        PAMI::Context *getContext(size_t clientid, size_t contextix) {
                return _contexts[contextix];
        }

        inline uint64_t getContextSet(size_t clientid, size_t threadid) {
                _mutex.acquire();
                uint64_t m = _sets[threadid] & ~ENABLE;
#if 0
		// should we check "_notset" and possibly pick up more contexts?
		// or just trigger a "rejoin"?
		if (!m) {
			// must not cause thrashing, but try to get some contexts...
			if (_notset || _ncontexts >= _nactive) {
				m = __rebalanceContexts();
fprintf(stderr, "picking up extra contexts %04zx\n", m);
				_sets[threadid] = m | ENABLE;
			}
		}
#endif
                _mutex.release();
		return m;
        }

        inline void joinContextSet(size_t clientid, size_t &threadid,
						uint64_t initial = 0ULL) {

                uint64_t m = 0;
                _mutex.acquire();
                threadid = _nactive;
                if (_nactive == 0) {
                        // take all? or just a few...
                        m |= (_notset | initial);
                        _notset = 0;
                        ++_nactive;
                } else {
                        ++_nactive;
			m |= __rebalanceContexts(initial);
                }
                _sets[threadid] = m | ENABLE;
                _mutex.release();
        }

        inline void leaveContextSet(size_t clientid, size_t &threadid) {
                _mutex.acquire();
                --_nactive;
                uint64_t m = _sets[threadid];
                // assert((m & ENABLE) != 0);
                m &= ~ENABLE;
                _sets[threadid] = 0;
                if (_nactive == 0) {
                        _notset |= m;
                } else {
			__giveupContexts(m);
                }
                _mutex.release();
                threadid = -1; // only valid while we're joined.
        }
private:
        PAMI::Context **_contexts;
        size_t _ncontexts_total;
        size_t _ncontexts;

        ContextSetMutex _mutex;
        uint64_t *_sets;	// protected by _mutex
        uint64_t _notset;	// protected by _mutex
        size_t _nsets;		// protected by _mutex
        size_t _nactive;	// protected by _mutex
        size_t _lastset;	// protected by _mutex
}; // class BgqContextPool

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

#endif // __components_devices_bgq_commthread_ContextSets_h__
