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

#include "LockManager.h"
#include "AtomicFactory.h"
#include <spi/bgp_SPI.h>

// These define the range of lockboxes we're allowed to use.
#define LBX_MIN_LOCKBOX		0
#define LBX_MAX_NUMLOCKBOX	256

////////////////////////////////////////////////////////////////////////
///  \file sysdep/prod/BGP/LockBoxFactory.h
///  \brief Implementation of BGP AtomicFactory scheme(s).
///
///  This object is a portability layer that implements allocation
///  of lockboxes for use in Mutexes, Barriers, and Atomic (counters).
///
///  Namespace:  DCMF, the messaging namespace
///  Notes:  This is currently indended for use only by the lock manager
///
////////////////////////////////////////////////////////////////////////
namespace XMI
{
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

	class LockBoxFactory : public AtomicFactory {
	public:
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
		static inline void lbx_alloc(atomic_factory_t *lm,
					void **p, int numLockBoxes,
					lm_scope scope) {
			static int desiredLock = LBX_MIN_LOCKBOX;
			static uint32_t *lockp[NUM_CORES * MAX_NUMLOCKBOXES];
			int lockSpan = numLockBoxes;
			unsigned flags = 0;
			DCMF_assert_debug(numLockBoxes <= MAX_NUMLOCKBOXES);
			switch(scope) {
			case LM_NODE:
			case LM_NODE_PROC:
			case LM_NODE_CORE:
				// Node-scoped lockboxes...
				flags = (lm->coreXlat[lm->masterProc] << 4) | lm->numProc | LOCKBOX_ORDERED_ALLOC;
				
				break;
			case LM_PROC:
				// Process-scoped lockboxes...
				// Allocate the entire block on all processes, but
				// only return our specific lock(s).
				flags = (lm->coreXlat[lm->myProc] << 4) | 1;
				// ensure all get different lockboxes
				lockSpan *= lm->numProc;
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
				case LM_NODE:
				case LM_NODE_PROC:
				case LM_NODE_CORE:
					// Node-scoped lockboxes...
					// we get exactly what we asked for.
					memcpy(p, &lockp[0], numLockBoxes * sizeof(*p));
					break;
				case LM_PROC:
					// Process-scoped lockboxes...
					// Take our specific lock out of the entire block.
					memcpy(p, &lockp[numLockBoxes * lm->myProc], numLockBoxes * sizeof(*p));
					break;
				default:
					DCMF_abort();
					break;
				}
				return; // success.
			}
			XMI_abortf("Fatal: Kernel_AllocateLockBox: no available lockboxes");
		}
	}; // class LockBoxFactory

}; // namespace XMI

#endif /* __xmi_bgp_lockboxfactory_h__ */
