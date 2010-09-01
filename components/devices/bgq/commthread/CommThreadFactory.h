/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file components/devices/bgq/commthread/CommThreadFactory.h
/// \brief

#ifndef __components_devices_bgq_commthread_CommThreadFactory_h__
#define __components_devices_bgq_commthread_CommThreadFactory_h__

namespace PAMI {
namespace Device {
namespace CommThread {

class BgqCommThread;
class Factory {
public:
        Factory(PAMI::Memory::MemoryManager *genmm, PAMI::Memory::MemoryManager *l2xmm);
        ~Factory();

	/// \brief Return reference to the entire commthreads array
	///
	/// \return	pointer to the global _commThreads array
	///
        inline BgqCommThread *getCommThreads() { return _commThreads; }

	/// \brief return the next SMT to use on the given core
	///
	/// Implements a round-robin algorithm starting at the highest
	/// hwthread id a going downward. This is for finding the
	/// "home" commthread for a new context, so the SMT returned
	/// may already have a running commthread on it. It is also
	/// up to the caller to decide if a commthread has too many
	/// "core" contexts and whether to put the context on a core
	/// that might be sub-optimal.
	///
	/// Note we also don't have an "undo", so if the caller fails to
	/// actually assign this to the core/thread returned, it may end
	/// up out of balance. The caller should be taking steps to ensure
	/// that the context will be handled, but there could be an imbalance
	/// of assigned "home" threads.
	///
	/// \param[in,out] core	The core on which to place another commthread
	/// \param[out] thread	The hwthread on 'core' to use
	///
	inline void getNextThread(uint32_t &core, uint32_t &thread) {
		if (!_num_avail[core]) {
			// special case: desired core not avail
			// probably never happens, the MU RM avoids this.
			int j, k;
			// Find the first core available to this process
			for (j = NUM_CORES - 1; j >= 0 && !_num_avail[j]; --j);
			PAMI_assert_debugf(j >= 0, "Internal error: no available cores");

			// Now find the core with the least number of contexts assigned
			for (k = j--; j >= 0; --j) {
				if (!_num_avail[j]) continue;
				if (_num_used[j] < _num_used[k]) k = j;
			}
			core = k;
		}
		int x = _num_used[core]++;
		int c = ((NUM_CORES - 1) - core) * NUM_SMT; // core 0 is MSBs
		uint64_t m;
		int y;

		x %= _num_avail[core];
		x += _first[core];
		// this does not handle cases where avail threads are not adjacent,
		// but we don't expect that to ever happen.
		for (y = 0; y < _num_avail[core]; ++y) {
			m = (1UL << (x + c));
			if (_avail_threads & m) {
				thread = (NUM_SMT - 1) - x;
				return;
			}
		}
		// can this ever happen?
		// PAMI_abortf("Internal error: no avail threads");
		thread = 0; // TBD: some error value...
	}
private:
        BgqCommThread *_commThreads;
	uint64_t _proc_threads;
	uint64_t _comm_threads;
	uint64_t _avail_threads;
	int _num_used[NUM_CORES];
	int _num_avail[NUM_CORES];
	int _first[NUM_CORES];
}; // class Factory

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

#endif // __components_devices_bgq_commthread_CommFactory_h__
