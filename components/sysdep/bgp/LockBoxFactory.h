/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __xmi_bgp_lockboxfactory_h__
#define __xmi_bgp_lockboxfactory_h__

#include <spi/bgp_SPI.h>

// These define the range of lockboxes we're allowed to use.
#define LBX_MIN_LOCKBOX		0
#define LBX_MAX_NUMLOCKBOX	256

////////////////////////////////////////////////////////////////////////
///  \file components/sysdep/bgp/LockBoxFactory.h
///  \brief Implementation of BGP AtomicFactory scheme(s).
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
namespace BGP {
	// These may need to be put in a (more) common header... somewhere...
	typedef enum {
		LBX_NODE_SCOPE,
		LBX_NODE_PROC_SCOPE,
		LBX_NODE_CORE_SCOPE,
		LBX_PROC_SCOPE,
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
	static const int MAX_NUMLOCKBOXES = 5;

	class LockBoxFactory {
	private:
		/// \brief Storage for the implementation parameters
		atomic_factory_t _factory;
		size_t __masterRank;
		int __numProc;
		bool __isMasterRank;
	public:
		LockBoxFactory(XMI::BgpSysDep *sd) {
			// Compute all implementation parameters,
			// i.e. fill-in _factory struct.
			XMI_Coord_t coord;
			int ranks[4];
			int i;
			int ncores = Kernel_ProcessorCount();
			int t = sd->mapping().vnpeers(ranks);
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
					sd->mapping().rank2Network((size_t)ranks[i], &coord, XMI_TORUS_NETWORK);
					_factory.coreXlat[i] = coord.torus.t << shift;
					if ((size_t)ranks[i] == sd->mapping().rank()) {
						_factory.myProc = i;
					}
					if (_factory.masterProc == (unsigned)-1) {
						__masterRank = ranks[i];
						_factory.masterProc = i;
					}
				}
			}
			__numProc = _factory.numProc;
			__isMasterRank = (__masterRank == sd->mapping().rank());
		}

		~LockBoxFactory() {}

		/**
		 * \brief Generic LockBox allocation
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
		 * numLockBoxes * lm->myProc to the desired lockbox
		 * number and multiplying lockSpan by lm->numProc.
		 * In this way, each process will be asking for different
		 * lockboxes, even if they have to iterate to find a
		 * free lockbox (set). In order to ensure all processes
		 * stay in agreement on the next lockbox number, after
		 * allocation the (next) lockbox number will have
		 * numLockBoxes * lm->myProc subtracted from it.
		 *
		 * Only one core from each process can allocate
		 * lockboxes, and it must be the lowest-numbered
		 * core of the process.  This is to ensure that
		 * Kernel_AllocateLockBox() can properly handle the
		 * LOCKBOX_ORDERED_ALLOC flag.
		 *
		 * \param[in] lm	Implementation parameters
		 * \param[out] p	Place to record lockbox id(s)
		 * \param[in] numLockBoxes Number of lockboxes to get
		 * \param[in] scope	Scope of lockboxes
		 */
		static inline void lbx_alloc(void **p, int numLockBoxes,
					lbx_scope_t scope) {
			static int desiredLock = LBX_MIN_LOCKBOX;
			static uint32_t *lockp[NUM_CORES * MAX_NUMLOCKBOXES];
			int lockSpan = numLockBoxes;
			unsigned flags = 0;
			XMI_assert_debug(numLockBoxes <= MAX_NUMLOCKBOXES);
			switch(scope) {
			case LBX_NODE_SCOPE:
			case LBX_NODE_PROC_SCOPE:
			case LBX_NODE_CORE_SCOPE:
				// Node-scoped lockboxes...
				flags = (_factory.coreXlat[_factory.masterProc] << 4) | _factory.numProc | LOCKBOX_ORDERED_ALLOC;

				break;
			case LBX_PROC_SCOPE:
				// Process-scoped lockboxes...
				// Allocate the entire block on all processes, but
				// only return our specific lock(s).
				flags = (_factory.coreXlat[_factory.myProc] << 4) | 1;
				// ensure all get different lockboxes
				lockSpan *= _factory.numProc;
				break;
			default:
				XMI_abortf("Invalid lockbox scope");
				break;
			}
			*p = NULL;
			int rc = 0;
			while (desiredLock < LBX_MAX_NUMLOCKBOX) {
				rc = Kernel_AllocateLockBox(desiredLock,
						lockSpan,
						lockp,
						flags);
				desiredLock += lockSpan;
				if (rc == EAGAIN) {
					continue;
				} else if(rc != 0) {
					perror("Kernel_AllocateLockBox");
					XMI_abortf("Fatal: allocLockBoxes(%d) rc=%d p=%p", desiredLock-1, rc, p);
				}
				switch(scope) {
				case LBX_NODE_SCOPE:
				case LBX_NODE_PROC_SCOPE:
				case LBX_NODE_CORE_SCOPE:
					// Node-scoped lockboxes...
					// we get exactly what we asked for.
					memcpy(p, &lockp[0], numLockBoxes * sizeof(*p));
					break;
				case LBX_PROC_SCOPE:
					// Process-scoped lockboxes...
					// Take our specific lock out of the entire block.
					memcpy(p, &lockp[numLockBoxes * _factory.myProc], numLockBoxes * sizeof(*p));
					break;
				default:
					XMI_abortf("Unsupported lockbox scope");
					break;
				}
				return; // success.
			}
			XMI_abortf("Fatal: Kernel_AllocateLockBox: no available lockboxes");
		}
	}; // class LockBoxFactory

}; // namespace BGP
}; // namespace Atomic
}; // namespace XMI

#endif /* __xmi_bgp_lockboxfactory_h__ */
