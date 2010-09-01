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
#include "components/devices/bgq/commthread/CommThreadFactory.h"
#include "common/bgq/Context.h"
#include <pthread.h>
#include <signal.h>

#include "hwi/include/bqc/A2_inlines.h"
#include "spi/include/kernel/thread.h"

#define DEBUG_COMMTHREADS // Enable debug messages here

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

/// \todo #warning Need CNK method to enable MU "interrupts" through Wakeup Unit
#define WU_ArmMU(bits) {				\
	int thr = Kernel_PhysicalHWThreadID();		\
	USR_WAKEUP_BASE[SET_THREAD(thr)] = _B4(52,(bits));\
}

#define WU_DisarmMU(bits) {				\
	int thr = Kernel_PhysicalHWThreadID();		\
	USR_WAKEUP_BASE[CLEAR_THREAD(thr)] = _B4(52,(bits));\
}

#endif // !HAVE_WU_ARMWITHADDRESS

#ifndef SCHED_COMM
/// \todo #warning No SCHED_COMM from CNK yet
#define SCHED_COMM SCHED_RR
#endif // !SCHED_COMM

#define COMMTHREAD_SCHED	SCHED_COMM
//#define COMMTHREAD_SCHED	SCHED_FIFO
//#define COMMTHREAD_SCHED	SCHED_OTHER

#ifdef DEBUG_COMMTHREADS
#define DEBUG_INIT()						\
		char __dbgbuf[64];				\
		sprintf(__dbgbuf, "xx %ld\n", pthread_self());	\
		int __dbgbufl = strlen(__dbgbuf);

#define DEBUG_WRITE(a,b)				\
		__dbgbuf[0] = a; __dbgbuf[1] = b;	\
		write(2, __dbgbuf, __dbgbufl);
#else // ! DEBUG_COMMTHREADS
#define DEBUG_INIT()
#define DEBUG_WRITE(a,b)
#endif // ! DEBUG_COMMTHREADS

extern PAMI::Device::CommThread::Factory __CommThreadGlobal;

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
	inline size_t __lockContextSet(uint64_t &old, uint64_t new_) {
		uint64_t m, l = old;
		size_t x, e = 0;
		pami_result_t r;

		m = (old ^ new_) & old; // must unlock these
		x = 0;
		while (m) {
			if (m & 1) {
				_ctxset->getContext(x)->cleanupAffinity(false);
				r = _ctxset->getContext(x)->unlock();
				l &= ~(1ULL << x);
			}
			m >>= 1;
			++x;
		}

		m = (old ^ new_) & new_; // must lock these
		x = 0;
		while (m) {
			if (m & 1) {
				++e; // if we need, or get, new conetxt
				r = _ctxset->getContext(x)->trylock();
				if (r == PAMI_SUCCESS) {
					l |= (1ULL << x);
					_ctxset->getContext(x)->cleanupAffinity(true);
				}
			}
			m >>= 1;
			++x;
		}
		old = l;
		return e;
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
				e += _ctxset->getContext(x)->advance(1, r);
				r = r; // avoid warning until we figure out what to do with result
			}
			m >>= 1;
			++x;
		}
		return e;
	}

	/// \brief Arm the MU interrupt-through-Wakeup Unit
	///
	inline void __armMU_WU() {
		WU_ArmMU(0x0f);
	}

	/// \brief Disarm the MU interrupt-through-Wakeup Unit
	///
	inline void __disarmMU_WU() {
		WU_DisarmMU(0x0f);
	}

public:
	BgqCommThread(BgqWakeupRegion *wu, BgqContextPool *pool, size_t num_ctx) :
	_wakeup_region(wu),
	_ctxset(pool),
	_thread(0),
	_falseWU(0),
	_shutdown(false)
	{ }

	~BgqCommThread() { }


	static void *commThread(void *cookie) {
		BgqCommThread *thus = (BgqCommThread *)cookie;
		pami_result_t r = thus->__commThread();
		r = r; // avoid warning until we decide how to use result
		return NULL;
	}

	static void commThreadSig(int sig) {
		// BgqCommThread *thus = (BgqCommThread *)???;
		// thus->__commThreadSig(sig);
		// force us to highest priority so we can exit...
		pthread_t self = pthread_self();
		int max_pri = sched_get_priority_max(COMMTHREAD_SCHED);
//fprintf(stderr, "Bop %ld\n", self);
		pthread_setschedprio(self, max_pri);
	}

	static inline void initContext(size_t clientid,
					size_t contextid, pami_context_t context) {
		// might need hook later, to do per-context initialization?
	}

