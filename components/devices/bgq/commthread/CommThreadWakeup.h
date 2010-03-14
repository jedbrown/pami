/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file components/devices/bgq/commthread/CommThreadWakeup.h
/// \brief

#ifndef __components_devices_bgq_commthread_CommThreadWakeup_h__
#define __components_devices_bgq_commthread_CommThreadWakeup_h__

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
private:
	/// \brief Inform the community that we are ready to advance some contexts
	///
	/// No contexts are locked at this point (by this member).
	///
	inline void __joinActiveGroup() {
		// _ctxset->becomeActive();
	}

	/// \brief Inform the community that we will no longer advance contexts
	///
	/// All contexts are unlocked at this point (by this member).
	///
	inline void __leaveActiveGroup() {
		// _ctxset->becomeInactive();
	}

	/// \brief Get the set of contexts this comm thread should use
	///
	/// This is certainly not the right way. Need a scheme that
	/// does not require all comm threads to be altered. Probably
	/// should work off some central variable that all comm threads
	/// use identically to compute who advances what contexts.
	/// Perhaps an atomic counter which provides a comm thread with
	/// a position in the context list, and a variable (atomic counter)
	/// which tells how many active comm threads there are. But this
	/// will require some synchronization to ensure everyone uses the
	/// same data to make that decision. Maybe two (or more) sets of
	/// this data and a generation scheme whereby comm threads know
	/// what generation they are working off.
	///
	/// This also needs to be fairly lightweight, and we need to
	/// avoid thrashing in cases where comm threads are frequently
	/// trying to sleep (e.g. ill-behaved app thread that keeps waking
	/// up only long enough to discover it should go back to sleep).
	///
	/// \param[out] ctx0	First context ID to advance
	/// \param[out] nctx	Number of contiguous contexts to advance
	/// \param[out] mctx	Power-of-two size of context set (for WAC mask)
	///
	inline void __getContextSet(size_t &ctx0, size_t &nctx, size_t &mctx) {
		// _ctxset->compute_set(ctx0, nctx, mctx);
		// return;
		do {
			_contexts_changed = 0;	// tells others that
						// our previous set of
						// contexts are unlocked.
			// mbar?
			ctx0 = _first_context;
			nctx = _num_contexts;
		} while (_contexts_changed != 0);
		// assert(ctx0 + nctx <= _total_ncontexts);
		// ctx0 and mctx must be powers of two.
	}

	inline void __dropContextSet(size_t &ctx0, size_t &nctx, size_t &mctx) {
	}

	/// \brief Tell whether comm thread should reshuffle context set
	///
	/// This is probably not right yet.
	/// \ref __getContextSet
	///
	/// need to detect when our context assignment changes,
	/// then need to tell others that we have responded.
	/// This is probably not right yet. Need to do some sort
	/// of barrier-like operation to ensure that everyone
	/// has compatible (disjoint) sets of contexts. Perhaps
	/// some global data about number of comm threads to
	/// split contexts across, and then each commthread just
	/// uses the same information in a consistent, cooperative,
	/// way. Need to ensure, though, that all comm threads quickly
	/// respond to changes. And need to know when they all have
	/// reponded.
	///
	/// The event that triggers this is a comm thread voluntarily
	/// relinquishing control (__leave/joinActiveGroup()). At that
	/// point the comm thread should remove itself from the "global
	/// pool" and cause the rest to reshuffle contexts between
	/// them.  At the point that the comm thread regains control
	/// and raises it's priority, it needs to rejoin the global
	/// pool and cause the rest to reshuffle some contexts back
	/// to this thread. These reshuffle operations could be
	/// done lazily, but should not take too long. We also need
	/// to avoid thrashing if for some reason a competing thread
	/// is ill-behaved and keeps waking up only long enough to
	/// confirm it should go back to sleep. These reshufflings
	/// need to be efficient, or at least managed efficiently.
	///
	/// \return	true if contexts need to be shuffled
	///
	inline bool __needContextShuffle() {
		// return _ctxset->isOutOfDate();
		return (_contexts_changed != 0);
	}

	/// \brief Convenience code to lock all contexts in set
	///
	/// We also can't use tryLock here, or at least can't
	/// forget contexts if tryLock fails, since we need the
	/// WAC region to be contiguous and without any unrelated
	/// changing memory over the entire WAC range. So, if a tryLock
	/// fails we need to be persistent about retrying and
	/// beware of doing a wait if we don't have all the locks.
	///
	/// This also assumes that the context lock is not needed
	/// in order to initiate communications - which may not
	/// be true. In that case we hope to be frequently going
	/// through the 'wait' code (unlocking contexts), but that
	/// may not always be the case. Perhaps the communications
	/// (models?) always have a fall-back mode that does a
	/// context-post of the operation to the desired context.
	///
	/// \todo Determine if tryLock is required and if so work out scheme
	/// to lazily acquire locks while still advancing some contexts.
	///
	/// \param[in] ctx0	First context ID to advance
	/// \param[in] nctx	Number of contiguous contexts to advance
	///
	inline void __lockContextSet(size_t ctx0, size_t nctx) {
		size_t x;
		// must lock all contexts, since we need a contiguous region
		// to wait on.
		for (x = ctx0; x < ctx0 + nctx; ++x) {
			XMI_Context_lock(_all_contexts[x]);
		}
	}

	/// \brief Convenience code to unlock all contexts in set
	///
	/// \param[in] ctx0	First context ID to advance
	/// \param[in] nctx	Number of contiguous contexts to advance
	///
	inline void __unlockContextSet(size_t ctx0, size_t nctx) {
		size_t x;
		for (x = ctx0; x < ctx0 + nctx; ++x) {
			XMI_Context_unlock(_all_contexts[x]);
		}
	}

	/// \brief Convenience code to advance all contexts in set
	///
	/// \param[in] ctx0	First context ID to advance
	/// \param[in] nctx	Number of contiguous contexts to advance
	/// \return	Number of work events
	///
	inline size_t __advanceContextSet(size_t ctx0, size_t nctx) {
		size_t x, e = 0;
		for (x = ctx0; x < ctx0 + nctx; ++x) {
			e += XMI_Context_advance(_all_contexts[x], 1);
		}
		return e;
	}

	/// \brief Arm the MU interrupt-through-Wakeup Unit
	///
	inline void __armMU_WU() { }

	/// \brief Disarm the MU interrupt-through-Wakeup Unit
	///
	inline void __disarmMU_WU() { }
