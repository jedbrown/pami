/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/barrier_multiep.c
 * \brief Simple Barrier test on sub-geometries
 */

#define MAX_THREADS 128
#define NITERLAT   100
#include "../pami_util.h"
#include <pthread.h>

static void *barrier_test(void*);

pami_geometry_t      newgeometry;
pami_task_t          task_id;
size_t               num_tasks;
pami_context_t      *context;
pami_client_t        client;
pami_task_t          non_root[2];

int main(int argc, char*argv[])
{
  pthread_t threads[MAX_THREADS];
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

  int                  iContext=0; /* main thread creates geometry*/
  int                  num_threads    = 1;

  /* Process environment variables and setup globals */
  setup_env();

  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);

  /*  Initialize PAMI; This will also create gNum_contexts contexts*/
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

  assert(task_id >= 0);
  assert(task_id < num_tasks);

  /*  Query the world geometry for barrier algorithms */
  rc |= query_geometry_world(client,
                             context[0],
                             &world_geometry,
                             barrier_xfer,
                             num_algorithm,
                             &always_works_algo,
                             &always_works_md,
                             &must_query_algo,
                             &must_query_md);

  if (rc == 1)
    return 1;

  /*  Create the range geometry */
  pami_geometry_range_t *range;
  int                    rangecount;

  rangecount = 1;
  range     = (pami_geometry_range_t *)malloc(((rangecount)) * sizeof(pami_geometry_range_t));

  /*init range */
  range[0].lo = 0;
  range[0].hi = num_tasks-1;

  /*init range geometry; this will allocate an endpoint for each collective*/
  pami_geometry_t parent = (gNum_contexts>1)?PAMI_GEOMETRY_NULL:world_geometry;
  rc |= create_all_ctxt_geometry(client,
                                 context,
                                 gNum_contexts,
                                 parent,
                                 &newgeometry,
                                 range,
                                 rangecount,
                                 iContext+1);

  if (rc == 1)
    return 1;

  /* variables to control the delay for barrier correctness */
  non_root[0] = 1;                          /* first non-root ordinal in the comm  */
  non_root[1] = num_tasks*gNum_contexts - 1;/* last ordinal in the comm  */

  /*  Set up world barrier */
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & poll_flag;
  barrier.algorithm = always_works_algo[0];

  rc |= blocking_coll(context[iContext], &barrier, &poll_flag);
  if (rc == 1) return 1;
  int t;
  /* here we start a number of threads; each of them will query geometry and run an algo*/

  assert(gNum_contexts >= num_threads);
  for(t=0; t<num_threads; t++){
    rc = pthread_create(&threads[t], NULL, barrier_test, (void*)&context[t]);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  /* Free attribute and wait for the other threads */
  void* status;
  for(t=0; t<num_threads; t++) {
    rc = pthread_join(threads[t], &status);
    if (rc) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }
  }

  blocking_coll(context[iContext], &barrier, &poll_flag);

  free(always_works_algo);
  free(always_works_md);
  free(must_query_algo);
  free(must_query_md);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}


static void * barrier_test(void* p) {
  pami_context_t        *myContext = (pami_context_t*)p;
  int                    iContext = -1;
  size_t                 newbar_num_algo[2];
  pami_algorithm_t      *newbar_algo        = NULL;
  pami_metadata_t       *newbar_md          = NULL;
  pami_algorithm_t      *q_newbar_algo      = NULL;
  pami_metadata_t       *q_newbar_md        = NULL;

  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  pami_xfer_t            newbarrier;
  int rc, k;
  volatile unsigned    poll_flag = 0;
  int nalg;

  /* Find my context offset */
  for(k=0; k<gNum_contexts; k++)
    if(context[k] == *myContext) iContext = k;
  assert(iContext != -1);

  int ep_task_id = task_id * gNum_contexts + iContext;

  /*query the geometry in parallel from all threads*/
  rc = query_geometry(client,
                     *myContext,
                     newgeometry,
                     barrier_xfer,
                     newbar_num_algo,
                     &newbar_algo,
                     &newbar_md,
                     &q_newbar_algo,
                     &q_newbar_md);

  /*  Set up sub geometry barrier */
  newbarrier.cb_done   = cb_done;
  newbarrier.cookie    = (void*) & poll_flag;

  for (nalg = 0; nalg < newbar_num_algo[0]; nalg++)
    {
      double ti, tf, usec;
      newbarrier.algorithm = newbar_algo[nalg];

      if (ep_task_id == 0)
          {
            printf("# Test set(%u):  Barrier Test -- context = %d, task_zero = %d, protocol: %s (%d of %zd algorithms)\n",
                   k, iContext, task_id, newbar_md[nalg].name, nalg + 1, newbar_num_algo[0]);
            printf("# -------------------------------------------------------------------\n");
          }

      if (((strstr(newbar_md[nalg].name,gSelected) == NULL) && gSelector) ||
          ((strstr(newbar_md[nalg].name,gSelected) != NULL) && !gSelector))  continue;


      /* Do two functional runs with different delaying ranks*/
      int j;

      for (j = 0; j < 2; ++j)
      {
        if (ep_task_id==0)
        {
          fprintf(stderr, "Test Barrier protocol(%s) Correctness (%d of %zd algorithms)\n",
                  newbar_md[nalg].name, nalg + 1, newbar_num_algo[0]);
          ti = timer();
          blocking_coll(*myContext, &newbarrier, &poll_flag);
          tf = timer();
          usec = tf - ti;

          if ((usec < 1800000.0 || usec > 2200000.0) && (num_tasks > 1))
          {
            rc = 1;
            fprintf(stderr, "%s FAIL: usec=%f want between %f and %f!\n", newbar_md[nalg].name,
                    usec, 1800000.0, 2200000.0);
          }
          else
            fprintf(stderr, "%s PASS: Barrier correct!\n", newbar_md[nalg].name);
        }
        else
        {
          /* Try to vary where the delay comes from... by picking first and last (non-roots) we
             *might* be getting same node/different node delays.
          */
          if (ep_task_id == non_root[j])
            delayTest(2);

          blocking_coll(*myContext, &newbarrier, &poll_flag);
        }
      }

      int niter = gNiterlat;
      blocking_coll(*myContext, &newbarrier, &poll_flag);

      ti = timer();
      int i;

      for (i = 0; i < niter; i++)
        blocking_coll(*myContext, &newbarrier, &poll_flag);

      tf = timer();
      usec = tf - ti;

      if (!ep_task_id)
      {
        fprintf(stderr, "Test Barrier protocol(%s) Performance: time=%f usec\n",
                newbar_md[nalg].name, usec / (double)niter);
        delayTest(2);
      }

      blocking_coll(*myContext, &newbarrier, &poll_flag);
    }/*for all algos*/

  free(newbar_algo);
  free(newbar_md);
  free(q_newbar_algo);
  free(q_newbar_md);
  pthread_exit(NULL);
}
