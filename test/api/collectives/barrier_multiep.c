/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/barrier_multiep.c
 * \brief Simple Barrier test on sub-geometries
 */

/* see setup_env() for environment variable overrides               */
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
int fence_arrivals;

typedef struct thread_data_t
{
  pami_context_t context;
  int            tid;
} thread_data_t;

void fence_cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  int * arrived = (int *) clientdata;
  (*arrived)--;
}

int main(int argc, char*argv[])
{
  setup_env();

  pthread_t threads[MAX_THREADS];
  pami_geometry_t      world_geometry;
  size_t               num_algorithm[2];
  pami_algorithm_t    *always_works_algo = NULL;
  pami_metadata_t     *always_works_md = NULL;
  pami_algorithm_t    *must_query_algo = NULL;
  pami_metadata_t     *must_query_md = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  pami_xfer_t          barrier;
  volatile unsigned    poll_flag = 0;
  int                  num_threads    = gNum_contexts;
  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);
  thread_data_t *td = (thread_data_t*)malloc(sizeof(thread_data_t) * gNum_contexts);
  assert(context);
  assert(td);
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

  int i=0;
  printf("Contexts: [ ");
  for(i=0;i<num_threads;i++)
    {
      printf("%p ", context[i]);
      td[i].context = context[i];
      td[i].tid     = i;
    }
  printf("]\n");
  if(task_id == 0) printf("%s:  Querying World Geometry\n", argv[0]);
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
  range[0].lo = 0;
  range[0].hi = num_tasks-1;
  pami_geometry_t parent = (gNum_contexts>1)?PAMI_GEOMETRY_NULL:world_geometry;
  if(task_id == 0) printf("%s:  Creating All Context World Geometry\n", argv[0]);
  rc |= create_all_ctxt_geometry(client,
                                 context,
                                 gNum_contexts,
                                 parent,
                                 &newgeometry,
                                 range,
                                 rangecount,
                                 1);
  if (rc == 1)
    return 1;

  /*  Set up world barrier */
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & poll_flag;
  barrier.algorithm = always_works_algo[0];
  fence_arrivals = num_threads;

  gContext = context[0];
  rc |= blocking_coll(context[0], &barrier, &poll_flag);
  if (rc == 1) return 1;
  int t;

  if(task_id == 0) printf("%s:  Tasks:%zu Threads/task:%d Contexts/task:%zu\n",
                          argv[0],num_tasks,num_threads, gNum_contexts);
  assert(gNum_contexts >= num_threads);
  for(t=0; t<num_threads; t++){
    rc = pthread_create(&threads[t], NULL, barrier_test, (void*)(&td[t]));
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
  gContext = context[0];
  blocking_coll(context[0], &barrier, &poll_flag);

  free(always_works_algo);
  free(always_works_md);
  free(must_query_algo);
  free(must_query_md);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}


static void * barrier_test(void* p)
{
  thread_data_t     *td        = (thread_data_t*)p;
  pami_context_t     myContext = (pami_context_t)td->context;
  size_t                 newbar_num_algo[2];
  pami_algorithm_t      *newbar_algo        = NULL;
  pami_metadata_t       *newbar_md          = NULL;
  pami_algorithm_t      *q_newbar_algo      = NULL;
  pami_metadata_t       *q_newbar_md        = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  pami_xfer_t            newbarrier;
  int                rc,nalg;
  volatile unsigned    poll_flag = 0;

  gContext  = td->context;
  gThreadId = td->tid;

  if (task_id == 0 && td->tid == 0)
    printf("Querying geometry: ctxt=%p tid=%d\n", gContext, gThreadId);

  /*query the geometry in parallel from all threads*/
  rc = query_geometry(client,
                     myContext,
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

      if (((strstr(newbar_md[nalg].name,gSelected) == NULL) && gSelector) ||
          ((strstr(newbar_md[nalg].name,gSelected) != NULL) && !gSelector))  continue;

      ti = timer();
      if (task_id == 0 && td->tid == 0)
        {
          printf("# Barrier Test(correctness) context = %d protocol: %s (%d of %zd algorithms)\n",
                 td->tid, newbar_md[nalg].name, nalg + 1, newbar_num_algo[0]);
          printf("# -------------------------------------------------------------------\n");
          blocking_coll(myContext,&newbarrier, &poll_flag);
        }
      else
        {
          delayTest(2);
          blocking_coll(myContext,&newbarrier, &poll_flag);
        }
      tf = timer();
      usec = tf - ti;

      if (task_id == 0 && td->tid == 0)
        {
          if (( usec > 2200000.0 || usec < 1800000.0) && (num_tasks > 1))
	  {
	    rc = 1;
	    fprintf(stderr, "%s FAIL: usec=%f want between %f and %f!\n", newbar_md[nalg].name,
		    usec, 1800000.0, 2200000.0);
	  }
      }
      int niter = gNiterlat;
      if (task_id == 0 && td->tid == 0)
        {
          printf("# Barrier Test(performance) context = %d protocol: %s (%d of %zd algorithms)\n",
                 td->tid, newbar_md[nalg].name, nalg + 1, newbar_num_algo[0]);
          printf("# -------------------------------------------------------------------\n");
        }
      blocking_coll(myContext, &newbarrier, &poll_flag);

      ti = timer();
      int i;
      for (i = 0; i < niter; i++)
        blocking_coll(myContext, &newbarrier, &poll_flag);
      tf = timer();
      usec = tf - ti;

      if (task_id == 0 && td->tid == 0)
        printf("# Barrier protocol(%s) Performance: time=%f usec\n",
            newbar_md[nalg].name, usec / (double)niter);
    }

  free(newbar_algo);
  free(newbar_md);
  free(q_newbar_algo);
  free(q_newbar_md);
  rc = PAMI_Fence_all (myContext,
		       fence_cb_done,
		       &fence_arrivals);

  while (fence_arrivals != 0)
    rc = PAMI_Context_advance (myContext, 1);

  pthread_exit(NULL);
}
