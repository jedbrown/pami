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
#include "common/bgq/Context.h"
#include "pthread.h"

#warning need CNK definitions for WU_ArmWithAddress and Kernel_SnoopScheduler
#define WU_ArmWithAddress(...)
#define Kernel_SnoopScheduler()		(0)

namespace XMI {
namespace Device {
namespace CommThread {

class BgqCommThread {
private:
	/// \brief Convenience code to lock and/or unlock contexts in set
	///
	/// \param[in] old	Currently locked contexts
	/// \param[in] new	New set of contexts that should be locked
	///
	inline void __lockContextSet(uint64_t &old, uint64_t &new) {
		uint64_t m, l = 0;
		size_t x;
		xmi_result_t r;

		m = (old ^ new) & old; // must unlock these
		x = 0;
		while (m) {
			if (m & 1) {
				r = _ctxset.getContext(_client, x)->unlock();
			}
			m >>= 1;
			++x;
		}

		m = (old ^ new) & new; // must lock these
		x = 0;
		while (m) {
			if (m & 1) {
				r = _ctxset.getContext(_client, x)->trylock();
				if (r == XMI_SUCCESS) {
					l |= (1ULL << x);
				}
			}
			m >>= 1;
			++x;
		}
		old = l;
	}

	/// \brief Convenience code to advance all contexts in set
	///
	/// \param[in] ctx	Bitmap of contexts that are locked
	/// \return	Number of work events
	///
	inline size_t __advanceContextSet(uint64_t ctx) {
		size_t x, e = 0;
		uint64_t m = ctx;
		xmi_result_t r;
		x = 0;
		while (m) {
			if (m & 1) {
				e += _ctxset.getContext(_client, x)->advance(1, r);
				r = r; // avoid warning until we figure out what to do with result
			}
			m >>= 1;
			++x;
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
	BgqCommThread(BgqWakeupRegion *wu, BgqContextPool *pool, size_t clientid, size_t num_ctx) :
	_wakeup_region(wu),
	_ctxset(pool),
	_client(clientid)
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
							Memory::MemoryManager *mm) {
		BgqCommThread *devs;
		BgqWakeupRegion *wu;
		xmi_context_t *ctxs;
		BgqContextPool *pool;
		size_t x;
		posix_memalign((void **)&devs, 16, num_ctx * sizeof(*devs));
		posix_memalign((void **)&pool, 16, sizeof(*pool));

		new (pool) BgqContextPool();
		pool->init(clientid, num_ctx, mm);

		posix_memalign((void **)&wu, 16, sizeof(*wu)); // one per client
		new (wu) BgqWakeupRegion();
		wu->init(clientid, num_ctx, mm);

		for (x = 0; x < num_ctx; ++x) {
			// one per context, but not otherwise tied to a context.
			new (&devs[x]) BgqCommThread(wu, pool, clientid, num_ctx);
		}
		return devs;
	}

	static inline void initContext(BgqCommThread *devs, size_t clientid,
					size_t contextid, xmi_context_t context) {
		// might need hook later, to do per-context initialization?
	}

	static void *commThread(void *cookie) {
		BgqCommThread *thus = (BgqCommThread *)cookie;
		xmi_result_t r = thus->__commThread();
		return NULL;
	}

private:
	inline xmi_result_t __commThread() {
		// should/can this use the internal (C++) interface?
		uint64_t new_ctx, old_ctx, lkd_ctx;
		size_t n, events;
		size_t max_loop = 100; // \todo need some heuristic or tunable for max loops
		size_t id; // our current commthread id, among active ones.
		pthread_t self = pthread_self();
		uint64_t wu_start, wu_mask;

		pthread_setschedprio(self, 99); // need official constant here
		_ctxset->joinContextSet(0, id);
		new_ctx = old_ctx = lkd_ctx = 0;
		while (1) {
			//
new_context_assignment:	// These are the same now, assuming the re-locking is
more_work:		// lightweight enough.
			//
			new_ctx = _ctxset->getContextSet(0, id);
			_wakeup_region->getWURange(new_ctx, &wu_start, &wu_mask);

			__armMU_WU();

			// this only locks/unlocks what changed...
			__lockContextSet(lkd_ctx, new_ctx);
			old_ctx = new_ctx;
			n = 0;
			do {
				WU_ArmWithAddress(wu_start, wu_mask);
				events = __advanceContextSet(lkd_ctx);
			} while (events != 0 || ++n < max_loop);

			// Snoop the scheduler to see if other threads are competing.
			// This should also include total number of threads on
			// the core, and some heuristic by which we decide to
			// back-off more. This gets complicated if we consider
			// whether those other sw threads are truly active, or even
			// running in some syncopated "tag team" mode.
			// TBD
re_evaluate:
			n = Kernel_SnoopScheduler();

			if (n == 1) {
				// we are alone
				if (events == 0) {
					// The wait can only detect new work.
					// Only do the wait if we know the
					// contexts have no work. otherwise
					// we could wait forever for new work
					// while existing work waits for us to
					// advance it.
					ppc_waitimpl();
				}
				// need to re-evaluate things here?
				// goto re_evaluate;
				// ... or just go back and do work?
				goto more_work;
			} else {
				__disarmMU_WU();

				// this only locks/unlocks what changed...
				__lockContextSet(lkd_ctx, 0);

				_ctxset->leaveContextSet(0, id); // id invalid now

				pthread_setschedprio(self, 0); // need official constant
				//=== we get preempted here ===//
				pthread_setschedprio(self, 99); // need official constant

				_ctxset->joinContextSet(0, id); // got new id now...
				// always assume context set changed... just simpler.
				goto new_context_assignment;
			}
		}
		// not reached?

		// must have unlocked before this... ??
		// __lockContextSet(lkd_ctx, 0);
		_ctxset->leaveContextSet(0, id); // id invalid now
		return XMI_SUCCESS;
	}

	BgqWakeupRegion *_wakeup_region;///< memory for WAC for all contexts
	XMI::Device::CommThread::BgqContextPool *_ctxset;
	size_t _client;
}; // class BgqCommThread

}; // namespace CommThread
}; // namespace Device
}; // namespace XMI

#endif // __components_devices_bgq_commthread_CommThread_h__
