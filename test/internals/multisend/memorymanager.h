#include <stdlib.h>
#include "components/memory/MemoryManager.h"

inline xmi_result_t initializeMemoryManager(const char *name, size_t bytes,
					XMI::Memory::MemoryManager &mm) {
	char shmemfile[1024];

	snprintf(shmemfile, 1023, "/xmi-test-%s", name);

	// Round up to the page size
	//size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

	int fd, rc;
	size_t n = bytes;

	// CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
	rc = shm_open(shmemfile, O_CREAT | O_RDWR, 0600);

	if (rc != -1) {
		fd = rc;
		rc = ftruncate(fd, n);

		if (rc != -1) {
			void *ptr = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

			if (ptr != MAP_FAILED) {
				mm.init(ptr, n);
				return XMI_SUCCESS;
			}
		}
	}

	// Failed to create shared memory .. fake it using the heap ??
	mm.init(malloc(n), n);

	return XMI_SUCCESS;
}