#if 0
	static void balanceThreads(uint32_t core, uint32_t &newcore, uint32_t &newthread) {

		uint64_t mask = __CommThreadGlobal.availThreads(core);



		uint32_t c = core;
		uint32_t m = (c > _ptCore ? _ptThread + 1 : _ptThread);
		if (!m) {
			uint32_t j;
			for (c = j = 16 - 1; j > _ptCore; --j) {
				if (_core_iter[j] < _core_iter[c]) c = j;
			}
			m = _ptThread + 1;
		}
		newthread = (NUM_SMT - 1) - (_core_iter[c] % m);
		newcore = c;
		++_core_iter[c];
	}
#endif

	// arrange for the new comm-thread to hunt-down this context and
	// take it, even if some other thread already picked it up.
	// This helps ensure a more-balanced startup, and prevents some
	// complexities of trying to re-balance later. Then, only when
	// a comm-thread leaves the set does there have to be a re-balance.
	static inline pami_result_t addContext(size_t clientid,
					pami_context_t context) {
		int status;
		uint32_t c, t, core;
#if 0
		PAMI::Context *ctx = (PAMI::Context *)context;
		core = __MUGlobal.getMuRM().getAffinity(clientid, ctx->getId());
#else
		static uint32_t __core = NUM_CORES - 1;
		core = __core;
		__core = (__core - 1) % NUM_CORES;
#endif
		BgqCommThread *thus;

		//balanceThreads(core, c, t);
		c = core;
		__CommThreadGlobal.getNextThread(c, t);
		// any error checking?
		thus = _comm_xlat[c][t];

		// all BgqCommThread objects have the same ContextSet object.
		BgqCommThread *devs = __CommThreadGlobal.getCommThreads();
		// this will wakeup existing commthreads...
		// note, unsolvable race condition: we want to add 'm' to
		// the existing commthread's _initCtxs, but can't do that
		// until getting 'm' from call to addContext() which will
		// wakeup commthreads. This means any commthread might get
		// the new context, and the true "home" commthread will
		// have to force it away later. A more-complex two-phased
		// approach would help. But, this is really not much different
		// than a newly created commthread which will have to force
		// its core context(s) back to itself.
		uint64_t m = devs[0]._ctxset->addContext(context);
		if (thus) {
			if (m == 0) {
				return PAMI_EAGAIN; // closest thing to ENOSPC ?
			}
			thus->_initCtxs |= m;
			mem_sync();
			return PAMI_SUCCESS;
		}

		// we assume this is single-threaded so no locking required...
		size_t x = _numActive++;
		thus = &devs[x];
		thus->_initCtxs = m;	// this should never be zero
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
		cpu_set_t cpu_mask;
		CPU_ZERO(&cpu_mask);
		CPU_SET(c * NUM_SMT + t, &cpu_mask);
		status = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpu_mask);
		if (status) {
			pthread_attr_destroy(&attr);
			--_numActive;
			errno = status;
			return PAMI_CHECK_ERRNO;
		}

		thus->_shutdown = false;
		mem_sync();
		status = pthread_create(&thus->_thread, &attr, commThread, thus);
		pthread_attr_destroy(&attr);
		if (status) {
			--_numActive;
			thus->_thread = 0; // may not always be legal assignment?
			errno = status;
			return PAMI_CHECK_ERRNO;
		}
		_comm_xlat[c][t] = thus;
		return PAMI_SUCCESS;
	}

	static inline pami_result_t rmContexts(size_t clientid,
						pami_context_t *ctxs, size_t nctx) {
		size_t x;
		if (_numActive == 0) {
			return PAMI_SUCCESS;
		}
		BgqCommThread *devs = __CommThreadGlobal.getCommThreads();

		// all BgqCommThread objects have the same ContextSet object.

		// This should wakeup all commthreads, and any one holding
		// this context should release it...
		uint64_t mask = devs[0]._ctxset->disableContexts(ctxs, nctx);
		for (x = 0; x < _numActive; ++x) {
			devs[x]._initCtxs &= ~mask;
		}

		// wait here for all contexts to get released? must only
		// wait for all commthreads to release, not for other threads
		// that might have the context locked - how to tell?
		// Maybe the caller knows best?

		// wait for commthreads to unlock contexts we removed.
		// is this guaranteed to complete?
		// is this guaranteed not to race with locker?
		// do we need some sort of "generation counter" barrier?
		uint64_t lmask;
		do {
			lmask = 0;
			for (x = 0; x < _numActive; ++x) {
				lmask |= devs[x]._lockCtxs;
			}
		} while (lmask & mask);

		devs[0]._ctxset->rmContexts(ctxs, nctx);

		return PAMI_SUCCESS;
	}

