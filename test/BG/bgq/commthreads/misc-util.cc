#include "Global.h"
#include "misc-util.h"

pami_task_t TEST_Local_index2task(size_t index) {
	return __global.topology_local.index2Rank(index);
}
