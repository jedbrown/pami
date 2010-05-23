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

#include <pami.h>
#include "components/devices/bgq/commthread/WakeupRegion.h"
#include "components/devices/bgq/commthread/ContextSets.h"
#include "common/bgq/Context.h"
#include <pthread.h>

#include "hwi/include/bqc/A2_inlines.h"
#include "spi/include/kernel/thread.h"

#undef HAVE_WU_ARMWITHADDRESS

#ifndef HAVE_WU_ARMWITHADDRESS
/// \todo #warning need CNK definition for WU_ArmWithAddress

// temporary test implementation going directly to WAC registers...
#include "hwi/include/bqc/wu_mmio.h"
#define USR_WAKEUP_BASE ((unsigned long *)(PHYMAP_MINADDR_L1P + 0x1c00))

#define CNK_WAKEUP_SPI_FIRST_WAC	0	// TBD: get from CNK
#define WU_ArmWithAddress(addr,mask) {				\
	int thr = Kernel_PhysicalHWThreadID();			\
	int reg = thr + CNK_WAKEUP_SPI_FIRST_WAC;		\
	USR_WAKEUP_BASE[CLEAR_THREAD(thr)] = _BN(reg);		\
	USR_WAKEUP_BASE[SET_THREAD(thr)] = (_BN(reg) >> 32);	\
	USR_WAKEUP_BASE[WAC_BASE(reg)] = addr;			\
	USR_WAKEUP_BASE[WAC_ENABLE(reg)] = mask;		\
}

#endif // !HAVE_WU_ARMWITHADDRESS

#ifndef SCHED_COMM
/// \todo #warning No SCHED_COMM from CNK yet
#define SCHED_COMM SCHED_RR
#endif // !SCHED_COMM

#define COMMTHREAD_SCHED	SCHED_COMM
//#define COMMTHREAD_SCHED	SCHED_FIFO
//#define COMMTHREAD_SCHED	SCHED_OTHER

