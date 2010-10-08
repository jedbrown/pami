/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/multisend/memorymanager.h
 * \brief Common routines and items for concurrency tests
 */

#ifndef __test_internals_multisend_memorymanager_h__
#define __test_internals_multisend_memorymanager_h__

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "Global.h"
#include "components/memory/MemoryManager.h"

#define TEST_DEF_SHMEM_SIZE	(32*1024)

// re-initializes mm on subsequent calls. Caller ensures/expects size to be the same.
//
static inline pami_result_t initializeMemoryManager(const char *name, size_t bytes,
                                        PAMI::Memory::MemoryManager &mm,
					size_t numparts = 0, bool ismaster = false) {
        static size_t _bytes = 0;
        static void *_ptr = NULL;
        char shmemfile[PAMI::Memory::MMKEYSIZE];

        if (!_ptr) {
		
                snprintf(shmemfile, sizeof(shmemfile) - 1, "/pami-test-%s", name);

        	mm.init(__global.shared_mm, bytes, 1, 1, 0, shmemfile);
		_ptr = mm.base();
		_bytes = mm.size();

        } else {
                // assert(bytes == _bytes || bytes == 0);
        }
	if (numparts != 0) {
		if (numparts > 1) {
			local_barriered_shmemzero(_ptr, _bytes, numparts, ismaster);
		} else {
			memset(_ptr, 0, _bytes);
		}
	}
	// need to "free" everything and reset...
        mm.reset(true);

        return PAMI_SUCCESS;
}

#endif
