/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/coll_util.h
 * \brief ???
 */

#ifndef __test_api_coll_util_h__
#define __test_api_coll_util_h__

unsigned gVerbose = 1; /* Global verbose flag, some tests set with TEST_VERBOSE=n */

/* Docs09:  Done/Decrement call */
void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  if(gVerbose && !ctxt)
    fprintf(stderr, "Error. Null context received on cb_done.\n");
  int * active = (int *) clientdata;
  (*active)--;
}
/* Docs10:  Done/Decrement call */

/* Docs07:  Blocking Collective Call */
int blocking_coll (pami_context_t      context,
                   pami_xfer_t        *coll,
                   volatile unsigned  *active)
{
  pami_result_t result;
  (*active)++;
  result = PAMI_Collective(context, coll);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue  collective. result = %d\n", result);
      return 1;
    }
  while (*active)
    result = PAMI_Context_advance (context, 1);
  return 0;
}
/* Docs08:  Blocking Collective Call */

#include<assert.h>
int query_geometry_world(pami_client_t       client,
                         pami_context_t      context,
                         pami_geometry_t    *world_geometry,
                         pami_xfer_type_t    xfer_type,
                         size_t             *num_algorithm,
                         pami_algorithm_t  **always_works_alg,
                         pami_metadata_t   **always_works_md,
                         pami_algorithm_t  **must_query_alg,
                         pami_metadata_t   **must_query_md)
{

  pami_result_t     result           = PAMI_SUCCESS;
  /* Docs01:  Get the World Geometry */
  result = PAMI_Geometry_world (client,world_geometry);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get world geometry: result = %d\n", result);
      return 1;
    }
  /* Docs02:  Get the World Geometry */

  /* Docs03:  Query the number of algorithms */
  result = PAMI_Geometry_algorithms_num(context,
                                        *world_geometry,
                                        xfer_type,
                                        num_algorithm);

  if (result != PAMI_SUCCESS || num_algorithm[0]==0)
    {
      fprintf (stderr,
               "Error. Unable to query algorithm, or no algorithms available result = %d\n",
               result);
      return 1;
    }
  /* Docs04:  Query the number of algorithms */

  *always_works_alg = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[0]);
  *always_works_md  = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[0]);
  *must_query_alg   = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[1]);
  *must_query_md    = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[1]);

  /* Docs05:  Query the algorithm lists */
  result = PAMI_Geometry_algorithms_query(context,
                                          *world_geometry,
                                          xfer_type,
                                          *always_works_alg,
                                          *always_works_md,
                                          num_algorithm[0],
                                          *must_query_alg,
                                          *must_query_md,
                                          num_algorithm[1]);
  /* Docs06:  Query the algorithm lists */
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get query algorithm. result = %d\n", result);
      return 1;
    }
  return 0;
}

int query_geometry(pami_client_t       client,
                   pami_context_t      context,
                   pami_geometry_t     geometry,
                   pami_xfer_type_t    xfer_type,
                   size_t             *num_algorithm,
                   pami_algorithm_t  **always_works_alg,
                   pami_metadata_t   **always_works_md,
                   pami_algorithm_t  **must_query_alg,
                   pami_metadata_t   **must_query_md)
{
  pami_result_t     result           = PAMI_SUCCESS;

  result = PAMI_Geometry_algorithms_num(context,
                                        geometry,
                                        xfer_type,
                                        num_algorithm);

  if (result != PAMI_SUCCESS || num_algorithm[0]==0)
    {
      fprintf (stderr,
               "Error. query, Unable to query algorithm, or no algorithms available result = %d\n",
               result);
      return 1;
    }

  *always_works_alg = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[0]);
  *always_works_md  = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[0]);
  *must_query_alg   = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[1]);
  *must_query_md    = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[1]);
  result = PAMI_Geometry_algorithms_query(context,
                                          geometry,
                                          xfer_type,
                                          *always_works_alg,
                                          *always_works_md,
                                          num_algorithm[0],
                                          *must_query_alg,
                                          *must_query_md,
                                          num_algorithm[1]);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get query algorithm. result = %d\n", result);
      return 1;
    }
  return 0;
}




int create_and_query_geometry(pami_client_t           client,
                              pami_context_t          context,  /* context for create callback */
                              pami_context_t          contextq, /* context for query */
                              pami_geometry_t         parent_geometry,
                              pami_geometry_t        *new_geometry,
                              pami_geometry_range_t  *range,
                              unsigned                numranges,
                              unsigned                id,
                              pami_xfer_type_t        xfer_type,
                              size_t                 *num_algorithm,
                              pami_algorithm_t      **always_works_alg,
                              pami_metadata_t       **always_works_md,
                              pami_algorithm_t      **must_query_alg,
                              pami_metadata_t       **must_query_md)
{
  pami_result_t result;
  int           geom_init=1;

  pami_configuration_t config;
  config.name = PAMI_GEOMETRY_OPTIMIZE;
  
  result = PAMI_Geometry_create_taskrange (client,
                                           &config, /*NULL*/
                                           1, /*0, */
                                           new_geometry,
                                           parent_geometry,
                                           id,
                                           range,
                                           numranges,
                                           context,
                                           cb_done,
                                           &geom_init);
    if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to create a new geometry. result = %d\n", result);
      return 1;
    }
    while (geom_init)
      result = PAMI_Context_advance (context, 1);

    return query_geometry(client,
                          contextq,
                          *new_geometry,
                          xfer_type,
                          num_algorithm,
                          always_works_alg,
                          always_works_md,
                          must_query_alg,
                          must_query_md);
}

#endif /* __test_api_coll_util_h__*/
