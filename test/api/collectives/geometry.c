/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/geometry.c
 * \brief Test for large geometry create counts
 */



#define NITER   10000


#include "../pami_util.h"

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_task_t          task_id, local_task_id, task_real_zero=0, task_zero=0;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               num_algorithm[2];
  pami_algorithm_t    *always_works_algo = NULL;
  pami_metadata_t     *always_works_md = NULL;
  pami_algorithm_t    *must_query_algo = NULL;
  pami_metadata_t     *must_query_md = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;

  /* Process environment variables and setup globals */
  setup_env();

  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);


  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     context,        /* Context            */
                     NULL,           /* Clientname=default */
                     &gNum_contexts, /* gNum_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */

  if (rc == 1)
    return 1;

  if (num_tasks == 1)
  {
    fprintf(stderr, "No barrier subcomms on 1 node\n");
    return 0;
  }

  assert(task_id >= 0);
  assert(task_id < num_tasks);

  unsigned iContext = 0;

  for (; iContext < gNum_contexts; ++iContext)
  {

    if (task_id == 0)
      printf("# Context: %u\n", iContext);

    /*  Query the world geometry for barrier algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               barrier_xfer,
                               num_algorithm,
                               &always_works_algo,
                               &always_works_md,
                               &must_query_algo,
                               &must_query_md);

    if (rc == 1)
      return 1;

    /*  Create the subgeometry */
    pami_geometry_range_t *range;
    int                    rangecount;
    pami_geometry_t        newgeometry;
    size_t                 newbar_num_algo[2];
    pami_algorithm_t      *newbar_algo        = NULL;
    pami_metadata_t       *newbar_md          = NULL;
    pami_algorithm_t      *q_newbar_algo      = NULL;
    pami_metadata_t       *q_newbar_md        = NULL;

    size_t                 set[2];
    int                    id, non_root[2];

    range     = (pami_geometry_range_t *)malloc(((num_tasks + 1) / 2) * sizeof(pami_geometry_range_t));

    get_split_method(&num_tasks, task_id, &rangecount, range, &local_task_id, set, &id, &task_zero,non_root);

    int k;
    double ti=timer(), timeElapsed=0, timeIteration=0;
    for(k=0; k<NITER; k++)
    {
      if (task_id == k%num_tasks)
      {
        unsigned ii = 0;
        for (; ii < gNum_contexts; ++ii)
          PAMI_Context_advance (context[ii], 1000);
      }

      rc |= create_and_query_geometry(client,
                                      context[0],
                                      context[iContext],
                                      gParentless ? PAMI_GEOMETRY_NULL : world_geometry,
                                      &newgeometry,
                                      range,
                                      rangecount,
                                      id + iContext, /* Unique id for each context */
                                      barrier_xfer,
                                      newbar_num_algo,
                                      &newbar_algo,
                                      &newbar_md,
                                      &q_newbar_algo,
                                      &q_newbar_md);
      if (rc == 1)
        return 1;

      rc |= PAMI_Geometry_destroy(client, &newgeometry);
      if (rc == 1)
        return 1;
      if(k!=0 && (k%(NITER/10))==0 && task_id == task_real_zero)
      {
        double tf     = timer();
        timeIteration = timeElapsed;
        timeElapsed   = tf - ti;
        timeIteration = timeElapsed - timeIteration;
        fprintf(stdout, "Done with iteration %d of %d timePerIteration=%f usec\n",
                k, NITER, timeIteration/(double)(NITER/10.0));
      }
    }


  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  rc |= pami_shutdown(&client, context, &gNum_contexts);

  return rc;
}
