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

#define BGQ_WACREGION_SIZE	(64)	// the number of L2 Atomics (uint64_t)

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

// These define the range of L2Atomics we're allowed to use.
// We might need extra space for the WAC Region.
// Must take into account (considerable) waste from large alignment value for WAC,
// Using 2x as an upper-bound.
#define L2A_MAX_NUMNODEL2ATOMIC(nproc,nctx)	(16*256+(nproc)*(nctx)*BGQ_WACREGION_SIZE) ///< max number of node-scope atomics
#define L2A_MAX_NUMPROCL2ATOMIC	(16*256)	///< max number of proc(etc)-scope atomics

////////////////////////////////////////////////////////////////////////
///  \file common/bgq/L2AtomicFactory.h
///  \brief Implementation of BGQ AtomicFactory scheme(s).
///
///  This object is a portability layer that implements allocation
///  of L2Atomics for use in Mutexes, Barriers, and Atomic (counters).
///
///  Namespace:  PAMI, the messaging namespace
///  Notes:  This is currently indended for use only by the lock manager
///
////////////////////////////////////////////////////////////////////////
namespace PAMI {
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
                size_t coreXlat[PAMI_MAX_PROC_PER_NODE]; /**< translate process to core */
                size_t coreShift;	/**< translate core to process */
        };

        class L2AtomicFactory {
        private:
                /// \brief Storage for the implementation parameters
                atomic_factory_t _factory;
                pami_task_t __masterRank;
                size_t __numProc;
                bool __isMasterRank;
        public:
		PAMI::Memory::MemoryManager __nodescoped_mm;
		PAMI::Memory::MemoryManager __procscoped_mm;

                L2AtomicFactory() { }

		/// \brief Initialize the L2AtomicFactory
		///
		/// \param[in] mm	Shmem MemoryManager
		/// \param[in] mapping	Mapping object
		/// \param[in] local	Topology for tasks local to node
		///
                inline void init(PAMI::Memory::MemoryManager *mm,
                                PAMI::Mapping *mapping, PAMI::Topology *local) {
                        pami_result_t rc;
                        int irc;

			/// \todo The must be re-worked for >4G Nodes, since we won't
			/// have enough TLBs to map all of memory for L2 Atomic use.
			if (mapping->task() == 0) {
				fprintf(stderr, "NOTICE: L2AtomicFactory currently "
						"requires MEMSIZE <= 4G\n");
			}

                        // Must coordinate with all other processes on this node,
                        // and arrive at a common chunk of physical address memory
                        // which we all will use for allocating "L2Atomics" from.
                        // One sure way to do this is to allocate shared memory.
			void *virt;
			size_t size;
                        size_t t = local->size();

                        size = L2A_MAX_NUMPROCL2ATOMIC;
                        virt = NULL;
                        irc = posix_memalign(&virt, sizeof(uint64_t),
                                        sizeof(uint64_t) * size);
                        PAMI_assertf(irc == 0 && virt,
                                "Failed to get memory for _l2proc, asked size %zu",
                                sizeof(uint64_t) * size);
			/// \todo need to "register" this memory for use by L2 Atomic Ops
                        memset(virt, 0, sizeof(uint64_t) * size);
			__procscoped_mm.init(virt, size);

                        size = L2A_MAX_NUMNODEL2ATOMIC(t,64);
                        virt = NULL;

                        rc = mm->memalign(&virt, sizeof(uint64_t),
                                        sizeof(uint64_t) * size);
                        PAMI_assertf(rc == PAMI_SUCCESS && virt,
                                "Failed to get shmem for _l2node, asked size %zu",
                                sizeof(uint64_t) * size);
			/// \todo need to "register" this memory for use by L2 Atomic Ops

			/// \todo need to coordinate clearing of shmem unless we know a barrier follows
			// clearing of memory done after computing local params
			__nodescoped_mm.init(virt, size);

                        // Compute all implementation parameters,
                        // i.e. fill-in _factory struct.
                        PAMI::Interface::Mapping::nodeaddr_t addr;
                        pami_task_t ranks[PAMI_MAX_PROC_PER_NODE];
                        size_t i;

                        /** \todo #warning This needs a proper CNK function for number of threads per process, when it exists... */
                        int ncores = (PAMI_MAX_PROC_PER_NODE / Kernel_ProcessCount());
                        // int ncores = Kernel_ThreadCount();

                        //int t = mapping->vnpeers(ranks);
                        // There should be a more elegent way to do this... yuk.
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
                        PAMI_assertf(shift > 0, "Internal error: no cores in process?");
                        --shift;
                        _factory.coreShift = shift;
                        for (i = 0; i < t; ++i) {
                                if (ranks[i] >= 0) {
                                        _factory.numCore += ncores;
                                        ++_factory.numProc;
                                        rc = mapping->task2node(ranks[i], addr);
                                        PAMI_assertf(rc == PAMI_SUCCESS, "[%zu] task2node(%d, addr) failed\n", i, ranks[i]);
                                        size_t p;
                                        rc = mapping->node2peer(addr, p);
                                        PAMI_assertf(rc == PAMI_SUCCESS, "[%zu] node2peer(addr, p) failed\n", i);
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
                        local_barriered_shmemzero(virt, sizeof(uint64_t) * size,
					__numProc, __isMasterRank);
                }

                ~L2AtomicFactory() {}

                inline size_t masterProc() { return _factory.masterProc; }
                inline size_t coreShift() { return _factory.coreShift; }
                inline size_t numCore() { return _factory.numCore; }
                inline size_t numProc() { return _factory.numProc; }
                inline size_t coreXlat(size_t x) { return _factory.coreXlat[x]; }
                inline bool isMasterRank() { return __isMasterRank; }

                /// callers must ensure all use the same order
                inline pami_result_t l2x_alloc(void **p, int numAtomics, l2x_scope_t scope) {
                        int lockSpan = numAtomics;
                        PAMI::Memory::MemoryManager *arena;
                        switch(scope) {
                        case L2A_NODE_SCOPE:
                        case L2A_NODE_PROC_SCOPE:
                        case L2A_NODE_PTHREAD_SCOPE:
                        case L2A_NODE_CORE_SCOPE:
                        case L2A_NODE_SMT_SCOPE:
                                // Node-scoped L2Atomics...
                                // barrier... ??
                                arena = &__nodescoped_mm;
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
                                arena = &__procscoped_mm;
                                break;
                        case L2A_CORE_SCOPE:
                        case L2A_CORE_SMT_SCOPE:
                                /// \todo what are core-scoped atomics?
                                arena = &__procscoped_mm;
                                //break;
                        case L2A_SMT_SCOPE:
                                /// \todo what are smt-scoped atomics?
                                arena = &__procscoped_mm;
                                //break;
                        case L2A_PTHREAD_SCOPE:
                                /// \todo what are pthread-scoped atomics?
                                arena = &__procscoped_mm;
                                //break;
                        default:
                                PAMI_abortf("Invalid L2Atomic scope");
                                break;
                        }
			uint64_t *v = NULL;
			arena->memalign((void **)&v, sizeof(uint64_t),
						lockSpan * sizeof(uint64_t));
			if (v == NULL) {
                                return PAMI_EAGAIN;
                        }
			int i;
                        switch(scope) {
                        case L2A_NODE_SCOPE:
                        case L2A_NODE_PROC_SCOPE:
                        case L2A_NODE_PTHREAD_SCOPE:
                        case L2A_NODE_CORE_SCOPE:
                        case L2A_NODE_SMT_SCOPE:
                                // Node-scoped L2Atomics...
                                // we get exactly what we asked for.
				*p = v;
                                // barrier... ??
                                break;
                        case L2A_PROC_SCOPE:
                        case L2A_PROC_CORE_SCOPE:
                        case L2A_PROC_SMT_SCOPE:
                        case L2A_PROC_PTHREAD_SCOPE:
                                // Process-scoped L2Atomics...
                                // Take our specific lock out of the entire block.
                        	i = (numAtomics * _factory.myProc);
				*p = &v[i];
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
                                PAMI_abortf("Invalid L2Atomic scope");
                                break;
                        }
                        return PAMI_SUCCESS;
                }

        }; // class L2AtomicFactory

}; // namespace BGQ
}; // namespace Atomic
}; // namespace PAMI

#endif // __pami_bgq_l2atomicfactory_h__
