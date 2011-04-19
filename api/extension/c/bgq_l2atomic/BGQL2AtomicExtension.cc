/**
 * \file api/extension/c/bgq_l2atomic/bgq/BGQL2AtomicExtension.cc
 * \brief ???
 */
#include "api/extension/c/bgq_l2atomic/BGQL2AtomicExtension.h"

#include "Global.h"

PAMI::BGQL2AtomicExtension::BGQL2AtomicExtension ()
{
}

pami_result_t PAMI::BGQL2AtomicExtension::node_memalign(void **memptr, size_t alignment,
						size_t bytes, const char *key)
{
	return __global.l2atomicFactory.__nodescoped_mm.memalign(memptr,
							alignment, bytes, key);
}

void PAMI::BGQL2AtomicExtension::node_free(void *mem)
{
	__global.l2atomicFactory.__nodescoped_mm.free(mem);
}

pami_result_t PAMI::BGQL2AtomicExtension::proc_memalign(void **memptr, size_t alignment,
						size_t bytes, const char *key)
{
	return __global.l2atomicFactory.__procscoped_mm.memalign(memptr,
							alignment, bytes, key);
}

void PAMI::BGQL2AtomicExtension::proc_free(void *mem)
{
	__global.l2atomicFactory.__procscoped_mm.free(mem);
}
