/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/api/collectives/scatter.c
 * \brief Simple Scatter test on world geometry (only scatters bytes)
 */

#include "../../../../api/pami_util.h"


/*define this if you want to validate the data */
#define CHECK_DATA

#define ROOT     (0)         /* see envvar TEST_ROOT for overrides */
size_t root = ROOT;


#define COUNT     (524288)   /* see envvar TEST_COUNT for overrides */
unsigned max_count = COUNT;

#define OFFSET     0            /* see envvar TEST_OFFSET for overrides */
unsigned buffer_offset = OFFSET;

#define NITERLAT   1            /* see envvar TEST_ITER for overrides */
unsigned niterlat  = NITERLAT;

#define NITERBW    MIN(10, niterlat/100+1)

#define CUTOFF     65536

char* protocolName;

void initialize_sndbuf (void *sbuf, int bytes, size_t ntasks)
{
  size_t i;
  unsigned char *cbuf = (unsigned char *)  sbuf;

  for (i=0; i<ntasks; i++)
  {
    unsigned char c = 0xFF & i;
    memset(cbuf+(i*bytes),c,bytes);
  }
}

int check_rcvbuf (void *rbuf, int bytes, pami_task_t task)
{
  int i;
  unsigned char *cbuf = (unsigned char *)  rbuf;

  unsigned char c = 0xFF & task;
  for (i=0; i<bytes; i++)
    if (cbuf[i] != c)
    {
      fprintf(stderr, "%s:Check(%d) failed <%p>rbuf[%d]=%.2u != %.2u \n", protocolName, bytes, cbuf,i,cbuf[i], c);
      return 1;
    }
  return 0;
}

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t      *context;
  size_t               num_contexts = 1;
  pami_task_t          task_id;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag = 0;

  /* Scatter variables */
  size_t               scatter_num_algorithm[2];
  pami_algorithm_t    *scatter_always_works_algo = NULL;
  pami_metadata_t     *scatter_always_works_md = NULL;
  pami_algorithm_t    *scatter_must_query_algo = NULL;
  pami_metadata_t     *scatter_must_query_md = NULL;
  pami_xfer_type_t     scatter_xfer = PAMI_XFER_SCATTER;
  volatile unsigned    scatter_poll_flag = 0;

  int                  nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          scatter;

  /* \note Test environment variable" TEST_VERBOSE=N     */
  char* sVerbose = getenv("TEST_VERBOSE");

  if (sVerbose) gVerbose=atoi(sVerbose); /* set the global defined in coll_util.h */

  /* \note Test environment variable" TEST_PROTOCOL={-}substring.       */
  /* substring is used to select, or de-select (with -) test protocols */
  unsigned selector = 1;
  char* selected = getenv("TEST_PROTOCOL");

  if (!selected) selected = "";
  else if (selected[0]=='-')
  {
    selector = 0 ;
    ++selected;
  }

  /* \note Test environment variable" TEST_COUNT=N max count     */
  char* sCount = getenv("TEST_COUNT");

  /* Override COUNT */
  if (sCount) max_count = atoi(sCount);

  /* \note Test environment variable" TEST_OFFSET=N buffer offset/alignment*/
  char* sOffset = getenv("TEST_OFFSET");

  /* Override OFFSET */
  if (sOffset) buffer_offset = atoi(sOffset);

  /* \note Test environment variable" TEST_ITER=N iterations      */
  char* sIter = getenv("TEST_ITER");

  /* Override NITERLAT */
  if (sIter) niterlat = atoi(sIter);

  /* \note Test environment variable" TEST_NUM_CONTEXTS=N, defaults to 1.*/
  char* snum_contexts = getenv("TEST_NUM_CONTEXTS");

  if (snum_contexts) num_contexts = atoi(snum_contexts);

  assert(num_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t)*num_contexts);

  /* \note Test environment variable" TEST_ROOT=N, defaults to 0.*/
  char* sRoot = getenv("TEST_ROOT");

  /* Override ROOT */
  if (sRoot) root = atoi(sRoot);


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

  /*  Allocate buffer(s) */
  int err = 0;
  void* buf = NULL;
  if (task_id == root)
  {
    err = posix_memalign(&buf, 128, (max_count*num_tasks)+buffer_offset);
    assert(err == 0);
    buf = (char*)buf + buffer_offset;
  }
  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, max_count+buffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + buffer_offset;


  unsigned iContext = 0;

  for (; iContext < num_contexts; ++iContext)
  {

    if (task_id == 0)
      printf("# Context: %u\n", iContext);

    /*  Query the world geometry for barrier algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               barrier_xfer,
                               barrier_num_algorithm,
                               &bar_always_works_algo,
                               &bar_always_works_md,
                               &bar_must_query_algo,
                               &bar_must_query_md);

    if (rc == 1)
      return 1;

    /*  Query the world geometry for scatter algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               scatter_xfer,
                               scatter_num_algorithm,
                               &scatter_always_works_algo,
                               &scatter_always_works_md,
                               &scatter_must_query_algo,
                               &scatter_must_query_md);

    if (rc == 1)
      return 1;

    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];
    blocking_coll(context[iContext], &barrier, &bar_poll_flag);

    for (nalg = 0; nalg < scatter_num_algorithm[0]; nalg++)
    {
      scatter.cb_done    = cb_done;
      scatter.cookie     = (void*) & scatter_poll_flag;
      scatter.algorithm  = scatter_always_works_algo[nalg];
      scatter.cmd.xfer_scatter.root       = root;
      scatter.cmd.xfer_scatter.sndbuf     = buf;
      scatter.cmd.xfer_scatter.stype      = PAMI_TYPE_CONTIGUOUS;
      scatter.cmd.xfer_scatter.stypecount = 0;
      scatter.cmd.xfer_scatter.rcvbuf     = rbuf;
      scatter.cmd.xfer_scatter.rtype      = PAMI_TYPE_CONTIGUOUS;
      scatter.cmd.xfer_scatter.rtypecount = 0;

      protocolName = scatter_always_works_md[nalg].name;

      if (task_id == root)
      {
        printf("# Scatter Bandwidth Test -- context = %d, protocol: %s\n", 
               iContext, protocolName);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(scatter_always_works_md[nalg].name,selected) == NULL) && selector) ||
          ((strstr(scatter_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

      int i, j;

      for (i = 1; i <= max_count; i *= 2)
      {
        long long dataSent = i;
        int          niter;

        if (dataSent < CUTOFF)
          niter = niterlat;
        else
          niter = NITERBW;

        scatter.cmd.xfer_scatter.stypecount = i;
        scatter.cmd.xfer_scatter.rtypecount = i;

#ifdef CHECK_DATA
        if (task_id == root)
          initialize_sndbuf (buf, i, num_tasks);
        memset(rbuf, 0xFF, i);

#endif
        blocking_coll(context[iContext], &barrier, &bar_poll_flag);
        ti = timer();

        for (j = 0; j < niter; j++)
        {
          blocking_coll(context[iContext], &scatter, &scatter_poll_flag);
        }

        tf = timer();
        blocking_coll(context[iContext], &barrier, &bar_poll_flag);

#ifdef CHECK_DATA
        rc |= check_rcvbuf (rbuf, i, task_id);
#endif
        usec = (tf - ti) / (double)niter;

        if (task_id == root)
        {
          printf("  %11lld %16lld %14.1f %12.2f\n",
                 dataSent,
                 0LL,
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
    free(scatter_always_works_algo);
    free(scatter_always_works_md);
    free(scatter_must_query_algo);
    free(scatter_must_query_md);

  } /*for(unsigned iContext = 0; iContext < num_contexts; ++iContexts)*/

  if (task_id == root)
  {
    buf = (char*)buf - buffer_offset;
    free(buf);
  }
  rbuf = (char*)rbuf - buffer_offset;
  free(rbuf);

  rc |= pami_shutdown(&client, context, &num_contexts);
  return rc;
}
