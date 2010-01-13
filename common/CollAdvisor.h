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

#include "sys/xmi.h"
#include "util/compact_attributes.h"

#define XMI_MAX_PROTOCOLS                                                    20

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
  xmi_metadata_t metadata;
  xmi_algorithm_t alg_id;
} xmi_alg_repo;

extern xmi_alg_repo *coll_repos[XMI_XFER_COUNT];
extern int coll_repo_size[XMI_XFER_COUNT];
extern int coll_repo_enabled[XMI_XFER_COUNT];
extern xmi_algorithm_t algorithm_ids[XMI_XFER_COUNT][XMI_MAX_PROTOCOLS];


xmi_result_t xmi_advisor_init();

xmi_result_t xmi_advisor_suggest_algorithm(xmi_metadata_t callsite_meta,
                                           xmi_metadata_t alg_meta,
                                           xmi_xfer_type_t coll_op,
                                           xmi_algorithm_t *alg);

xmi_result_t xmi_advisor_repo_fill(xmi_context_t context,
                                   xmi_xfer_type_t xfer_type);

xmi_result_t xmi_advisor_coll(xmi_context_t context,
                              xmi_xfer_t *collective,
                              xmi_metadata_t meta);

#ifdef __cplusplus
}
#endif

#endif
