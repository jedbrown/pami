/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file components/devices/bgq/commthread/CommThread.h
/// \brief

#ifndef __components_devices_bgq_commthread_CommThread_h__
#define __components_devices_bgq_commthread_CommThread_h__

#include "sys/xmi.h"
#include "components/devices/bgq/commthread/WakeupRegion.h"
#include "pthread.h"

#warning need CNK definitions for WU_ArmWithAddress and Kernel_SnoopScheduler
#define WU_ArmWithAddress(...)
#define Kernel_SnoopScheduler()		(0)

namespace XMI {
namespace Device {
namespace CommThread {

class BgqCommThread {
public:
	BgqCommThread(BgqWakeupRegion *wu) :
	_wakeup_region(wu),
	_first_context(0),
	_num_contexts(0),
	_contexts_changed(0)
	{ }

	~BgqCommThread() { }

	static BgqCommThread *generate(size_t clientid, size_t num_ctx, Memory::MemoryManager &mm) {
		BgqCommThread *devs;
		BgqWakeupRegion *wu;
		size_t x;
		posix_memalign((void **)&devs, 16, num_ctx * sizeof(*devs));
		posix_memalign((void **)&wu, 16, sizeof(*wu)); // not right alignment...
		new (wu) BgqWakeupRegion();
		wu->init(clientid, num_ctx, mm);
		for (x = 0; x < num_ctx; ++x) {
			new (&devs[x]) BgqCommThread(wu);
		}
		return devs;
	}

	static void *commThread(void *cookie) {
		// should/can this use the internal (C++) interface?
		size_t ctx0, nctx;
		size_t x, n, events;
		size_t max_loop = 100; // some heuristic or env var?
		BgqCommThread *thus = (BgqCommThread *)cookie;
		pthread_t self = pthread_self();
		uint64_t wu_start, wu_mask;

		pthread_setschedprio(self, 99); // need official constant here
		while (1) {
new_context_assignment:
			// need to detect when our context assignment changes,
			// then need to tell others that we have responded.
			// This is probably not right yet. Need to do some sort
			// of barrier-like operation to ensure that everyone
			// has compatible (disjoint) sets of contexts. Perhaps
			// some global data about number of comm threads to
			// split contexts across, and then each commthread just
			// uses the same information in a consistent, cooperative,
			// way. Need to ensure, though, that all comm threads quickly
			// respond to changes. And need to know when they all have
			// reponded.
			//
			// The event that triggers this is a comm thread voluntarily
			// relinquishing control (lowering context below). At that
			// point the comm thread should remove itself from the "global
			// pool" and cause the rest to reshuffle contexts between
			// them.  At the point that the comm thread regains control
			// and raises it's priority, it needs to rejoin the global
			// pool and cause the rest to reshuffle some contexts back
			// to this thread. These reshuffle operations could be
			// done lazily, but should not take too long. We also need
			// to avoid thrashing if for some reason a competing thread
			// is ill-behaved and keeps waking up only long enough to
			// confirm it should go back to sleep. These reshufflings
			// need to be efficient, or at least managed efficiently.
			//
			// We also can't use tryLock here, or at least can't
			// forget contexts if tryLock fails, since we need the
			// WAC region to be contiguous and without any unrelated
			// changing memory over the entire WAC range. So, if a tryLock
			// fails we need to be persistent about retrying and
			// beware of doing a wait if we don't have all the locks.
			//
			// This also assumes that the context lock is not needed
			// in order to initiate communications - which may not
			// be true. In that case we hope to be frequently going
			// through the 'wait' code (unlocking contexts), but that
			// may not always be the case. Perhaps the communications
			// (models?) always have a fall-back mode that does a
			// context-post of the operation to the desired context.
			//
			do {
				thus->_contexts_changed = 0;	// tells others that
								// our previous set of
								// contexts are unlocked.
				// mbar?
				ctx0 = thus->_first_context;
				nctx = thus->_num_contexts;
			} while (thus->_contexts_changed != 0);
			// assert(ctx0 + nctx <= _total_ncontexts);

			// arm MU-WU signals...
			// must lock all contexts, since we need a contiguous region
			// to wait on.
			for (x = ctx0; x < ctx0 + nctx; ++x) {
				XMI_Context_lock(thus->_all_contexts[x]);
			}
more_work:
			thus->_wakeup_region->getWURange(ctx0, nctx, &wu_start, &wu_mask);
			n = 0;
			do {
				WU_ArmWithAddress(wu_start, wu_mask);
				events = 0;
				for (x = ctx0; x < ctx0 + nctx; ++x) {
					// better way to advance these?
					events += XMI_Context_advance(thus->_all_contexts[x], 1);
				}
			} while (events != 0 || ++n < max_loop);
			n = Kernel_SnoopScheduler();
			if (n == 1 && thus->_contexts_changed == 0) { // we are alone
				if (events == 0) {
					ppc_waitimpl();
				}
				goto more_work;
			} else {
				// disarm MU-WU signals...
				for (x = ctx0; x < ctx0 + nctx; ++x) {
					XMI_Context_unlock(thus->_all_contexts[x]);
				}
				// if n == 1 (i.e. _contexts_changed != 0) we could
				// go directly to new_context_assignment,
				// and avoid priority change.

				pthread_setschedprio(self, 0); // need official constant
				//=== we get preempted here ===//
				pthread_setschedprio(self, 99); // need official constant

				// always assume context set changed... just simpler.
				goto new_context_assignment;
			}
		}
		return NULL;
	}

private:
	xmi_context_t *_all_contexts;	///< all contexts (in client)
	BgqWakeupRegion *_wakeup_region;///< memory for WAC for all contexts
	size_t _total_ncontexts;	///< total number of contexts (in client)
	// operate strictly on a range of contexts, so that we can use the wakeup unit.
	size_t _first_context;
	size_t _num_contexts;
	bool _contexts_changed;
}; // class BgqCommThread

}; // namespace CommThread
}; // namespace Device
}; // namespace XMI

#endif // __components_devices_bgq_commthread_CommThread_h__
