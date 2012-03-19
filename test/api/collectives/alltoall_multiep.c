/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/alltoall_multiep.c
 * \brief Simple Alltoall test on sub-geometries
 */

/* see setup_env() for environment variable overrides               */
#define COUNT     (524288)   /* see envvar TEST_BYTES for overrides */

#define MAX_THREADS 128
#define NITERLAT   100
#include "../pami_util.h"
#include <pthread.h>

static void *alltoall_test(void*);
pami_geometry_t      newgeometry;
pami_task_t          task_id;
size_t               num_tasks;
size_t               num_ep;
pami_context_t      *context;
pami_client_t        client;
int                  fence_arrivals;


THREAD_LOCAL char *sbuf = NULL;
THREAD_LOCAL char *rbuf = NULL;

typedef struct thread_data_t
{
  pami_context_t context;
  int            tid;
  int            logical_rank;
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
  pami_metadata_t     *always_works_md   = NULL;
  pami_algorithm_t    *must_query_algo   = NULL;
  pami_metadata_t     *must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer      = PAMI_XFER_BARRIER;
  pami_xfer_t          barrier;
  volatile unsigned    poll_flag         = 0;
  int                  num_threads       = gNum_contexts;

  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);
  thread_data_t *td = (thread_data_t*)malloc(sizeof(thread_data_t) * gNum_contexts);
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

  num_ep = num_tasks *gNum_contexts;

  assert(task_id >= 0);
  assert(task_id < num_tasks);

  int i=0;
  printf("Contexts: [ ");
  for(i=0;i<num_threads;i++)
    {
      td[i].context = context[i];
      td[i].tid     = i;
      td[i].logical_rank   = task_id*num_threads+i;
      printf("(%d|%d)%p ", i, td[i].logical_rank, context[i]);
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

  /*init range */
  range[0].lo = 0;
  range[0].hi = num_tasks-1;

  /*init range geometry; this will allocate an endpoint for each collective*/
  if(task_id == 0) printf("%s:  Creating All Context World Geometry\n", argv[0]);
  pami_geometry_t parent = (gNum_contexts>1)?PAMI_GEOMETRY_NULL:world_geometry;
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

  rc |= blocking_coll(context[0], &barrier, &poll_flag);
  if (rc == 1) return 1;
  int t;

  assert(gNum_contexts >= num_threads);
  if(task_id == 0) printf("%s:  Tasks:%zu Threads/task:%d Contexts/task:%zu\n",
                          argv[0],num_tasks,num_threads, gNum_contexts);
  for(t=0; t<num_threads; t++){
    rc = pthread_create(&threads[t], NULL, alltoall_test, (void*)(&td[t]));
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  void* status;
  for(t=0; t<num_threads; t++) {
    rc = pthread_join(threads[t], &status);
    if (rc) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }
  }

  blocking_coll(context[0], &barrier, &poll_flag);

  free(always_works_algo);
  free(always_works_md);
  free(must_query_algo);
  free(must_query_md);
  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}


