/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __xmi_bgp_lwarxstwcxbarrier_h__
#define __xmi_bgp_lwarxstwcxbarrier_h__

/**
 * \file components/atomic/bgp/LwarxStwcxBarrier.h
 * \brief LockBox implementation of a Node-scoped Barrier
 * This implementation is different from, and incompatible with,
 * the BGP SPI LockBox_Barrier implementation. Support for core
 * granularity barriers is added here, in a way that makes the
 * barrier code common for all granularities.
 */
#include "components/sysdep/SysDep.h"
#include "components/atomic/Barrier.h"
#include <bpcore/bgp_atomic_ops.h>

namespace XMI {
namespace Atomic {
namespace BGP {
	/*
	 */
	class _LwarxStwcxNodeBarrier {
	private:
		struct LwarxStwcx_Barrier_s {
			union {
				_BGP_Atomic atomics[5]; /**< access all 5 lockboxes */
				struct {
					_BGP_Atomic ctrl_lock; /**< the control lockbox (phase) */
					_BGP_Atomic lock[2]; /**< lock lockboxes for all phases */
					_BGP_Atomic status[2]; /**< status lockboxes for all phases */
				} lwx_s;
			} lwx_u;
			uint8_t master;    /**< master participant */
			uint8_t coreshift; /**< convert core to proc */
			uint8_t nparties;  /**< number of participants */
			uint8_t _pad[5];   /**< pad to long */
		};
#define lwx_atomics      lwx_u.atomics           /**< shortcut for atomics */
#define lwx_ctrl_lock    lwx_u.lwx_s.ctrl_lock   /**< shortcut for ctrl_lock */
#define lwx_lock         lwx_u.lwx_s.lock        /**< shortcut for lock */
#define lwx_status       lwx_u.lwx_s.status      /**< shortcut for status */

	public:
		_LwarxStwcxNodeBarrier() { }
		~_LwarxStwcxNodeBarrier() { }

		inline void init_impl() {
			XMI_abortf("_LwarxStwcxNodeBarrier must be subclass");
		}

		inline void dump_impl(const char *string) {
			fprintf(stderr, "%d: %p %s status=%d master=%d parties=%d core=%d [%d] %d %d %d %d %d\n",
				Kernel_PhysicalProcessorID(), _barrier, string,
				__status,
				_barrier->master, _barrier->nparties,
				_barrier->coreshift, (uint32_t)_data,
				_barrier->lwx_u.atomics[0].atom,
				_barrier->lwx_u.atomics[1].atom,
				_barrier->lwx_u.atomics[2].atom,
				_barrier->lwx_u.atomics[3].atom,
				_barrier->lwx_u.atomics[4].atom);
		}

		inline void enter_impl() {
			pollInit();
			while (poll() != Done);
		}

		inline void enterPoll_impl(pollFcn fcn, void *arg) {
			pollInit();
			while (poll() != Done) {
				fcn(arg);
			}
		}

		inline void pollInit_impl() {
			uint32_t lockup;
			lockup = _barrier->lwx_ctrl_lock.atom;
			_bgp_fetch_and_add(&_barrier->lwx_lock[lockup], 1);
			_data = (void*)lockup;
			__status = Entered;
		}

		inline DCMF::barrierPollStatus poll_impl() {
			DCMF_assert(__status == Entered);
			uint32_t lockup, value;
			lockup = (uint32_t)_data;
			if (_barrier->lwx_lock[lockup].atom < _barrier->nparties) {
				return Entered;
			}

			// All cores have participated in the barrier
			// We need all cores to block until checkin
			// to clear the lock atomically
			_bgp_fetch_and_add(&_barrier->lwx_lock[lockup], 1);
			do {
				value = _barrier->lwx_lock[lockup].atom;
			} while (value > 0 && value < (uint32_t)(2 * _barrier->nparties));

			if ((Kernel_PhysicalProcessorID() >> _barrier->coreshift) == _barrier->master) {
				if (lockup) {
					_bgp_fetch_and_add(&_barrier->lwx_ctrl_lock, -1);
				} else {
					_bgp_fetch_and_add(&_barrier->lwx_ctrl_lock, 1);
				}
				_barrier->lwx_status[lockup].atom = 0;
				_barrier->lwx_lock[lockup].atom = 0;
			} else {
				// wait until master releases the barrier by clearing the lock
				while (_barrier->lwx_lock[lockup].atom > 0);
			}
			__status = Initialized;
			return Done;
		}
		// With 5 lockboxes used... which one should be returned?
		inline void *returnBarrier_impl() { return &_barrier->lwx_ctrl_lock; }
	private:
		LwarxStwcx_Barrier_s *_barrier;
		void *_data;
		static inline void compile_time_assert() {
			COMPILE_TIME_ASSERT(sizeof(LwarxStwcxNodeBarrier) <= LM_MAX_SIZEOF_BARRIER);
		}
	}; // class _LwarxStwcxNodeBarrier

	class LwarxStwcxNodeProcBarrier : public _LwarxStwcxNodeBarrier {
	public:
		LwarxStwcxNodeProcBarrier() : _LwarxStwcxNodeBarrier() {}
		~LwarxStwcxNodeProcBarrier() {}

		inline void init_impl() {
			// For proc-granularity, must convert
			// between core id and process id,
			// and only one core per process will
			// participate.
			_barrier->master = lm->coreXlat[lm->masterProc] >> lm->coreShift;
			_barrier->coreshift = lm->coreShift;
			_barrier->nparties = lm->numProc;
			__status = Initialized;
		}
	}; // class LwarxStwcxNodeProcBarrier

	class LwarxStwcxNodeCoreBarrier : public _LwarxStwcxNodeBarrier {
	public:
		LwarxStwcxNodeCoreBarrier() : _LwarxStwcxNodeBarrier() {}
		~LwarxStwcxNodeCoreBarrier() {}

		inline void init_impl() {
			// For core-granularity, everything is
			// a core number. Assume the master core
			// is the lowest-numbered core in the
			// process.
			_barrier->master = lm->masterProc << lm->coreShift;
			_barrier->coreshift = 0;
			_barrier->nparties = lm->numCore;
			__status = Initialized;
		}
	}; // class LwarxStwcxNodeCoreBarrier

}; // BGP namespace
}; // Atomic namespace
}; // XMI namespace

#endif // __xmi_bgp_lwarxstwcxbarrier_h__
