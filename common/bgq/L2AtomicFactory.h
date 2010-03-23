/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __common_bgq_L2AtomicFactory_h__
#define __common_bgq_L2AtomicFactory_h__

#include "Mapping.h"
#include "Topology.h"

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
	} l2x_scope_t;
	/**
	 * \brief Structure used to pass implementation parameters
	 */
	struct atomic_factory_t {
		size_t myProc;		/**< current process number */
		size_t masterProc;	/**< chosen master process */
		size_t numProc;		/**< num active processes */
		size_t numCore;		/**< num active cores */
		size_t coreXlat[NUM_CORES]; /**< translate process to core */
		size_t coreShift;	/**< translate core to process */
	};
	static const int MAX_NUML2ATOMICS = 5;

	class L2AtomicFactory {
	private:
		/// \brief Storage for the implementation parameters
		atomic_factory_t _factory;
		xmi_task_t __masterRank;
		size_t __numProc;
		bool __isMasterRank;
		struct {
			size_t size;
			void *virt;
			uintptr_t arena;
			size_t next;
		} _l2atomic;
	public:
		L2AtomicFactory() { }

		inline void init( /* XMI::Memory::MemoryManager *mm,  */
				XMI::Mapping *mapping, XMI::Topology *local) {
			xmi_result_t rc;
                        /** \todo #warning must figure out L2 Atomic Factory memory management... */
#if 0
			// Must coordinate with all other processes on this node,
			// and arrive at a common chunk of physical address memory
			// which we all will use for allocating "lockboxes" from.
			// One sure way to do this is to allocate shared memory.
			_l2atomic.size = sizeof(uint64_t) * L2A_MAX_NUML2ATOMIC;
			mm->memalign((void **)&_l2atomic.virt, 8, _l2atomic.size);
			XMI_assertf(_l2atomic.virt, "Out of shared memory in L2AtomicFactory");
			_l2atomic.arena = (uintptr_t) _l2atomic.virt; //MapL2AtomicRegion(_l2atomic.virt);
			_l2atomic.next = 0;
			// ...something like that...
#endif

			// Compute all implementation parameters,
			// i.e. fill-in _factory struct.
			XMI::Interface::Mapping::nodeaddr_t addr;
			xmi_task_t ranks[NUM_CORES];
			size_t i;

                        /** \todo #warning This needs a proper CNK function for number of threads per process, when it exists... */
			int ncores = (64 / Kernel_ProcessCount());
			// int ncores = Kernel_ThreadCount();

			//int t = mapping->vnpeers(ranks);
			// There should be a more elegent way to do this... yuk.
			size_t t = local->size();
			for (i = 0; i < t; ++i) {
				ranks[i] = local->index2Rank(i);
			}

			_factory.numCore = 0;
			_factory.numProc = 0;
			_factory.masterProc = (size_t)-1;
			//
			// t | VN core | DUAL | SMP |
			//---+---------+------+-----+---
			// 0 |    0    |  0   |  0  |
			// 1 |    1    |  2   |  -  |
			// 2 |    2    |  -   |  -  |
			// 3 |    3    |  -   |  -  |
			//
			// assert((ncores & (ncores - 1) == 0);
			int shift = ffs(ncores);
			XMI_assertf(shift > 0, "Internal error: no cores in process?");
			--shift;
			_factory.coreShift = shift;
			for (i = 0; i < t; ++i) {
				if (ranks[i] >= 0) {
					_factory.numCore += ncores;
					++_factory.numProc;
					rc = mapping->task2node(ranks[i], addr);
					XMI_assertf(rc == XMI_SUCCESS, "[%zd] task2node(%d, addr) failed\n", i, ranks[i]);
					size_t p;
					rc = mapping->node2peer(addr, p);
					XMI_assertf(rc == XMI_SUCCESS, "[%zd] node2peer(addr, p) failed\n", i);
					_factory.coreXlat[i] = p << shift;
					if (ranks[i] == mapping->task()) {
						_factory.myProc = i;
					}
					if (_factory.masterProc == (size_t)-1) {
						__masterRank = ranks[i];
						_factory.masterProc = i;
					}
				}
			}
			__numProc = _factory.numProc;
			__isMasterRank = (__masterRank == mapping->task());
		}

		~L2AtomicFactory() {}

		inline size_t masterProc() { return _factory.masterProc; }
		inline size_t coreShift() { return _factory.coreShift; }
		inline size_t numCore() { return _factory.numCore; }
		inline size_t numProc() { return _factory.numProc; }
		inline size_t coreXlat(size_t x) { return _factory.coreXlat[x]; }
		inline bool isMasterRank() { return __isMasterRank; }

	}; // class L2AtomicFactory

}; // namespace BGQ
}; // namespace Atomic
}; // namespace XMI

#endif // __xmi_bgq_l2atomicfactory_h__
