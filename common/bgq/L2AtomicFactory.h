/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __common_bgq_L2AtomicFactory_h__
#define __common_bgq_L2AtomicFactory_h__

// not sure what this should be yet...
#define MapL2AtomicRegion(v)	((uintptr_t)v)

// These define the range of lockboxes we're allowed to use.
#define L2A_MIN_L2ATOMIC	0
#define L2A_MAX_NUML2ATOMIC	256

////////////////////////////////////////////////////////////////////////
///  \file common/bgq/L2AtomicFactory.h
///  \brief Implementation of BGQ AtomicFactory scheme(s).
///
///  This object is a portability layer that implements allocation
///  of lockboxes for use in Mutexes, Barriers, and Atomic (counters).
///
///  Namespace:  XMI, the messaging namespace
///  Notes:  This is currently indended for use only by the lock manager
///
////////////////////////////////////////////////////////////////////////
namespace XMI {
namespace Atomic {
namespace BGQ {
	// These may need to be put in a (more) common header... somewhere...
	typedef enum {
		L2A_NODE_SCOPE,
		L2A_NODE_PROC_SCOPE,
		L2A_NODE_PTHREAD_SCOPE,
		L2A_NODE_CORE_SCOPE,
		L2A_NODE_SMT_SCOPE,

		L2A_PROC_SCOPE,
		L2A_PROC_CORE_SCOPE,
		L2A_PROC_SMT_SCOPE,
		L2A_PROC_PTHREAD_SCOPE,

		L2A_CORE_SCOPE,
		L2A_CORE_SMT_SCOPE,

		L2A_SMT_SCOPE,

		L2A_PTHREAD_SCOPE,
	} lbx_scope_t;
	/**
	 * \brief Structure used to pass implementation parameters
	 */
	struct atomic_factory_t {
		unsigned myProc;	/**< current process number */
		unsigned masterProc;	/**< chosen master process */
		int numProc;		/**< num active processes */
		int numCore;		/**< num active cores */
		unsigned coreXlat[NUM_CORES]; /**< translate process to core */
		int coreShift;		/**< translate core to process */
	};
	static const int MAX_NUML2ATOMICS = 5;

	class L2AtomicFactory {
	private:
		/// \brief Storage for the implementation parameters
		atomic_factory_t _factory;
		size_t __masterRank;
		int __numProc;
		bool __isMasterRank;
		struct {
			size_t size;
			void *virt;
			uintptr_t arena;
			size_t next;
		} _l2atomic;
	public:
		L2AtomicFactory(XMI::BgqSysDep *sd) {
			// Must coordinate with all other processes on this node,
			// and arrive at a common chunk of physical address memory
			// which we all will use for allocating "lockboxes" from.
			// One sure way to do this is to allocate shared memory.
			_l2atomic.size = sizeof(uint64_t) * L2A_MAX_NUML2ATOMIC;
			_l2atomic.virt = sd->memoryManager().scratchpad_dynamic_area_malloc(_l2atomic.size);
			XMI_assertf(_l2atomic.virt, "Out of shared memory in L2AtomicFactory");
			_l2atomic.arena = MapL2AtomicRegion(_l2atomic.virt);
			_l2atomic.next = 0;
			// ...something like that...

			// Compute all implementation parameters,
			// i.e. fill-in _factory struct.
			XMI_Coord_t coord;
			int ranks[4];
			int i;
			int ncores = Kernel_ProcessorCount();
			int t = __global.mapping.vnpeers(ranks);
			_factory.numCore = 0;
			_factory.numProc = 0;
			_factory.masterProc = (unsigned)-1;
			//
			// t | VN core | DUAL | SMP |
			//---+---------+------+-----+---
			// 0 |    0    |  0   |  0  |
			// 1 |    1    |  2   |  -  |
			// 2 |    2    |  -   |  -  |
			// 3 |    3    |  -   |  -  |
			//
			int shift = (ncores == 4 ? 2 : (ncores == 2 ? 1 : 0));
			_factory.coreShift = shift;
			for (i = 0; i < t; ++i) {
				if (ranks[i] >= 0) {
					_factory.numCore += ncores;
					++_factory.numProc;
					__global.mapping.rank2Network((size_t)ranks[i], &coord, XMI_TORUS_NETWORK);
					_factory.coreXlat[i] = coord.u.torus.t << shift;
					if ((size_t)ranks[i] == __global.mapping.rank()) {
						_factory.myProc = i;
					}
					if (_factory.masterProc == (unsigned)-1) {
						__masterRank = ranks[i];
						_factory.masterProc = i;
					}
				}
			}
			__numProc = _factory.numProc;
			__isMasterRank = (__masterRank == __global.mapping.rank());
		}

