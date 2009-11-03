/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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

typedef struct
{
  xmi_metadata_t meta;
  xmi_algorithm_t alg_id;
  int alg_rank;
} xmi_alg_repo;
  
xmi_result_t suggest_algorithm(xmi_metadata_t callsite_meta,
                               xmi_metadata_t alg_meta,
                               xmi_xfer_type_t coll_op, 
                               xmi_algorithm_t *alg);
  
xmi_result_t advisor_repo_fill(xmi_xfer_type_t coll_op);

xmi_result_t collective(xmi_context_t context,
                        xmi_xfer_t *collective,
                        xmi_metadata_t meta);


#endif
