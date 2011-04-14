/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/api/collectives/alltoallv.c
 * \brief Simple Alltoallv test on world geometry (only bytes)
 */

#include "../../../../api/pami_util.h"


/*define this if you want to validate the data */
#define CHECK_DATA


#define COUNT     4096          /* see envvar TEST_COUNT for overrides */
unsigned max_count = COUNT;

#define OFFSET     0            /* see envvar TEST_OFFSET for overrides */
unsigned buffer_offset = OFFSET;

#define NITERLAT   100          /* see envvar TEST_ITER for overrides */
unsigned niterlat  = NITERLAT;

#define NITERBW    MIN(10, niterlat/100+1)

#define CUTOFF     1024

char* protocolName;

size_t *sndlens = NULL;
size_t *sdispls = NULL;
size_t *rcvlens = NULL;
size_t *rdispls = NULL;

void initialize_sndbuf(size_t r,char *sbuf,char *rbuf)
{
  size_t k;

  for (k = 0; k < sndlens[r]; k++)
  {
    sbuf[ sdispls[r] + k ] = ((r + k) & 0xff);
    rbuf[ rdispls[r] + k ] = 0xff;
  }
}

int check_rcvbuf(size_t sz, size_t myrank,char *rbuf)
{
  size_t r, k;

  for (r = 0; r < sz; r++)
    for (k = 0; k < rcvlens[r]; k++)
    {
      if (rbuf[ rdispls[r] + k ] != (char)((myrank + k) & 0xff))
      {
        fprintf(stderr,"%s:Check(%zu) failed rbuf[%zu+%zu]:%02x instead of %02zx (rank:%zu)\n",
                protocolName,sndlens[r],
                rdispls[r],k,
                rbuf[ rdispls[r] + k ],
                ((r + k) & 0xff),
                r );
        return 1;
      }
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

  /* Alltoallv variables */
  size_t               alltoallv_num_algorithm[2];
  pami_algorithm_t    *alltoallv_always_works_algo = NULL;
  pami_metadata_t     *alltoallv_always_works_md = NULL;
  pami_algorithm_t    *alltoallv_must_query_algo = NULL;
  pami_metadata_t     *alltoallv_must_query_md = NULL;
  pami_xfer_type_t     alltoallv_xfer = PAMI_XFER_ALLTOALLV;
  volatile unsigned    alltoallv_poll_flag = 0;

  int                  nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          alltoallv;

  /* \note Test environment variable" TEST_VERBOSE=N     */
  char* sVerbose = getenv("TEST_VERBOSE");

  if (sVerbose) gVerbose=atoi(sVerbose); /* set the global defined in coll_util.h */

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
  void* sbuf = NULL;
  err = posix_memalign((void*)&sbuf, 128, (max_count*num_tasks)+buffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + buffer_offset;

  void* rbuf = NULL;
  err = posix_memalign((void*)&rbuf, 128, (max_count*num_tasks)+buffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + buffer_offset;

  sndlens = (size_t*) malloc(num_tasks * sizeof(size_t));
  assert(sndlens);
  sdispls = (size_t*) malloc(num_tasks * sizeof(size_t));
  assert(sdispls);
  rcvlens = (size_t*) malloc(num_tasks * sizeof(size_t));
  assert(rcvlens);
  rdispls = (size_t*) malloc(num_tasks * sizeof(size_t));
  assert(rdispls);

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

    /*  Query the world geometry for alltoallv algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               alltoallv_xfer,
                               alltoallv_num_algorithm,
                               &alltoallv_always_works_algo,
                               &alltoallv_always_works_md,
                               &alltoallv_must_query_algo,
                               &alltoallv_must_query_md);

    if (rc == 1)
      return 1;

    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];
    blocking_coll(context[iContext], &barrier, &bar_poll_flag);

    for (nalg = 0; nalg < alltoallv_num_algorithm[0]; nalg++)
    {
      alltoallv.cb_done    = cb_done;
      alltoallv.cookie     = (void*) & alltoallv_poll_flag;
      alltoallv.algorithm  = alltoallv_always_works_algo[nalg];
      alltoallv.cmd.xfer_alltoallv.sndbuf        = sbuf;
      alltoallv.cmd.xfer_alltoallv.stype         = PAMI_TYPE_CONTIGUOUS;
      alltoallv.cmd.xfer_alltoallv.stypecounts   = sndlens;
      alltoallv.cmd.xfer_alltoallv.sdispls       = sdispls;
      alltoallv.cmd.xfer_alltoallv.rcvbuf        = rbuf;
      alltoallv.cmd.xfer_alltoallv.rtype         = PAMI_TYPE_CONTIGUOUS;
      alltoallv.cmd.xfer_alltoallv.rtypecounts   = rcvlens;
      alltoallv.cmd.xfer_alltoallv.rdispls       = rdispls;

      protocolName = alltoallv_always_works_md[nalg].name;

      if (task_id == 0)
      {
        printf("# Alltoallv Bandwidth Test(size:%zu) -- context = %d, protocol: %s\n", 
               num_tasks, iContext, alltoallv_always_works_md[nalg].name);
        printf("# Size(bytes)           cycles    bytes/sec      usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }
      if (((strstr(alltoallv_always_works_md[nalg].name, selected) == NULL) && selector) ||
          ((strstr(alltoallv_always_works_md[nalg].name, selected) != NULL) && !selector))  continue;

      int i, j;

      for (i = 1; i <= max_count; i *= 2)
      {
        long long dataSent = i;
        int          niter;

        if (dataSent < CUTOFF)
          niter = niterlat;
        else
          niter = NITERBW;

        for (j = 0; j < num_tasks; j++)
        {
          sndlens[j] = rcvlens[j] = i;
          sdispls[j] = rdispls[j] = i * j;
#ifdef CHECK_DATA
          initialize_sndbuf( j,sbuf,rbuf );
#endif
        }

        blocking_coll(context[iContext], &barrier, &bar_poll_flag);

        /* Warmup */
        blocking_coll(context[iContext], &alltoallv, &alltoallv_poll_flag);

        blocking_coll(context[iContext], &barrier, &bar_poll_flag);

        ti = timer();

        for (j = 0; j < niter; j++)
        {
          blocking_coll(context[iContext], &alltoallv, &alltoallv_poll_flag);
        }

        tf = timer();
        blocking_coll(context[iContext], &barrier, &bar_poll_flag);

#ifdef CHECK_DATA
        rc |= check_rcvbuf(num_tasks, task_id, rbuf);
#endif
        usec = (tf - ti) / (double)niter;

        if (task_id == 0)
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
    free(alltoallv_always_works_algo);
    free(alltoallv_always_works_md);
    free(alltoallv_must_query_algo);
    free(alltoallv_must_query_md);
  } /*for(unsigned iContext = 0; iContext < num_contexts; ++iContexts)*/
  sbuf = (char*)sbuf - buffer_offset;
  free(sbuf);

  rbuf = (char*)rbuf - buffer_offset;
  free(rbuf);

  free(sndlens);
  free(sdispls);
  free(rcvlens);
  free(rdispls);

  rc |= pami_shutdown(&client, context, &num_contexts);
  return rc;
}
