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
#include "components/memory/MemoryManager.h"

#define TEST_DEF_SHMEM_SIZE	(32*1024)

// re-initializes mm on subsequent calls. Caller ensures/expects size to be the same.
//
static inline pami_result_t initializeMemoryManager(const char *name, size_t bytes,
                                        PAMI::Memory::MemoryManager &mm) {
        static size_t _bytes = 0;
        static void *_ptr = NULL;
        char shmemfile[1024];

        if (!_ptr) {
                snprintf(shmemfile, 1023, "/pami-test-%s", name);

                // Round up to the page size
                //size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

                int fd, rc;
                size_t n = bytes;
                void *ptr;

                // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
                rc = shm_open(shmemfile, O_CREAT | O_RDWR, 0600);
                if (rc != -1) {
                        fd = rc;
                        rc = ftruncate(fd, n);
                        if (rc != -1) {
                                ptr = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                                if (ptr == MAP_FAILED) {
                                        rc = -1;
                                }
                        }
                }
                if (rc == -1) {
                        // Failed to create shared memory .. fake it using the heap ??
                        // assume SMP mode (?)
                        ptr = malloc(n);
                        // check for failure???
                }
                _ptr = ptr;
                _bytes = n;
        } else {
                // assert(bytes == _bytes || bytes == 0);
        }
        mm.init(_ptr, _bytes);

        return PAMI_SUCCESS;
}

#endif
