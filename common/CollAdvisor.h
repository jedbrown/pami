/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/CollAdvisor.h
 * \brief ???
 */

#ifndef __common_CollAdvisor_h__
#define __common_CollAdvisor_h__

#include "sys/pami.h"
#include "util/compact_attributes.h"

#define PAMI_MAX_PROTOCOLS                                                    20

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
  pami_metadata_t metadata;
  pami_algorithm_t alg_id;
} pami_alg_repo;

extern pami_alg_repo *coll_repos[PAMI_XFER_COUNT];
extern int coll_repo_size[PAMI_XFER_COUNT];
extern int coll_repo_enabled[PAMI_XFER_COUNT];
extern pami_algorithm_t algorithm_ids[PAMI_XFER_COUNT][PAMI_MAX_PROTOCOLS];


pami_result_t pami_advisor_init();

pami_result_t pami_advisor_suggest_algorithm(pami_metadata_t callsite_meta,
                                           pami_metadata_t alg_meta,
                                           pami_xfer_type_t coll_op,
                                           pami_algorithm_t *alg);

  pami_result_t pami_advisor_repo_fill(pami_client_t client,
                                     pami_context_t context,
                                   pami_xfer_type_t xfer_type);

pami_result_t pami_advisor_coll(pami_context_t context,
                              pami_xfer_t *collective,
                              pami_metadata_t meta);

#ifdef __cplusplus
}
#endif

#endif
