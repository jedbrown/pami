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

#include "sys/xmi.h"

namespace XMI {
namespace Device {
namespace CommThread {

class BgqContextSets {
private:
	struct Control {
		uint64_t generation;
		// ???
	};

	struct Generation {
		uint64_t num_participants;
		uint64_t num_active;
		// ???
	};

	/// \brief Find the position of the MSB in word that is '1'
	///
	/// Returns 0 if no '1' bits exist. LSB is position 1.
	/// \todo should use better (more efficient) routine.
	///
	/// \param[in] n	Word to scan
	/// \return	bit position of most-significant '1'
	inline size_t fls(size_t n) {
		int x = 0;
		while (n) {
			n >>= 1;
			++x;
		}
		return x;
	}
public:
	BgqContextSets() { }

	~BgqContextSets() { }

	/// \brief Compute context set from relative position among active comm threads
	///
	/// All contexts sets must be powers of two (except last). In addition,
	/// a context set must be less than or equal in size to any context set
	/// on the left (i.e. for a lower thread index). Again, the last set
	/// (right-most) being excepted. Two values for the size of the context set
	/// are returned, 'nctx' is the number of contexts that actually exist
	/// (used for loop control when operating on contexts) and 'mctx' is the
	/// size as a power of two, used to compute the WAC address mask.
	///
	/// \param[out] ctx0	First context of set
	/// \param[out] nctx	Number of contexts to advance, etc
	/// \param[out] mctx	Size of context set (power of two)
	///
	inline void compute_set(size_t &ctx0, size_t &nctx, size_t &mctx) {
		size_t b, z, zz;
		size_t base, div;

		// assert(_numActiveThreads > 0); // at least one - ourself!

		// special-case optimize when more (same) threads than contexts
		if (_numActiveThreads >= _numContexts) {
			if (_threadIndex >= _numContexts) {
				*ctx0 = 0;
				*nctx = 0;
				*mctx = 0;
				return;
			} else {
				*ctx0 = _threadIndex;
				*nctx = 1;
				*mctx = 0;
				return;
			}
		}
		// (_numContexts / _numActiveThreads) can never be zero...

		// base number of contexts for each thread
		base = fls(_numContexts / _numActiveThreads) - 1;

		// number of threads that take extra contexts
		div = (_numContexts >> base) - _numActiveThreads;

		// may need one more thread to take extra contexts
		div += (_numContexts != ((_numActiveThreads + div) << base)) ? 1 : 0;

		// first context ID for _threadIndex
		b = (_threadIndex + (_threadIndex > div ? div : _threadIndex)) << base;

		// number of contexts to take
		z = ((_threadIndex < div ? 2 : 1) << base);

		// actual number of contexts
		zz = (b + z > _numContexts ? _numContexts - b : z);

		ctx0 = b;	// first context in set (2^X).
		nctx = zz;	// size of context set for lock/unlock/advance.
		mctx = z;	// size of context set used for wakeup unit mask (2^Y).
	}

	inline bool outOfDate() {
		return L2_AtomicLoad(&_ctl.generation) != _currentGeneration;
	}

	inline void becomeActive() {
		size_t gen = L2_AtomicLoadIncrement(&_ctl.generation);
		// this isn't even close...
		_gen[gen & _generationMask].num_active += 1;
	}

private:
	size_t _currentGeneration;
	size_t _generationMask;	///< for indexing _gen[] with _currentGeneration
	size_t _numContexts;
	size_t _numActiveThreads;
	size_t _threadIndex;
	Control *_ctl;
	Generation *_gen;
}; // class BgqContextSets

}; // namespace CommThread
}; // namespace Device
}; // namespace XMI

#endif // __components_devices_bgq_commthread_ContextSets_h__