static void * alltoall_test(void* p)
{
  thread_data_t       *td        = (thread_data_t*)p;
  pami_context_t       myContext = (pami_context_t)td->context;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag = 0;

  /* Alltoall variables */
  size_t               alltoall_num_algorithm[2];
  pami_algorithm_t    *alltoall_always_works_algo = NULL;
  pami_metadata_t     *alltoall_always_works_md   = NULL;
  pami_algorithm_t    *alltoall_must_query_algo   = NULL;
  pami_metadata_t     *alltoall_must_query_md     = NULL;
  pami_xfer_type_t     alltoall_xfer = PAMI_XFER_ALLTOALL;
  volatile unsigned    alltoall_poll_flag = 0;

  int                  nalg= 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          alltoall;
  int rc = 0;

  /*  Allocate buffer(s) */
  int err = 0;
  err = posix_memalign((void*)&sbuf, 128, (gMax_byte_count*num_ep)+gBuffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + gBuffer_offset;

  err = posix_memalign((void*)&rbuf, 128, (gMax_byte_count*num_ep)+gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;

  fprintf(stderr, "%d:  sbuf=%p rbuf=%p\n", td->tid, sbuf, rbuf);

  /*  Query the world geometry for barrier algorithms */
  rc |= query_geometry(client,
		       myContext,
		       newgeometry,
		       barrier_xfer,
		       barrier_num_algorithm,
		       &bar_always_works_algo,
		       &bar_always_works_md,
		       &bar_must_query_algo,
		       &bar_must_query_md);


  /*  Query the world geometry for alltoall algorithms */
  rc |= query_geometry(client,
		       myContext,
		       newgeometry,
		       alltoall_xfer,
		       alltoall_num_algorithm,
		       &alltoall_always_works_algo,
		       &alltoall_always_works_md,
		       &alltoall_must_query_algo,
		       &alltoall_must_query_md);
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];

  blocking_coll(myContext, &barrier, &bar_poll_flag);


  alltoall.cb_done    = cb_done;
  alltoall.cookie     = (void*) & alltoall_poll_flag;

  pami_endpoint_t my_ep, zero_ep;
  PAMI_Endpoint_create(client,task_id,td->tid,&my_ep);
  PAMI_Endpoint_create(client,0,0,&zero_ep);
  for (nalg = 0; nalg < alltoall_num_algorithm[0]; nalg++)
    {
      size_t i, j;
      gProtocolName = alltoall_always_works_md[nalg].name;
      if (my_ep == zero_ep)
      {
        printf("# Alltoall Bandwidth Test(size:%zu) %p, protocol: %s\n",
               num_ep, cb_done, alltoall_always_works_md[nalg].name);
        printf("# Size(bytes)  iterations    bytes/sec      usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(alltoall_always_works_md[nalg].name, gSelected) == NULL) && gSelector) ||
          ((strstr(alltoall_always_works_md[nalg].name, gSelected) != NULL) && !gSelector))  continue;

      alltoall.algorithm  = alltoall_always_works_algo[nalg];

      for (i = gMin_byte_count; i <= gMax_byte_count; i *= 2)
      {
        size_t  dataSent = i;
        int          niter;

        if (dataSent < CUTOFF)
          niter = gNiterlat;
        else
          niter = NITERBW;

        for (j = 0; j < num_ep; j++)
        {
          alltoall_initialize_bufs(sbuf, rbuf, i, j );
        }

        blocking_coll(myContext, &barrier, &bar_poll_flag);

        /* Warmup */
        alltoall.cmd.xfer_alltoall.sndbuf        = sbuf;
        alltoall.cmd.xfer_alltoall.stype         = PAMI_TYPE_BYTE;
        alltoall.cmd.xfer_alltoall.stypecount    = i;
        alltoall.cmd.xfer_alltoall.rcvbuf        = rbuf;
        alltoall.cmd.xfer_alltoall.rtype         = PAMI_TYPE_BYTE;
        alltoall.cmd.xfer_alltoall.rtypecount    = i;

        blocking_coll(myContext, &alltoall, &alltoall_poll_flag);
        blocking_coll(myContext, &alltoall, &alltoall_poll_flag);

        blocking_coll(myContext, &barrier, &bar_poll_flag);

        ti = timer();

        for (j = 0; j < niter; j++)
        {
          alltoall.cmd.xfer_alltoall.sndbuf        = sbuf;
          alltoall.cmd.xfer_alltoall.stype         = PAMI_TYPE_BYTE;
          alltoall.cmd.xfer_alltoall.stypecount    = i;
          alltoall.cmd.xfer_alltoall.rcvbuf        = rbuf;
          alltoall.cmd.xfer_alltoall.rtype         = PAMI_TYPE_BYTE;
          alltoall.cmd.xfer_alltoall.rtypecount    = i;
          blocking_coll(myContext, &alltoall, &alltoall_poll_flag);
        }

        tf = timer();

        int rc_check;
        rc |= rc_check = alltoall_check_rcvbuf(rbuf, i, num_tasks, task_id);
        if (rc_check)
          {
            fprintf(stderr, "%s FAILED validation\n", gProtocolName);
            sleep(1);
            assert(0);
          }

        blocking_coll(myContext, &barrier, &bar_poll_flag);

        usec = (tf - ti) / (double)niter;

        if (my_ep == zero_ep)
        {

          printf("  %11zu %16d %14.1f %12.2f\n",
                 dataSent,
                 niter,
                 (double)1e6*(double)dataSent / (double)usec,
                 usec);
          fflush(stdout);
        }
      }
    }


  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(alltoall_always_works_algo);
  free(alltoall_always_works_md);
  free(alltoall_must_query_algo);
  free(alltoall_must_query_md);

  sbuf = (char*)sbuf - gBuffer_offset;
  free(sbuf);

  rbuf = (char*)rbuf - gBuffer_offset;
  free(rbuf);

  rc = PAMI_Fence_all (myContext,
		       fence_cb_done,
		       &fence_arrivals);

  while (fence_arrivals != 0)
    rc = PAMI_Context_advance (myContext, 1);

  pthread_exit(NULL);
}
