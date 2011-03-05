/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/api/collectives/barrier_query.c
 * \brief Simple Barrier test on world geometry using "must query" algorithms
 */

#include "../../../../api/pami_util.h"
#include <Arch.h> /* Don't use PAMI_MAX_PROC_PER_NODE in 'real' api test*/

#define NITERLAT   1   
#define NUM_NON_ROOT_DELAYS 2

#include <assert.h>

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context[PAMI_MAX_PROC_PER_NODE]; /* arbitrary max */
  size_t               num_contexts = 1;
  pami_task_t          task_id, non_root[NUM_NON_ROOT_DELAYS];
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               num_algorithm[2];
  pami_algorithm_t    *always_works_algo = NULL;
  pami_metadata_t     *always_works_md = NULL;
  pami_algorithm_t    *must_query_algo = NULL;
  pami_metadata_t     *must_query_md = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  pami_xfer_t          barrier;
  volatile unsigned    poll_flag = 0;

  int                  nalg;

  /* \note Test environment variable" TEST_PROTOCOL={-}substring.       */
  /* substring is used to select, or de-select (with -) test protocols */
  unsigned selector = 1;
  char* selected = getenv("TEST_PROTOCOL");

  if (!selected) selected = "";
  else if (selected[0] == '-')
  {
    selector = 0 ;
    ++selected;
  }

  /* \note Test environment variable" TEST_NUM_CONTEXTS=N, defaults to 1.*/
  char* snum_contexts = getenv("TEST_NUM_CONTEXTS");

  if (snum_contexts) num_contexts = atoi(snum_contexts);

  assert(num_contexts > 0);
  assert(num_contexts <= PAMI_MAX_PROC_PER_NODE);

  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     context,        /* Context            */
                     NULL,           /* Clientname=default */
                     &num_contexts,  /* num_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */

  if (rc == 1)
    return 1;
  if (num_tasks == 1)
  {
    fprintf(stderr,"No barrier on 1 node\n");
    return 0;
  }
  non_root[0] = 1;            /* first non-root rank in the comm  */
  non_root[1] = num_tasks -1;/* last rank in the comm  */

  unsigned iContext = 0;

  for (; iContext < num_contexts; ++iContext)
  {

    if (task_id == 0)
      printf("# Context: %u\n", iContext);

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

    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & poll_flag;

    for (nalg = 0; nalg < num_algorithm[1]; nalg++)
    {
      metadata_result_t result = {0};
      double ti, tf, usec;
      barrier.algorithm = must_query_algo[nalg];

      if (!task_id)
      {
        printf("# Barrier Test -- context = %d, protocol: %s (%d of %zd algorithms)\n",
               iContext, must_query_md[nalg].name, nalg + 1, num_algorithm[1]);
        printf("# -------------------------------------------------------------------\n");
      }

      if (((strstr(must_query_md[nalg].name, selected) == NULL) && selector) ||
          ((strstr(must_query_md[nalg].name, selected) != NULL) && !selector))  continue;

      unsigned mustquery = must_query_md[nalg].check_correct.values.mustquery; /*must query every time */
      assert(!mustquery || must_query_md[nalg].check_fn); /* must have function if mustquery. */

      if (must_query_md[nalg].check_fn)
        result = must_query_md[nalg].check_fn(&barrier);

      if (result.bitmask) continue;

      /* Do two functional runs with different delaying ranks*/
      int j;
      for(j = 0; j < NUM_NON_ROOT_DELAYS; ++j)
      {
        if (!task_id)
        {
          fprintf(stderr, "Test Barrier protocol(%s) Correctness (%d of %zd algorithms)\n",
                  must_query_md[nalg].name, nalg + 1, num_algorithm[1]);
          ti = timer();
          blocking_coll(context[iContext], &barrier, &poll_flag);
          tf = timer();
          usec = tf - ti;
  
          if (usec < 1800000.0 || usec > 2200000.0)
          {  
            rc = 1;  
            fprintf(stderr, "%s FAIL: usec=%f want between %f and %f!\n",must_query_md[nalg].name,
                    usec, 1800000.0, 2200000.0);
          }
          else
            fprintf(stderr, "%s PASS: Barrier correct!\n",must_query_md[nalg].name);
        }
        else
        {
          /* Try to vary where the delay comes from... by picking first and last (non-roots) we
             *might* be getting same node/different node delays.
          */
          if (task_id == non_root[j])
            delayTest(2);
          blocking_coll(context[iContext], &barrier, &poll_flag);
        }
      }

      int niter = NITERLAT;
      blocking_coll(context[iContext], &barrier, &poll_flag);

      ti = timer();
      int i;

      for (i = 0; i < niter; i++)
      {
        if (mustquery) /* must query every time */
        {
          result = must_query_md[nalg].check_fn(&barrier);

          if (result.bitmask) continue;
        }

        blocking_coll(context[iContext], &barrier, &poll_flag);
      }

      tf = timer();
      usec = tf - ti;

      if (!task_id)
      {
        fprintf(stderr, "Test Barrier protocol(%s) Performance: time=%f usec\n",
                must_query_md[nalg].name, usec / (double)niter);
        delayTest(2);
      }

      blocking_coll(context[iContext], &barrier, &poll_flag);
    }

    free(always_works_algo);
    free(always_works_md);
    free(must_query_algo);
    free(must_query_md);

  } /*for(unsigned iContext = 0; iContext < num_contexts; ++iContexts)*/

  rc |= pami_shutdown(&client, context, &num_contexts);

  return rc;
};
