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
				for (x = ctx0; x < ctx0 + nctx; ++x) {
					XMI_Context_unlock(thus->_all_contexts[x]);
				}
				// disarm MU-WU signals...
				pthread_setschedprio(self, 0); // need official constant
				// we get preempted here
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
