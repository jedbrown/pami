/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/BG/bgq/commthreads/misc-util.h
 * \brief ???
 */

#ifndef __test_internals_BG_bgq_commthreads_misc_util_h__
#define __test_internals_BG_bgq_commthreads_misc_util_h__

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
