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
#include "components/memory/MemoryManager.h"
#include "spi/include/kernel/memory.h"

// These define the range of L2Atomics we're allowed to use.
#define L2A_MAX_NUMNODEL2ATOMIC	16*256	///< max number of node-scope atomics
#define L2A_MAX_NUMPROCL2ATOMIC	16*256	///< max number of proc(etc)-scope atomics

////////////////////////////////////////////////////////////////////////
///  \file common/bgq/L2AtomicFactory.h
///  \brief Implementation of BGQ AtomicFactory scheme(s).
///
///  This object is a portability layer that implements allocation
///  of L2Atomics for use in Mutexes, Barriers, and Atomic (counters).
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
	struct atomic_arena_t {
		size_t size;	///< number of atomics
		void *virt;	///< virtual address of memory
		Kernel_MemoryRegion_t memreg;	///< memory region
		uint64_t *base;///< arena base (phys addr of memory)
		size_t next;	///< current number allocated
	};

	class L2AtomicFactory {
	private:
		/// \brief Storage for the implementation parameters
		atomic_factory_t _factory;
		xmi_task_t __masterRank;
		size_t __numProc;
		bool __isMasterRank;
		atomic_arena_t _l2node;
		atomic_arena_t _l2proc;
	public:
		L2AtomicFactory() { }

		inline void init(XMI::Memory::MemoryManager *mm,
				XMI::Mapping *mapping, XMI::Topology *local) {
			xmi_result_t rc;
			uint32_t krc;
			int irc;
                        /** \todo #warning must figure out L2 Atomic Factory memory management... */
			// Must coordinate with all other processes on this node,
			// and arrive at a common chunk of physical address memory
			// which we all will use for allocating "L2Atomics" from.
			// One sure way to do this is to allocate shared memory.
			_l2node.size = L2A_MAX_NUMNODEL2ATOMIC;
			_l2node.virt = NULL;

			rc = mm->memalign((void **)&_l2node.virt, sizeof(uint64_t),
					sizeof(uint64_t) * _l2node.size);
			XMI_assertf(rc == XMI_SUCCESS && _l2node.virt,
				"Failed to get shmem for _l2node, asked size %zd",
				sizeof(uint64_t) * _l2node.size);
			memset(_l2node.virt, 0, sizeof(uint64_t) * _l2node.size);
			krc = Kernel_CreateMemoryRegion(&_l2node.memreg,
							_l2node.virt, _l2node.size);
			XMI_assertf(krc == 0, "Failed to get physical address for L2 Atomic region");
			_l2node.base = (uint64_t *)_l2node.memreg.BasePa;
			// Kernel_DestroyMemoryRegion(&_l2node.memreg); ???
			_l2node.next = 0;

			_l2proc.size = L2A_MAX_NUMPROCL2ATOMIC;
			_l2proc.virt = NULL;
			irc = posix_memalign((void **)&_l2proc.virt, sizeof(uint64_t),
					sizeof(uint64_t) * _l2proc.size);
			XMI_assertf(irc == 0 && _l2proc.virt,
				"Failed to get memory for _l2proc, asked size %zd",
				sizeof(uint64_t) * _l2proc.size);
			memset(_l2proc.virt, 0, sizeof(uint64_t) * _l2proc.size);
			krc = Kernel_CreateMemoryRegion(&_l2proc.memreg,
							_l2proc.virt, _l2proc.size);
			XMI_assertf(krc == 0, "Failed to get physical address for L2 Atomic region");
			_l2proc.base = (uint64_t *)_l2proc.memreg.BasePa;
			// Kernel_DestroyMemoryRegion(&_l2proc.memreg); ???
			_l2proc.next = 0;

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

		/// callers must ensure all use the same order
		inline xmi_result_t l2x_alloc(void **p, int numAtomics, l2x_scope_t scope) {
			int lockSpan = numAtomics;
			atomic_arena_t *arena;
			switch(scope) {
			case L2A_NODE_SCOPE:
			case L2A_NODE_PROC_SCOPE:
			case L2A_NODE_PTHREAD_SCOPE:
			case L2A_NODE_CORE_SCOPE:
			case L2A_NODE_SMT_SCOPE:
				// Node-scoped L2Atomics...
				// barrier... ??
				arena = &_l2node;
				break;
			case L2A_PROC_SCOPE:
			case L2A_PROC_CORE_SCOPE:
			case L2A_PROC_SMT_SCOPE:
			case L2A_PROC_PTHREAD_SCOPE:
				// Process-scoped L2Atomics...
				// Allocate the entire block on all processes, but
				// only return our specific lock(s).
				// ensure all get different L2Atomics
				lockSpan *= _factory.numProc;
				arena = &_l2proc;
				break;
			case L2A_CORE_SCOPE:
			case L2A_CORE_SMT_SCOPE:
				/// \todo what are core-scoped atomics?
				arena = &_l2proc;
				//break;
			case L2A_SMT_SCOPE:
				/// \todo what are smt-scoped atomics?
				arena = &_l2proc;
				//break;
			case L2A_PTHREAD_SCOPE:
				/// \todo what are pthread-scoped atomics?
				arena = &_l2proc;
				//break;
			default:
				XMI_abortf("Invalid L2Atomic scope");
				break;
			}
			*p = NULL;
			if (arena->next + lockSpan > arena->size) {
				return XMI_EAGAIN;
			}
			size_t idx = arena->next;
			arena->next += lockSpan;
			int x;
			switch(scope) {
			case L2A_NODE_SCOPE:
			case L2A_NODE_PROC_SCOPE:
			case L2A_NODE_PTHREAD_SCOPE:
			case L2A_NODE_CORE_SCOPE:
			case L2A_NODE_SMT_SCOPE:
				// Node-scoped L2Atomics...
				// we get exactly what we asked for.
				for (x = 0; x < numAtomics; ++x) {
					p[x] = &arena->base[idx++];
				}
fprintf(stderr, "Got %d NODE atomics at %p\n", numAtomics, p[0]);
				// barrier... ??
				break;
			case L2A_PROC_SCOPE:
			case L2A_PROC_CORE_SCOPE:
			case L2A_PROC_SMT_SCOPE:
			case L2A_PROC_PTHREAD_SCOPE:
				// Process-scoped L2Atomics...
				// Take our specific lock out of the entire block.
				idx += (numAtomics * _factory.myProc);
				for (x = 0; x < numAtomics; ++x) {
					p[x] = &arena->base[idx++];
				}
fprintf(stderr, "Got %d PROC atomics at %p\n", numAtomics, p[0]);
				break;
			case L2A_CORE_SCOPE:
			case L2A_CORE_SMT_SCOPE:
				/// \todo what are core-scoped atomics?
				//break;
			case L2A_SMT_SCOPE:
				/// \todo what are smt-scoped atomics?
				//break;
			case L2A_PTHREAD_SCOPE:
				/// \todo what are pthread-scoped atomics?
				//break;
			default:
				XMI_abortf("Invalid L2Atomic scope");
				break;
			}
			return XMI_SUCCESS;
		}

	}; // class L2AtomicFactory

}; // namespace BGQ
}; // namespace Atomic
}; // namespace XMI

#endif // __xmi_bgq_l2atomicfactory_h__