public:
	BgqCommThread(xmi_context_t *ctxs, BgqWakeupRegion *wu, size_t num_ctx) :
	_all_contexts(ctxs),
	_wakeup_region(wu),
	_total_ncontexts(num_ctx),
	_first_context(0),
	_num_contexts(0),
	_contexts_changed(0)
	{ }

	~BgqCommThread() { }

	// This isn't really a device, only using this for convenience but
	// perhaps it should not be part of PlatformDeviceList?
	//
	// Also, comm threads should probably not be sub-ordinate to clients?
	// But then we have trouble keeping contexts contiguous.
	// Should comm threads be started before Client initialize?
	//
	static inline BgqCommThread *generate(size_t clientid, size_t num_ctx,
							Memory::MemoryManager &mm) {
		BgqCommThread *devs;
		BgqWakeupRegion *wu;
		xmi_context_t *ctxs;
		size_t x;
		posix_memalign((void **)&devs, 16, num_ctx * sizeof(*devs));
		posix_memalign((void **)&ctxs, 16, num_ctx * sizeof(*ctxs));

		posix_memalign((void **)&wu, 16, sizeof(*wu)); // one per client
		new (wu) BgqWakeupRegion();
		wu->init(clientid, num_ctx, mm);

		for (x = 0; x < num_ctx; ++x) {
			// one per context, but not otherwise tied to a context.
			new (&devs[x]) BgqCommThread(ctxs, wu, num_ctx);
		}
		return devs;
	}

	static inline void initContext(BgqCommThread *devs, size_t clientid,
					size_t contextid, xmi_context_t context) {
		// all have pointer to same array - one update gets all
		devs[0]._all_contexts[contextid] = context;
	}

	static void *commThread(void *cookie) {
		// should/can this use the internal (C++) interface?
		size_t ctx0, nctx, mctx;
		size_t n, events;
		size_t max_loop = 100; // \todo need some heuristic or tunable for max loops
		BgqCommThread *thus = (BgqCommThread *)cookie;
		pthread_t self = pthread_self();
		uint64_t wu_start, wu_mask;

		pthread_setschedprio(self, 99); // need official constant here
		thus->__joinActiveGroup();
		while (1) {
new_context_assignment:
			thus->__getContextSet(ctx0, nctx, mctx);
			thus->_wakeup_region->getWURange(ctx0, mctx, &wu_start, &wu_mask);

			thus->__armMU_WU();

			thus->__lockContextSet(ctx0, nctx);
more_work:
			n = 0;
			do {
				WU_ArmWithAddress(wu_start, wu_mask);
				events = thus->__advanceContextSet(ctx0, nctx);
			} while (events != 0 || ++n < max_loop);

			// Snoop the scheduler to see if other threads are competing.
			// This should also include total number of threads on
			// the core, and some heuristic by which we decide to
			// back-off more. This gets complicated if we consider
			// whether those other sw threads are truly active, or even
			// running in some syncopated "tag team" mode.
			// TBD
			n = Kernel_SnoopScheduler();

			if (n == 1 && !thus->__needContextShuffle()) {
				// we are alone and the active group did not change
				if (events == 0) {
					// The wait can only detect new work.
					// Only do the wait if we know the
					// contexts have no work. otherwise
					// we could wait forever for new work
					// while existing work waits for us to
					// advance it.
					ppc_waitimpl();
				}
				goto more_work;
			} else {
				thus->__disarmMU_WU();

				thus->__unlockContextSet(ctx0, nctx);
				thus->__dropContextSet(ctx0, nctx, mctx);
				if (n == 1) { // i.e. __needContextShuffle
					// avoid thrash just to adapt to other threads
					goto new_context_assignment;
				}
				thus->__leaveActiveGroup();

				// if n == 1 (i.e. __needContextShuffle()) we could
				// go directly to new_context_assignment,
				// and avoid priority change.

				pthread_setschedprio(self, 0); // need official constant
				//=== we get preempted here ===//
				pthread_setschedprio(self, 99); // need official constant

				thus->__joinActiveGroup();
				// always assume context set changed... just simpler.
				goto new_context_assignment;
			}
		}
		// not reached?

		// must have unlocked before this...
		thus->__leaveActiveGroup();
		return NULL; // or poof()
	}

private:
	xmi_context_t *_all_contexts;	///< all contexts (in client)
	BgqWakeupRegion *_wakeup_region;///< memory for WAC for all contexts
	size_t _total_ncontexts;	///< total number of contexts (in client)

	// operate strictly on a range of contexts, so that we can use the wakeup unit.
	// XMI::Device::CommThread::BgqContextSets *_ctxset;
	size_t _first_context;
	size_t _num_contexts;
	bool _contexts_changed;
}; // class BgqCommThread

}; // namespace CommThread
}; // namespace Device
}; // namespace XMI

#endif // __components_devices_bgq_commthread_CommThread_h__
