/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/BG/bgq/commthreads/misc-util.cc
 * \brief ???
 */

#include "Global.h"
#include "misc-util.h"

pami_task_t TEST_Global_index2task(size_t index) {
	return __global.topology_global.index2Rank(index);
}

size_t TEST_Global_size() {
	return __global.topology_global.size();
}

size_t TEST_Global_myindex() {
	return __global.topology_global.rank2Index(__global.mapping.task());
}

pami_task_t TEST_Local_index2task(size_t index) {
	return __global.topology_local.index2Rank(index);
}

size_t TEST_Local_size() {
	return __global.topology_local.size();
}

size_t TEST_Local_myindex() {
	return __global.topology_local.rank2Index(__global.mapping.task());
}
