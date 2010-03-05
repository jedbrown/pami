/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file core/xmi_advisor.c
 * \brief ???
 */
#define _ENABLE_COLLADVISOR_
#ifdef _ENABLE_COLLADVISOR_
#include "common/CollAdvisor.h"

#define REPO_TO_BE_INITIALIZED                                                 1
#define REPO_INITIALIZED                                                       2

xmi_alg_repo *coll_repos[XMI_XFER_COUNT] = {NULL};
int coll_repo_size[XMI_XFER_COUNT] = {0};
int coll_repo_enabled[XMI_XFER_COUNT] = {0};
xmi_algorithm_t algorithm_ids[XMI_XFER_COUNT][XMI_MAX_PROTOCOLS];

xmi_result_t xmi_advisor_init()
{
#if 0
  coll_repo_enabled[XMI_XFER_BROADCAST] = 1;
  algorithm_ids[XMI_XFER_BROADCAST][0] = 1;
  algorithm_ids[XMI_XFER_BROADCAST][1] = 3;
  algorithm_ids[XMI_XFER_BROADCAST][2] = 0;
  algorithm_ids[XMI_XFER_BROADCAST][3] = 2;
#endif
  return XMI_SUCCESS;
}

xmi_result_t xmi_advisor_suggest_algorithm(xmi_metadata_t callsite_meta,
                                           xmi_metadata_t alg_meta,
                                           xmi_xfer_type_t xfer_type,
                                           xmi_algorithm_t *alg)
{
  /*
  int i, match = 0, repo_size = coll_repo_size[xfer_type];
  for (i = 0; i < alg_list[0]; i++)
  {

  }
  */
  return XMI_UNIMPL;
}

// we have to manually sit the hints in the protocols.
// then here we also have to manually add each procotol to the list and
// order them
xmi_result_t xmi_advisor_repo_fill(xmi_client_t client,
                                   xmi_context_t context,
                                   xmi_xfer_type_t xfer_type)
{
  int alg_list[2] = {0};
  int algorithm_type = 0; // always works list
  xmi_result_t result;
  xmi_geometry_t world_geometry;

  if (xfer_type >= XMI_XFER_COUNT)
    return XMI_ERROR;


  result = XMI_Geometry_world (context, &world_geometry);

  if (coll_repo_enabled[xfer_type] == REPO_TO_BE_INITIALIZED)
  {
    xmi_algorithm_t *algs;
    xmi_metadata_t *metas;

    coll_repo_enabled[xfer_type] = REPO_INITIALIZED;

    XMI_Geometry_algorithms_num(context,
                                world_geometry,
                                xfer_type,
                                &alg_list[algorithm_type]);
    if (alg_list[algorithm_type])
    {
      algs = (xmi_algorithm_t*)
        malloc(sizeof(xmi_algorithm_t) * alg_list[algorithm_type]);
      metas = (xmi_metadata_t*)
        malloc(sizeof(xmi_metadata_t) * alg_list[algorithm_type]);

      XMI_Geometry_algorithms_info(client,
                                   world_geometry,
                                   algorithm_type,
                                   algs,
                                   metas,
                                   alg_list[algorithm_type],
                                   NULL,
                                   NULL,
                                   0);

    }
#if 0
    coll_repos[xfer_type] = (xmi_alg_repo *)
                            malloc(sizeof(xmi_alg_repo) * alg_list[0]);
    for (int i = 0, j = 0; i < alg_list[0]; i++)
    {
      result = XMI_Geometry_algorithm_info(context,
                                           world_geometry,
                                           xfer_type,
                                           algorithm_ids[xfer_type][i],
                                           algorithm_type,
                                           &coll_repos[xfer_type][i].metadata,
                                           NULL,
                                           NULL,
                                           0);

      if (result == XMI_SUCCESS)
      {
        coll_repos[xfer_type][j++].alg_id = algorithm_ids[xfer_type][i];
        coll_repo_size[xfer_type]++;
      }
    }
#endif
  }
  return XMI_SUCCESS;
}


xmi_result_t xmi_advisor_coll(xmi_context_t context,
                              xmi_xfer_t *collective,
                              xmi_metadata_t meta)
{
#if 0
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
#endif
    return XMI_UNIMPL;

}
#endif
