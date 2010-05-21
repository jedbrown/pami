#ifndef __test_BG_bgq_commthreads_misc_util_h__
#define __test_BG_bgq_commthreads_misc_util_h__

#include <sys/pami.h>

#ifdef __cplusplus
extern "C" {
#endif

pami_task_t TEST_Local_index2task(size_t x);

size_t TEST_Local_size();

size_t TEST_Local_myindex();

#ifdef __cplusplus
}
#endif

#endif // __test_BG_bgq_commthreads_misc_util_h__