		~L2AtomicFactory() {}

		/**
		 * \brief Generic L2Atomic allocation
		 *
		 *
		 * Warning: this allocation scheme must be used in the same way,
		 * same sequence, on every process in the node. This is the only
		 * way to ensure that each process has the same lockbox(es) for
		 * a given object. This is why all lockboxes (mutexes, barriers, ...)
		 * are allocated all at once at init time.
		 *
		 * When process-scoped lockboxes are requested, each
		 * process on the node will request a different lockbox
		 * number. This is accomplished by adding
		 * numL2Atomics * lm->myProc to the desired lockbox
		 * number and multiplying lockSpan by lm->numProc.
		 * In this way, each process will be asking for different
		 * lockboxes, even if they have to iterate to find a
		 * free lockbox (set). In order to ensure all processes
		 * stay in agreement on the next lockbox number, after
		 * allocation the (next) lockbox number will have
		 * numL2Atomics * lm->myProc subtracted from it.
		 *
		 * Only one core from each process can allocate
		 * lockboxes, and it must be the lowest-numbered
		 * core of the process.  This is to ensure that
		 * Kernel_AllocateLockBox() can properly handle the
		 * LOCKBOX_ORDERED_ALLOC flag.
		 *
		 * \param[in] lm	Implementation parameters
		 * \param[out] p	Place to record lockbox id(s)
		 * \param[in] numL2Atomics Number of lockboxes to get
		 * \param[in] scope	Scope of lockboxes
		 */
		static inline void lbx_alloc(void **p, int numL2Atomics,
					lbx_scope_t scope) {
			static int desiredLock = L2A_MIN_L2ATOMIC;
			static uint64_t *lockp[NUM_CORES * MAX_NUML2ATOMICS];
			int lockSpan = numL2Atomics;
			XMI_assert_debug(numL2Atomics <= MAX_NUML2ATOMICS);
			switch(scope) {
			case L2A_NODE_SCOPE:
			case L2A_NODE_PROC_SCOPE:
			case L2A_NODE_CORE_SCOPE:
				// Node-scoped lockboxes...
				local_barrier(_factory.coreXlat[_factory.masterProc], _factory.numProc);
				break;
			case L2A_PROC_SCOPE:
				// Process-scoped lockboxes...
				// Allocate the entire block on all processes, but
				// only return our specific lock(s).
				// ensure all get different lockboxes
				lockSpan *= _factory.numProc;
				break;
			default:
				XMI_abortf("Invalid lockbox scope");
				break;
			}
			*p = NULL;
			int rc = 0;
			while (desiredLock < L2A_MAX_NUML2ATOMIC) {
				// this can't be right...
				for (x = 0; x < lockSpan; ++x) {
					lockp[x] = _l2atomic.arena++ + (_l2atomic.next * sizeof(uintptr_t))
				}
				rc = 0;
				desiredLock += lockSpan;
				if (rc == EAGAIN) {
					continue;
				} else if(rc != 0) {
					perror("Kernel_AllocateLockBox");
					XMI_abortf("Fatal: allocLockBoxes(%d) rc=%d p=%p", desiredLock-1, rc, p);
				}
				switch(scope) {
				case L2A_NODE_SCOPE:
				case L2A_NODE_PROC_SCOPE:
				case L2A_NODE_CORE_SCOPE:
					// Node-scoped lockboxes...
					// we get exactly what we asked for.
					local_barrier(_factory.coreXlat[_factory.masterProc], _factory.numProc);
					memcpy(p, &lockp[0], numL2Atomics * sizeof(*p));
					break;
				case L2A_PROC_SCOPE:
					// Process-scoped lockboxes...
					// Take our specific lock out of the entire block.
					memcpy(p, &lockp[numL2Atomics * _factory.myProc], numL2Atomics * sizeof(*p));
					break;
				default:
					XMI_abortf("Unsupported lockbox scope");
					break;
				}
				return; // success.
			}
			XMI_abortf("Fatal: Kernel_AllocateLockBox: no available lockboxes");
		}
	}; // class L2AtomicFactory

}; // namespace BGQ
}; // namespace Atomic
}; // namespace XMI

#endif // __xmi_bgq_l2atomicfactory_h__
