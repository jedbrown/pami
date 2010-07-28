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

#include <pami.h>
#include "components/atomic/bgq/L2Mutex.h"

namespace PAMI {
namespace Device {
namespace CommThread {

class BgqContextPool {
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
	// right now, assumes caller's _sets[] is zero...
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
				if (_sets[_nsets]) {
					m |= __getOneContext(k);
					++n;
					continue;
				}
#endif
			}
			uint64_t k = _sets[_lastset];
			if (_actm & (1 << _lastset)) {
				if ((k & initial)) {
					++n; // for now, assume 1 bit set in "initial"...
					_sets[_lastset] = (k & ~initial);
				} else if (k) {
					// do not take their last context...
					uint64_t mm = __getOneContext(k);
					k &= mm;
					if (k) {
						m |= mm;
						++n;
						_sets[_lastset] = k;
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
int count = 0;
                                while (1) {
                                        if (++_lastset >= _nsets) {
						_lastset = 0;
if (++count == 5) fprintf(stderr, "hung in __giveupContexts(%zx) %zd %zd %zd (%zx)\n", ctxs, _nactive, _nsets, x, _actm);
					}
                                        if ((_actm & (1 << _lastset))) {
                                        	n = _sets[_lastset];
                                                _sets[_lastset] = n | (1ULL << x);
						break;
                                        }
                                }
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
        _actm(0),
        _sets(NULL),
        _nsets(0),
        _nactive(0),
        _lastset(0)
        {
        }

        inline void init(size_t nsets, size_t nctx,
				Memory::MemoryManager *mm,
				Memory::MemoryManager *setmm) {
                _mutex.init(mm);
                posix_memalign((void **)&_contexts, 16, nctx * sizeof(*_contexts));
                PAMI_assertf(_contexts, "Out of memory for BgqContextPool::_contexts");
                setmm->memalign((void **)&_sets, 16, (nsets + 1) * sizeof(*_sets));
                PAMI_assertf(_sets, "Out of memory for BgqContextPool::_sets");
		memset(_sets, 0, (nsets + 1) * sizeof(*_sets));
                _ncontexts_total = nctx;
                _nsets = nsets;
        }

	// caller promises to be single-threaded (?)
        inline uint64_t addContext(pami_context_t ctx) {
                _mutex.acquire();
		size_t x;
		for (x = 0; x < _ncontexts_total; ++x) {
			if (_contexts[x] == NULL) {
				break;
			}
		}
		if (x >= _ncontexts_total) {
			// should never happen
                        return 0ULL;
		}
                _contexts[x] = (PAMI::Context *)ctx;
		++_ncontexts;
		_sets[_nsets] |= (1ULL << x);
                _mutex.release();
		// presumably, a new comm thread is about to call joinContextSet(),
		// but can we guarantee timing? May need a way to ensure that
		// "_sets[_nsets]" will be checked.
                return (1ULL << x);
        }

        inline uint64_t rmContexts(pami_context_t *ctxs, size_t nctx) {
		size_t x, y;
		uint64_t mask = 0;
                _mutex.acquire();
		for (y = 0; y < nctx; ++y) {
			for (x = 0; x < _ncontexts_total; ++x) {
				if (_contexts[x] == ctxs[y]) {
					_contexts[x] = NULL;
					--_ncontexts;
					mask |= (1ULL << x);
				}
			}
		}
		for (x = 0; x <= _nsets; ++x) {
			_sets[x] &= ~mask;
		}
		// maybe should rebalance?
		// also, caller probably needs to wait until all the contexts
		// are released (i.e. lock released).
                _mutex.release();
		return mask;
	}

        PAMI::Context *getContext(size_t contextix) {
		PAMI::Context *c = _contexts[contextix];
		PAMI_assert_debugf(c, "Internal error: NULL context[%zd] assigned to commthread", contextix);
                return c;
        }

        inline uint64_t getContextSet(size_t threadid) {
                _mutex.acquire();
                uint64_t m = _sets[threadid];
#if 0
		// should we check "_sets[_nsets]" and possibly pick up more contexts?
		// or just trigger a "rejoin"?
		if (!m) {
			// must not cause thrashing, but try to get some contexts...
			if (_sets[_nsets] || _ncontexts >= _nactive) {
				m = __rebalanceContexts();
fprintf(stderr, "picking up extra contexts %04zx\n", m);
				_sets[threadid] = m;
				_actm |= (1 << threadid);
			}
		}
#endif
                _mutex.release();
		return m;
        }

        inline void joinContextSet(size_t &threadid,
						uint64_t initial = 0ULL) {

                uint64_t m = 0, n = 0;
                _mutex.acquire();
		while (n < 64 && (_actm & (1 << n)) != 0) {
			++n;
		}
                threadid = n;
                if (_nactive == 0) {
                        // take all? or just a few...
                        m |= (_sets[_nsets] | initial);
                        _sets[_nsets] = 0;
                        ++_nactive;
                } else {
                        ++_nactive;
			m |= __rebalanceContexts(initial);
                }
                _sets[threadid] = m;
		// do not set bit until after calling __rebalanceContexts()
		_actm |= (1 << threadid);
                _mutex.release();
        }

        inline void leaveContextSet(size_t &threadid) {
                _mutex.acquire();
                --_nactive;
                uint64_t m = _sets[threadid];
                // PAMI_assert((_actm & (1 << threadid)) != 0);
                _sets[threadid] = 0;
		// must clear bit before calling __giveupContexts()
		_actm &= ~(1 << threadid);
                if (_nactive == 0) {
                        _sets[_nsets] |= m;
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
        uint64_t _actm;		// protected by _mutex
        uint64_t *_sets;	// protected by _mutex
        size_t _nsets;		// protected by _mutex
        size_t _nactive;	// protected by _mutex
        size_t _lastset;	// protected by _mutex
}; // class BgqContextPool

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

#endif // __components_devices_bgq_commthread_ContextSets_h__
