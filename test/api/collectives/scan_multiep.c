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

#define COUNT     (524288)   /* see envvar TEST_COUNT for overrides */

#define MAX_THREADS 128
#define NITERLAT   100
#include "../pami_util.h"
#include <pthread.h>


void initialize_sndbuf (void *buf, int count, int op, int dt, int task_id)
{

  int i;
  /* if (op == PAMI_SUM && dt == PAMI_UNSIGNED_INT) { */
  if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
    {
      unsigned int *ibuf = (unsigned int *)  buf;
      for (i = 0; i < count; i++)
        {
          ibuf[i] = i;
        }
    }
  else
    {
      size_t sz=get_type_size(dt_array[dt]);
      memset(buf,  task_id,  count * sz);
    }
}

int check_rcvbuf (void *buf, int count, int op, int dt, int num_tasks, int task_id, int exclusive)
{

  int i, err = 0;
  /*  if (op == PAMI_SUM && dt == PAMI_UNSIGNED_INT) { */
  if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
    {
      unsigned int *rbuf = (unsigned int *)  buf;
      unsigned int x;

      if(exclusive == 1)
        {
          /* Receive buffer is not valid on task 0 for exclusive scan */
          if(task_id == 0)
            return 0;
          else
            x = task_id;
        }
      else
        {
          x = task_id +1;
        }
      for (i = 0; i < count; i++)
        {
          if (rbuf[i] != i * x)
            {
              fprintf(stderr,"Check(%d) failed rbuf[%d] %u != %u\n",count,i,rbuf[i],i*x);
              err = -1;
              return err;
            }
        }
    }

  return err;
}


static void *scan_test(void*);
pami_geometry_t      newgeometry;
pami_task_t          task_id;
size_t               num_tasks;
size_t               num_ep;
pami_context_t      *context;
pami_client_t        client;
int                  fence_arrivals;

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
    rc = pthread_create(&threads[t], NULL, scan_test, (void*)(&td[t]));
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


static void * scan_test(void* p)
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

  /* Scan variables */
  size_t               scan_num_algorithm[2];
  pami_algorithm_t    *scan_always_works_algo = NULL;
  pami_metadata_t     *scan_always_works_md   = NULL;
  pami_algorithm_t    *scan_must_query_algo   = NULL;
  pami_metadata_t     *scan_must_query_md     = NULL;
  pami_xfer_type_t     scan_xfer = PAMI_XFER_SCAN;
  volatile unsigned    scan_poll_flag = 0;

  int                  nalg= 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          scan;
  int rc = 0;

  /*  Allocate buffer(s) */
  /*  Allocate buffer(s) */
  int err = 0;
  void* sbuf = NULL;
  err = posix_memalign(&sbuf, 128, gMax_count + gBuffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + gBuffer_offset;
  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, gMax_count + gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;

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


  /*  Query the world geometry for scan algorithms */
  rc |= query_geometry(client,
		       myContext,
		       newgeometry,
		       scan_xfer,
		       scan_num_algorithm,
		       &scan_always_works_algo,
		       &scan_always_works_md,
		       &scan_must_query_algo,
		       &scan_must_query_md);
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];

  blocking_coll(myContext, &barrier, &bar_poll_flag);

  pami_endpoint_t my_ep, zero_ep;
  PAMI_Endpoint_create(client,task_id,td->tid,&my_ep);
  PAMI_Endpoint_create(client,0,0,&zero_ep);

  char * scan_type = "Inclusive";
  int exclusive;
  for (exclusive = 0; exclusive < 2; exclusive++)
    {
      if(exclusive == 1)
        scan_type = "Exclusive";
      for (nalg = 0; nalg < scan_num_algorithm[0]; nalg++)
        {
          if (my_ep == zero_ep)
            {
              printf("# %s Scan Bandwidth Test(size:%zu) -- context = %d, task_id = %d protocol: %s\n",
                     scan_type,num_tasks, td->tid, task_id, scan_always_works_md[nalg].name);
              printf("# Size(bytes)           cycles    bytes/sec    usec\n");
              printf("# -----------      -----------    -----------    ---------\n");
            }

          if (((strstr(scan_always_works_md[nalg].name, gSelected) == NULL) && gSelector) ||
              ((strstr(scan_always_works_md[nalg].name, gSelected) != NULL) && !gSelector))  continue;

          gProtocolName = scan_always_works_md[nalg].name;

          scan.cb_done   = cb_done;
          scan.cookie    = (void*)&scan_poll_flag;
          scan.algorithm = scan_always_works_algo[nalg];
          scan.cmd.xfer_scan.sndbuf    = sbuf;
          scan.cmd.xfer_scan.rcvbuf    = rbuf;
          scan.cmd.xfer_scan.rtype     = PAMI_TYPE_BYTE;
          scan.cmd.xfer_scan.rtypecount= 0;
          scan.cmd.xfer_scan.exclusive = exclusive;

          int op, dt;

          for (dt=0; dt<dt_count; dt++)
            {
              for (op=0; op<op_count; op++)
                {
                  if (gValidTable[op][dt])
                    {
                      if (my_ep == zero_ep)
                        printf("Running Scan: %s, %s\n",dt_array_str[dt], op_array_str[op]);
                      int i;
                      for (i = gMin_count; i <= gMax_count/get_type_size(dt_array[dt]); i *= 2)
                        {
                          size_t sz=get_type_size(dt_array[dt]);
                          size_t  dataSent = i * sz;
                          int niter;

                          if (dataSent < CUTOFF)
                            niter = gNiterlat;
                          else
                            niter = NITERBW;

                          scan.cmd.xfer_scan.stypecount=i;
                          scan.cmd.xfer_scan.rtypecount=dataSent;
                          scan.cmd.xfer_scan.stype     =dt_array[dt];
                          scan.cmd.xfer_scan.op        =op_array[op];

                          initialize_sndbuf (sbuf, i, op, dt, td->logical_rank);
                          memset(rbuf, 0xFF, dataSent);

                          blocking_coll(myContext, &barrier, &bar_poll_flag);
                          ti = timer();
                          int j;
                          for (j=0; j<1; j++)
                            {
                              blocking_coll(myContext, &scan, &scan_poll_flag);
                            }
                          tf = timer();
                          blocking_coll(myContext, &barrier, &bar_poll_flag);

                          int rc_check;
                          rc |= rc_check = check_rcvbuf (rbuf, i, op, dt, num_ep, td->logical_rank, exclusive);

                          if (rc_check) fprintf(stderr, "%s scan %s FAILED validation\n", scan_type, gProtocolName);

                          usec = (tf - ti)/(double)niter;
                          if (my_ep == zero_ep)
                            {
                              printf("  %11lld %16d %14.1f %12.2f\n",
                                     (long long)dataSent,
                                     niter,
                                     (double)1e6*(double)dataSent/(double)usec,
                                     usec);
                              fflush(stdout);
                            }
                        }
                    }
                }
            }
        }
    }




  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(scan_always_works_algo);
  free(scan_always_works_md);
  free(scan_must_query_algo);
  free(scan_must_query_md);

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
