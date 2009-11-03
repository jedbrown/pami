/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file core/Advisor.c
 * \brief ???
 */

#ifdef _ENABLE_COLLADVISOR_
#include "common/CollAdvisor.h"

xmi_alg_repo *coll_repos[XMI_XFER_COUNT];
int coll_repo_size[XMI_XFER_COUNT];
int coll_repo_enabled[XMI_XFER_COUNT];


xmi_result_t suggest_algorithm(xmi_metadata_t callsite_meta,
                               xmi_metadata_t alg_meta,
                               xmi_xfer_type_t coll_op, 
                               xmi_algorithm_t *alg);
{
  return XMI_UNIMPL;
}

// we have to manually sit the hints in the protocols.
// then here we also have to manually add each procotol to the list and
// order them
xmi_result_t advisor_repo_fill(xmi_context_t context,
                               xmi_xfer_type_t xfer_type);
{
  int alg_list[2] = {0};
  xmi_result_t result;
  xmi_geometry_t world_geometry;
  
  if (xfer_type >= XMI_XFER_COUNT)
    return XMI_ERROR;


  result = XMI_Geometry_world (context, &world_geometry);
  
  if (coll_repo_enabled[xfer_type])
  {
    XMI_Geometry_algorithms_num(context,
                                world_geometry,
                                xfer_type, 
                                &alg_list);
  }
}

xmi_result_t coll_advisor(xmi_context_t context,
                          xmi_xfer_t *collective,
                          xmi_metadata_t meta);
{
  if (collective->xfer_type >= XMI_XFER_COUNT)
    return XMI_ERROR;

  switch (collective->xfer_type)
  {
    case XMI_XFER_BROADCAST:
      op = BROADCAST;
      break;
    case XMI_XFER_ALLREDUCE:
      op = ALLREDUCE;
      break;
    case XMI_XFER_REDUCE:
      op = BROADCAST;
      break;
  case XMI_XFER_ALLGATHER:
    op = BROADCAST;
    break;
  case XMI_XFER_ALLGATHERV:
    op = BROADCAST;
    break;
  case XMI_XFER_ALLGATHERV_INT:
    op = BROADCAST;
    break;
  case XMI_XFER_SCATTER:
    op = BROADCAST;
    break;
  case XMI_XFER_SCATTERV:
    op = BROADCAST;
    break;
  case XMI_XFER_SCATTERV_INT:
    op = BROADCAST;
    break;
  case XMI_XFER_BARRIER:
    op = BROADCAST;
    break;
  case XMI_XFER_ALLTOALL:
    op = BROADCAST;
    break;
  case XMI_XFER_ALLTOALLV:
    op = BROADCAST;
    break;
  case XMI_XFER_ALLTOALLV_INT:
    op = BROADCAST;
    break;
  case XMI_XFER_SCAN:
    op = BROADCAST;
    break;
  case XMI_XFER_AMBROADCAST:
    op = BROADCAST;

    break;
  case XMI_XFER_AMSCATTER:
    op = BROADCAST;
    break;
  case XMI_XFER_AMGATHER:
    op = BROADCAST;
    break;
  case XMI_XFER_AMREDUCE:
    op = BROADCAST;
    break;
  default:
    return XMI_UNIMPL;
  }
}
#endif
