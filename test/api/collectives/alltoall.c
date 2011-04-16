/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/alltoall.c
 * \brief ???
 */
#include "../pami_util.h"

#define COUNT     4096          /* see envvar TEST_COUNT for overrides */
unsigned max_count = COUNT;

#define OFFSET     0            /* see envvar TEST_OFFSET for overrides */
unsigned buffer_offset = OFFSET;

#define NITERLAT   100          /* see envvar TEST_ITER for overrides */
unsigned niterlat  = NITERLAT;

#define NITERBW    MIN(10, niterlat/100+1)

#define CUTOFF     1024

/*#define INIT_BUFS(l,r) */
#define INIT_BUFS(l,r) init_bufs(l,r)

/*#define CHCK_BUFS */
#define CHCK_BUFS(l,s,r)    check_bufs(l,s,r)


char *sbuf = NULL;
char *rbuf = NULL;


void init_bufs(size_t l, size_t r)
{
  size_t k;
  size_t d = l * r;

  for ( k = 0; k < l; k++ )
    {
      sbuf[ d + k ] = ((r + k) & 0xff);
      rbuf[ d + k ] = 0xff;
    }
}


void check_bufs(size_t l, size_t nranks, size_t myrank)
{
  size_t r, k;

  for ( r = 0; r < nranks; r++ )
      {
        size_t d = l * r;
        for ( k = 0; k < l; k++ )
          {
            if ( rbuf[ d + k ] != (char)((myrank + k) & 0xff) )
              {
                printf("%zu: (E) rbuf[%zu]:%02x instead of %02zx (r:%zu)\n",
                       myrank,
                       d + k,
                       rbuf[ d + k ],
                       ((r + k) & 0xff),
                       r );
                exit(1);
              }
          }
      }
}


int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t       context;
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

  /* Alltoall variables */
  size_t               alltoall_num_algorithm[2];
  pami_algorithm_t    *alltoall_always_works_algo = NULL;
  pami_metadata_t     *alltoall_always_works_md = NULL;
  pami_algorithm_t    *alltoall_must_query_algo = NULL;
  pami_metadata_t     *alltoall_must_query_md = NULL;
  pami_xfer_type_t     alltoall_xfer = PAMI_XFER_ALLTOALL;
  volatile unsigned    alltoall_poll_flag = 0;

  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          alltoall;

  /* \note Test environment variable" TEST_VERBOSE=N     */
  char* sVerbose = getenv("TEST_VERBOSE");

  if(sVerbose) gVerbose=atoi(sVerbose); /* set the global defined in coll_util.h */

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

  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     &context,       /* Context            */
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
  err = posix_memalign((void*)&sbuf, 128, (max_count*num_tasks)+buffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + buffer_offset;

  err = posix_memalign((void*)&rbuf, 128, (max_count*num_tasks)+buffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + buffer_offset;

  /*  Query the world geometry for barrier algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            barrier_xfer,
                            barrier_num_algorithm,
                            &bar_always_works_algo,
                            &bar_always_works_md,
                            &bar_must_query_algo,
                            &bar_must_query_md);

  if (rc == 1)
    return 1;

  /*  Query the world geometry for alltoall algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            alltoall_xfer,
                            alltoall_num_algorithm,
                            &alltoall_always_works_algo,
                            &alltoall_always_works_md,
                            &alltoall_must_query_algo,
                            &alltoall_must_query_md);

  if (rc == 1)
    return 1;

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];

  alltoall.cb_done    = cb_done;
  alltoall.cookie     = (void*) & alltoall_poll_flag;

  {
    int nalg = 0;

    for (nalg = 0; nalg < alltoall_num_algorithm[0]; nalg++)
      {
        size_t i, j;

        if (task_id == 0)
          {
            printf("# Alltoall Bandwidth Test(size:%zu) %p, protocol: %s\n", num_tasks, cb_done, alltoall_always_works_md[nalg].name);
            printf("# Size(bytes)       iterations    bytes/sec      usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }

        if (((strstr(alltoall_always_works_md[nalg].name, selected) == NULL) && selector) ||
            ((strstr(alltoall_always_works_md[nalg].name, selected) != NULL) && !selector))  continue;

        alltoall.algorithm  = alltoall_always_works_algo[nalg];

        for (i = 1; i <= max_count; i *= 2)
          {
            long long dataSent = i;
            int          niter;

            if (dataSent < CUTOFF)
                niter = niterlat;
            else
                niter = NITERBW;

            for ( j = 0; j < num_tasks; j++ )
              {
                INIT_BUFS(i, j );
              }

            blocking_coll(context, &barrier, &bar_poll_flag);

            /* Warmup */
            alltoall.cmd.xfer_alltoall.sndbuf        = sbuf;
            alltoall.cmd.xfer_alltoall.stype         = PAMI_TYPE_BYTE;
            alltoall.cmd.xfer_alltoall.stypecount    = i;
            alltoall.cmd.xfer_alltoall.rcvbuf        = rbuf;
            alltoall.cmd.xfer_alltoall.rtype         = PAMI_TYPE_BYTE;
            alltoall.cmd.xfer_alltoall.rtypecount    = i;

            blocking_coll(context, &alltoall, &alltoall_poll_flag);

            blocking_coll(context, &barrier, &bar_poll_flag);

            ti = timer();

            for (j = 0; j < niter; j++)
              {
                alltoall.cmd.xfer_alltoall.sndbuf        = sbuf;
                alltoall.cmd.xfer_alltoall.stype         = PAMI_TYPE_BYTE;
                alltoall.cmd.xfer_alltoall.stypecount    = i;
                alltoall.cmd.xfer_alltoall.rcvbuf        = rbuf;
                alltoall.cmd.xfer_alltoall.rtype         = PAMI_TYPE_BYTE;
                alltoall.cmd.xfer_alltoall.rtypecount    = i;
                blocking_coll(context, &alltoall, &alltoall_poll_flag);
              }

            tf = timer();

            CHCK_BUFS(i, num_tasks, task_id);

            blocking_coll(context, &barrier, &bar_poll_flag);

            usec = (tf - ti) / (double)niter;

            if (task_id == 0)
              {

                printf("  %11lld %16d %14.1f %12.2f\n",
                       dataSent,
                       niter,
                       (double)1e6*(double)dataSent / (double)usec,
                       usec);
                fflush(stdout);
              }
          }
      }
  }
  rc = pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(alltoall_always_works_algo);
  free(alltoall_always_works_md);
  free(alltoall_must_query_algo);
  free(alltoall_must_query_md);

  sbuf = (char*)sbuf - buffer_offset;
  free(sbuf);

  rbuf = (char*)rbuf - buffer_offset;
  free(rbuf);

  return 0;
}