private:
	inline pami_result_t __commThread() {
		// should/can this use the internal (C++) interface?
		uint64_t new_ctx, old_ctx, lkd_ctx;
		size_t n, events, ev_since_wu;
		size_t max_loop = 100; // \todo need some heuristic or tunable for max loops
		size_t id; // our current commthread id, among active ones.
		pthread_t self = pthread_self();
		uint64_t wu_start, wu_mask;
		int min_pri = sched_get_priority_min(COMMTHREAD_SCHED);
		int max_pri = sched_get_priority_max(COMMTHREAD_SCHED);
DEBUG_INIT();

		pthread_setschedprio(self, max_pri);
		struct sigaction sigact;
		sigact.sa_handler = commThreadSig;
		sigemptyset(&sigact.sa_mask);
		sigact.sa_flags = 0;
		sigaction(SIGUSR1, &sigact, NULL);
		_ctxset->joinContextSet(id, _initCtxs);
//fprintf(stderr, "comm thread %ld for context %04zx\n", self, _initCtxs);
DEBUG_WRITE('c','t');
		new_ctx = old_ctx = lkd_ctx = 0;
		ev_since_wu = 0;
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
				new_ctx = _ctxset->getContextSet(id);

				// this only locks/unlocks what changed...
				events += __lockContextSet(lkd_ctx, new_ctx);
				_lockCtxs = lkd_ctx;
				if (old_ctx != new_ctx) ev_since_wu += 1;
				old_ctx = new_ctx;
				mem_sync();
				events += __advanceContextSet(lkd_ctx);
				ev_since_wu += events;
			} while (!_shutdown && lkd_ctx && events != 0 && ++n < max_loop);
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
					if (ev_since_wu == 0 && lkd_ctx) ++_falseWU;
DEBUG_WRITE('g','i');
					ppc_waitimpl();
DEBUG_WRITE('w','u');
					ev_since_wu = 0;
					if (_shutdown) break;
				}
				// need to re-evaluate things here?
				// goto re_evaluate;
				// ... or just go back and do work?
				goto more_work;
			} else {
				__disarmMU_WU();

				// this only locks/unlocks what changed...
				(void)__lockContextSet(lkd_ctx, 0);
				_lockCtxs = lkd_ctx;
				mem_sync();

				_ctxset->leaveContextSet(id); // id invalid now
DEBUG_WRITE('s','a');

				pthread_setschedprio(self, min_pri);
				//=== we get preempted here ===//
				pthread_setschedprio(self, max_pri);
DEBUG_WRITE('s','b');

				if (_shutdown) break;
				_ctxset->joinContextSet(id, _initCtxs); // got id
				// always assume context set changed... just simpler.
				goto new_context_assignment;
			}
		}

		if (lkd_ctx) {
			(void)__lockContextSet(lkd_ctx, 0);
			_lockCtxs = lkd_ctx;
			mem_sync();
		}
		if (id != (size_t)-1) {
			_ctxset->leaveContextSet(id); // id invalid now
		}
DEBUG_WRITE('t','t');
		return PAMI_SUCCESS;
	}

	friend class PAMI::Device::CommThread::Factory;
	BgqWakeupRegion *_wakeup_region;	///< WAC memory for contexts (common)
	PAMI::Device::CommThread::BgqContextPool *_ctxset; ///< context set (common)
	volatile uint64_t _initCtxs;	///< initial set of contexts to take
	volatile uint64_t _lockCtxs;	///< set of contexts we have locked
	pthread_t _thread;		///< pthread identifier
	volatile size_t _falseWU;	///< perf counter for false wakeups
	volatile bool _shutdown;	///< request commthread to exit
	static BgqCommThread *_comm_xlat[NUM_CORES][NUM_SMT];
	static size_t _numActive;
	static size_t _maxActive;
	static size_t _ptCore;
	static size_t _ptThread;
}; // class BgqCommThread

// Called from __global...
//
// This isn't really a device, only using this for convenience but
//
Factory::Factory(PAMI::Memory::MemoryManager *genmm,
			PAMI::Memory::MemoryManager *l2xmm) :