namespace PAMI {
namespace Device {
namespace CommThread {

class BgqCommThread {
private:
        /// \brief Convenience code to lock and/or unlock contexts in set
        ///
        /// \param[in] old	Currently locked contexts
        /// \param[in] new	New set of contexts that should be locked
        ///
        inline void __lockContextSet(uint64_t &old, uint64_t new_) {
                uint64_t m, l = old;
                size_t x;
                pami_result_t r;

                m = (old ^ new_) & old; // must unlock these
                x = 0;
                while (m) {
                        if (m & 1) {
                                r = _ctxset->getContext(_client, x)->unlock();
                                l &= ~(1ULL << x);
                        }
                        m >>= 1;
                        ++x;
                }

                m = (old ^ new_) & new_; // must lock these
                x = 0;
                while (m) {
                        if (m & 1) {
                                r = _ctxset->getContext(_client, x)->trylock();
                                if (r == PAMI_SUCCESS) {
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
                pami_result_t r;
                x = 0;
                while (m) {
                        if (m & 1) {
                                e += _ctxset->getContext(_client, x)->advance(1, r);
                                r = r; // avoid warning until we figure out what to do with result
                        }
                        m >>= 1;
                        ++x;
                }
                return e;
        }

/// \todo #warning Need CNK method to enable MU "interrupts" through Wakeup Unit
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
        _client(clientid),
	_thread(0),
	_shutdown(false)
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
                                                Memory::MemoryManager *genmm,
                                                Memory::MemoryManager *l2xmm) {
                BgqCommThread *devs;
                BgqWakeupRegion *wu;
                BgqContextPool *pool;
                size_t x;
		size_t me = __global.topology_local.rank2Index(__global.mapping.task());
		size_t lsize = __global.topology_local.size();
                posix_memalign((void **)&devs, 16, num_ctx * sizeof(*devs));
                posix_memalign((void **)&pool, 16, sizeof(*pool));
                posix_memalign((void **)&wu, 16, sizeof(*wu)); // one per client

                new (wu) BgqWakeupRegion();
                pami_result_t rc = wu->init(clientid, num_ctx, me, lsize, l2xmm);
		if (rc != PAMI_SUCCESS) {
			PAMI_abortf("Failed to inialize BgqWakeupRegion - not enough shared memory?");
		}
		__global._wuRegion_mms[clientid] = wu->getAllWUmm();

                new (pool) BgqContextPool();
                pool->init(clientid, num_ctx, genmm, wu->getWUmm());

                for (x = 0; x < num_ctx; ++x) {
                        // one per context, but not otherwise tied to a context.
                        new (&devs[x]) BgqCommThread(wu, pool, clientid, num_ctx);
                }
                return devs;
        }

        static void *commThread(void *cookie) {
                BgqCommThread *thus = (BgqCommThread *)cookie;
                pami_result_t r = thus->__commThread();
                r = r; // avoid warning until we decide how to use result
                return NULL;
        }

        static inline void initContext(BgqCommThread *devs, size_t clientid,
                                        size_t contextid, pami_context_t context) {
                // might need hook later, to do per-context initialization?
        }

	// arrange for the new comm-thread to hunt-down this context and
	// take it, even if some other thread already picked it up.
	// This helps ensure a more-balanced startup, and prevents some
	// complexities of trying to re-balance later. Then, only when
	// a comm-thread leaves the set does there have to be a re-balance.
        static inline pami_result_t addContext(BgqCommThread *devs, size_t clientid,
                                        pami_context_t context) {
                // all BgqCommThread objects have the same ContextSet object.
                uint64_t m = devs[0]._ctxset->addContext(clientid, context);
		if (m == 0) {
			return PAMI_EAGAIN; // closest thing to ENOSPC ?
		}
		int status;
		// we assume this is single-threaded so no locking required...
		size_t x = _numActive++;
		BgqCommThread *thus = &devs[x];
		thus->_initCtxs = m;	// this shold never be zero
		pthread_attr_t attr;

		status = pthread_attr_init(&attr);
		if (status) {
			--_numActive;
			errno = status;
			return PAMI_CHECK_ERRNO;
		}
		status = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
		if (status) {
			pthread_attr_destroy(&attr);
			--_numActive;
			errno = status;
			return PAMI_CHECK_ERRNO;
		}
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		if (status) {
			pthread_attr_destroy(&attr);
			--_numActive;
			errno = status;
			return PAMI_CHECK_ERRNO;
		}
		status = pthread_attr_setschedpolicy(&attr, COMMTHREAD_SCHED);
		if (status) {
			pthread_attr_destroy(&attr);
			--_numActive;
			errno = status;
			return PAMI_CHECK_ERRNO;
		}

		thus->_shutdown = false;
		status = pthread_create(&thus->_thread, &attr, commThread, thus);
		pthread_attr_destroy(&attr);
		if (status) {
			--_numActive;
			thus->_thread = 0; // may not always be legal assignment?
			errno = status;
			return PAMI_CHECK_ERRNO;
		}
		return PAMI_SUCCESS;
        }

        static inline pami_result_t shutdown(BgqCommThread *devs, size_t clientid) {
		size_t x;

		if (_numActive == 0) {
			return PAMI_SUCCESS;
		}

		// while touching _shutdown will not directly wakeup commthreads,
		// our subsequent rmAllContexts() will...
		for (x = 0; x < _numActive; ++x) {
			// kill is too harsh, provide orderly termination
			//pthread_kill(devs[x]._thread, SIGTERM);
			devs[x]._shutdown = true;
		}
                // all BgqCommThread objects have the same ContextSet object.
                devs[0]._ctxset->rmAllContexts(clientid);

		// need to pthread_join() here? or is it too risky (might hang)?
		for (x = 0; x < _numActive; ++x) {
			void *status;
			pthread_join(devs[x]._thread, &status);
		}
		_numActive = 0;
		return PAMI_SUCCESS;
	}

private:
        inline pami_result_t __commThread() {
                // should/can this use the internal (C++) interface?
                uint64_t new_ctx, old_ctx, lkd_ctx;
                size_t n, events;
                size_t max_loop = 100; // \todo need some heuristic or tunable for max loops
                size_t id; // our current commthread id, among active ones.
                pthread_t self = pthread_self();
                uint64_t wu_start, wu_mask;
		int min_pri = sched_get_priority_min(COMMTHREAD_SCHED);
		int max_pri = sched_get_priority_max(COMMTHREAD_SCHED);
char buf[64];
sprintf(buf, "ct %ld\n", pthread_self());
int bufl = strlen(buf);

                pthread_setschedprio(self, max_pri);
                _ctxset->joinContextSet(_client, id, _initCtxs);
//fprintf(stderr, "comm thread %ld for context %04zx\n", self, _initCtxs);
write(2, buf, bufl);
                new_ctx = old_ctx = lkd_ctx = 0;
                while (!_shutdown) {
                        //
new_context_assignment:	// These are the same now, assuming the re-locking is
more_work:		// lightweight enough.
                        //

                        __armMU_WU();

			// doing this without 'new_ctx' depends on it being ignored...
                        _wakeup_region->getWURange(0, &wu_start, &wu_mask);

                        n = 0;
			events = 0;
			do {
                        	WU_ArmWithAddress(wu_start, wu_mask);
                        	new_ctx = _ctxset->getContextSet(_client, id);

                        	// this only locks/unlocks what changed...
                        	__lockContextSet(lkd_ctx, new_ctx);
                        	old_ctx = new_ctx;
                                events = __advanceContextSet(lkd_ctx);
                        } while (!_shutdown && lkd_ctx && (events != 0 || ++n < max_loop));
			if (_shutdown) break;

                        // Snoop the scheduler to see if other threads are competing.
                        // This should also include total number of threads on
                        // the core, and some heuristic by which we decide to
                        // back-off more. This gets complicated if we consider
                        // whether those other sw threads are truly active, or even
                        // running in some syncopated "tag team" mode.
                        // TBD
//re_evaluate:
                        n = Kernel_SnoopRunnable();

                        if (n <= 1) {
                                // we are alone
                                if (events == 0) {
                                        // The wait can only detect new work.
                                        // Only do the wait if we know the
                                        // contexts have no work. otherwise
                                        // we could wait forever for new work
                                        // while existing work waits for us to
                                        // advance it.
#ifndef HAVE_WU_ARMWITHADDRESS
buf[0] = 'g'; buf[1] = 'i';
write(2, buf, bufl);
        				ppc_waitimpl();
buf[0] = 'w'; buf[1] = 'u';
write(2, buf, bufl);
#else // HAVE_WU_ARMWITHADDRESS
                                        ppc_waitimpl();
#endif // HAVE_WU_ARMWITHADDRESS
					if (_shutdown) break;
                                }
                                // need to re-evaluate things here?
                                // goto re_evaluate;
                                // ... or just go back and do work?
                                goto more_work;
                        } else {
                                __disarmMU_WU();

                                // this only locks/unlocks what changed...
                                __lockContextSet(lkd_ctx, 0);

                                _ctxset->leaveContextSet(_client, id); // id invalid now
buf[0] = 's'; buf[1] = 'a';
write(2, buf, bufl);

                                pthread_setschedprio(self, min_pri);
                                //=== we get preempted here ===//
                                pthread_setschedprio(self, max_pri);
buf[0] = 's'; buf[1] = 'b';
write(2, buf, bufl);

				if (_shutdown) break;
                		_ctxset->joinContextSet(_client, id, _initCtxs); // got id
                                // always assume context set changed... just simpler.
                                goto new_context_assignment;
                        }
                }

		if (lkd_ctx) {
                	__lockContextSet(lkd_ctx, 0);
		}
		if (id != (size_t)-1) {
                	_ctxset->leaveContextSet(_client, id); // id invalid now
		}
buf[0] = 't'; buf[1] = 't';
write(2, buf, bufl);
                return PAMI_SUCCESS;
        }

        BgqWakeupRegion *_wakeup_region;	///< WAC memory for contexts (common)
        PAMI::Device::CommThread::BgqContextPool *_ctxset; ///< context set (common)
        size_t _client;			///< client ID
	uint64_t _initCtxs;		///< initial set of contexts to take
	pthread_t _thread;		///< pthread identifier
	volatile bool _shutdown;	///< request commthread to exit
	static size_t _numActive;
}; // class BgqCommThread

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

size_t PAMI::Device::CommThread::BgqCommThread::_numActive = 0;

#endif // __components_devices_bgq_commthread_CommThread_h__
