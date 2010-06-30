/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file core/pami_advisor.c
 * \brief ???
 */
#define _ENABLE_COLLADVISOR_
#ifdef _ENABLE_COLLADVISOR_
#include "common/CollAdvisor.h"

#define REPO_TO_BE_INITIALIZED                                                 1
#define REPO_INITIALIZED                                                       2

pami_alg_repo *coll_repos[PAMI_XFER_COUNT] = {NULL};
int coll_repo_size[PAMI_XFER_COUNT] = {0};
int coll_repo_enabled[PAMI_XFER_COUNT] = {0};
pami_algorithm_t algorithm_ids[PAMI_XFER_COUNT][PAMI_MAX_PROTOCOLS];

pami_result_t pami_advisor_init()
{
#if 0
  coll_repo_enabled[PAMI_XFER_BROADCAST] = 1;
  algorithm_ids[PAMI_XFER_BROADCAST][0] = 1;
  algorithm_ids[PAMI_XFER_BROADCAST][1] = 3;
  algorithm_ids[PAMI_XFER_BROADCAST][2] = 0;
  algorithm_ids[PAMI_XFER_BROADCAST][3] = 2;
#endif
  return PAMI_SUCCESS;
}

pami_result_t pami_advisor_suggest_algorithm(pami_metadata_t callsite_meta,
                                           pami_metadata_t alg_meta,
                                           pami_xfer_type_t xfer_type,
                                           pami_algorithm_t *alg)
{
  /*
  int i, match = 0, repo_size = coll_repo_size[xfer_type];
  for (i = 0; i < alg_list[0]; i++)
  {

  }
  */
  return PAMI_UNIMPL;
}

// we have to manually sit the hints in the protocols.
// then here we also have to manually add each procotol to the list and
// order them
pami_result_t pami_advisor_repo_fill(pami_client_t client,
                                   pami_context_t context,
                                   pami_xfer_type_t xfer_type)
{
  size_t alg_list[2] = {0};
  int algorithm_type = 0; // always works list
  pami_result_t result;
  pami_geometry_t world_geometry;

  if (xfer_type >= PAMI_XFER_COUNT)
    return PAMI_ERROR;


  result = PAMI_Geometry_world (context, &world_geometry);

  if (coll_repo_enabled[xfer_type] == REPO_TO_BE_INITIALIZED)
  {
    pami_algorithm_t *algs;
    pami_metadata_t *metas;

    coll_repo_enabled[xfer_type] = REPO_INITIALIZED;

    PAMI_Geometry_algorithms_num(context,
                                world_geometry,
                                xfer_type,
                                &alg_list[algorithm_type]);
    if (alg_list[algorithm_type])
    {
      algs = (pami_algorithm_t*)
        malloc(sizeof(pami_algorithm_t) * alg_list[algorithm_type]);
      metas = (pami_metadata_t*)
        malloc(sizeof(pami_metadata_t) * alg_list[algorithm_type]);
#if 0

      PAMI_Geometry_algorithms_query(client,
                                   world_geometry,
                                   algorithm_type,
                                   algs,
                                   metas,
                                   alg_list[algorithm_type],
                                   NULL,
                                   NULL,
                                   0);
#endif

    }
#if 0
    coll_repos[xfer_type] = (pami_alg_repo *)
                            malloc(sizeof(pami_alg_repo) * alg_list[0]);
    for (int i = 0, j = 0; i < alg_list[0]; i++)
    {
      result = PAMI_Geometry_algorithm_info(context,
                                           world_geometry,
                                           xfer_type,
                                           algorithm_ids[xfer_type][i],
                                           algorithm_type,
                                           &coll_repos[xfer_type][i].metadata,
                                           NULL,
                                           NULL,
                                           0);

      if (result == PAMI_SUCCESS)
      {
        coll_repos[xfer_type][j++].alg_id = algorithm_ids[xfer_type][i];
        coll_repo_size[xfer_type]++;
      }
    }
#endif
  }
  return PAMI_SUCCESS;
}


pami_result_t pami_advisor_coll(pami_context_t context,
                              pami_xfer_t *collective,
                              pami_metadata_t meta)
{
#if 0
  if (collective->xfer_type >= PAMI_XFER_COUNT)
    return PAMI_ERROR;

  switch (collective->xfer_type)
  {
    case PAMI_XFER_BROADCAST:
      op = BROADCAST;
      break;
    case PAMI_XFER_ALLREDUCE:
      op = ALLREDUCE;
      break;
    case PAMI_XFER_REDUCE:
      op = BROADCAST;
      break;
  case PAMI_XFER_ALLGATHER:
    op = BROADCAST;
    break;
  case PAMI_XFER_ALLGATHERV:
    op = BROADCAST;
    break;
  case PAMI_XFER_ALLGATHERV_INT:
    op = BROADCAST;
    break;
  case PAMI_XFER_SCATTER:
    op = BROADCAST;
    break;
  case PAMI_XFER_SCATTERV:
    op = BROADCAST;
    break;
  case PAMI_XFER_SCATTERV_INT:
    op = BROADCAST;
    break;
  case PAMI_XFER_BARRIER:
    op = BROADCAST;
    break;
  case PAMI_XFER_ALLTOALL:
    op = BROADCAST;
    break;
  case PAMI_XFER_ALLTOALLV:
    op = BROADCAST;
    break;
  case PAMI_XFER_ALLTOALLV_INT:
    op = BROADCAST;
    break;
  case PAMI_XFER_SCAN:
    op = BROADCAST;
    break;
  case PAMI_XFER_AMBROADCAST:
    op = BROADCAST;

    break;
  case PAMI_XFER_AMSCATTER:
    op = BROADCAST;
    break;
  case PAMI_XFER_AMGATHER:
    op = BROADCAST;
    break;
  case PAMI_XFER_AMREDUCE:
    op = BROADCAST;
    break;
  default:
    return PAMI_UNIMPL;
  }
#endif
    return PAMI_UNIMPL;

}
#endif