_commThreads(NULL)
{
	BgqCommThread *devs;
	BgqWakeupRegion *wu;
	BgqContextPool *pool;

#if 0
	size_t num_ctx = __MUGlobal.getMuRM().getPerProcessMaxPamiResources();
	// may need to factor in others such as shmem?
#else
	size_t num_ctx = 64 / Kernel_ProcessCount();
#endif

	size_t x;
	size_t me = __global.topology_local.rank2Index(__global.mapping.task());
	size_t lsize = __global.topology_local.size();

	BgqCommThread::_maxActive = Kernel_ProcessorCount() - 1;
	// config param may also affect this?

	BgqCommThread::_ptCore = (NUM_CORES - 1) - (BgqCommThread::_maxActive % NUM_CORES);
	BgqCommThread::_ptThread = BgqCommThread::_maxActive / NUM_CORES;

	posix_memalign((void **)&devs, 16, BgqCommThread::_maxActive * sizeof(*devs));
	posix_memalign((void **)&pool, 16, sizeof(*pool));
	posix_memalign((void **)&wu, 16, sizeof(*wu)); // one per client

	new (wu) BgqWakeupRegion();
	pami_result_t rc = wu->init(num_ctx, me, lsize, l2xmm);
	if (rc != PAMI_SUCCESS) {
		PAMI_abortf("Failed to initialize BgqWakeupRegion - not enough shared memory?");
	}
	__global._wuRegion_mm = wu->getWUmm();

	new (pool) BgqContextPool();
	pool->init(BgqCommThread::_maxActive, num_ctx, genmm, wu->getWUmm());

	for (x = 0; x < BgqCommThread::_maxActive; ++x) {
		new (&devs[x]) BgqCommThread(wu, pool, num_ctx);
	}
	_commThreads = devs;

	// determine what range of hwthreads we have...
	// Try to generalize, and just use the bitmap as
	// a set of "available" threads, zero each one as we
	// start a commthread on it...  also take into account
	// the "main" thread, where we never start a commthread.
	uint64_t tmask = Kernel_ThreadMask(__global.mapping.t());
	_proc_threads = tmask;
	_comm_threads = 0; // not used?
	memset(_num_used, 0, sizeof(_num_used));
	memset(_num_avail, 0, sizeof(_num_avail));
	memset(_first, 0, sizeof(_first));

	// Assume the calling thread is main(), remove it from the mask.
	tmask &= ~(1UL << ((NUM_CORES * NUM_SMT - 1) - Kernel_ProcessorID()));
	_avail_threads = tmask;
	int k, c;
	uint32_t m;
	static int __num_bits[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
	for (k = 0; k < NUM_CORES; ++k) {
		c = ((NUM_CORES - 1) - k) * NUM_SMT;
		m = (tmask >> c) & 0x0f;
		if (m) {
			_num_avail[k] = __num_bits[m];
			_first[k] = ffs(m) - 1;
		}
	}
}

Factory::~Factory() {
	size_t x;

	if (BgqCommThread::_numActive == 0) {
		return;
	}

	// assert _commThreads[0]._ctxset->_nactive == 0...
	// will that be true? commthreads with no contexts will
	// still be in the waitimpl loop. We really should confirm
	// that all commthreads have reached the "zero contexts"
	// state, though. In that case, the commthread needs to
	// give some feedback telling us how many contexts it
	// thinks it has.

	for (x = 0; x < BgqCommThread::_numActive; ++x) {
		_commThreads[x]._shutdown = true;
		// touching _shutdown will not directly wakeup commthreads,
		// so must do something... tbd

		// If the target commthread is in waitimpl, we can
		// easily wake it.

		// If the thread is in pthread_setschedprio(min)
		// then it has released all locks and may be safely
		// terminated.

		// However, we can't tell which case applies.
		// If we can just get both cases to wake up, the
		// thread will notice the shutdown and exit.
		// But if it is in setsched the problem is that
		// most likely some other thread has preempted it
		// and so the pthread_join below may hang(?)
		// Maybe we don't really care about the pthread join?
		// We do need it for statistics gathering, though.

		// Note, should not get here unless all clients/contexts
		// have been destroyed, so in that case the commthreads
		// can just be terminated - they are all totally inactive.
//fprintf(stderr, "pthread_kill(%ld, SIGUSR1);\n", _commThreads[x]._thread);
		pthread_kill(_commThreads[x]._thread, SIGUSR1);
	}

	// need to pthread_join() here? or is it too risky (might hang)?
	size_t fwu = 0;
	for (x = 0; x < BgqCommThread::_numActive; ++x) {
		void *status;
//fprintf(stderr, "pthread_join(%ld, &status);\n", _commThreads[x]._thread);
		pthread_join(_commThreads[x]._thread, &status);
		fwu += _commThreads[x]._falseWU;
	}
	BgqCommThread::_numActive = 0;
fprintf(stderr, "Commthreads saw %zd false wakeups\n", fwu);
}

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

size_t PAMI::Device::CommThread::BgqCommThread::_numActive = 0;
size_t PAMI::Device::CommThread::BgqCommThread::_maxActive = 0;
size_t PAMI::Device::CommThread::BgqCommThread::_ptCore = 0;
size_t PAMI::Device::CommThread::BgqCommThread::_ptThread = 0;
PAMI::Device::CommThread::BgqCommThread *PAMI::Device::CommThread::BgqCommThread::_comm_xlat[NUM_CORES][NUM_SMT] = {{NULL}};

#undef DEBUG_INIT
#undef DEBUG_WRITE

#endif // __components_devices_bgq_commthread_CommThread_h__
