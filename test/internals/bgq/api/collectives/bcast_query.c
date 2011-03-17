/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/api/collectives/bcast_query.c
 * \brief Simple Bcast test on world geometry using "must query" algorithms
 */

#include "../../../../api/pami_util.h"
#include <Arch.h> /* Don't use PAMI_MAX_PROC_PER_NODE in 'real' api test*/

/*define this if you want to validate the data */
#define CHECK_DATA

#define COUNT     (1048576*8)   /* see envvar TEST_COUNT for overrides */
unsigned max_count = COUNT;

#define OFFSET     0            /* see envvar TEST_OFFSET for overrides */
unsigned buffer_offset = OFFSET;

#define NITERLAT   1            /* see envvar TEST_ITER for overrides */
unsigned niterlat  = NITERLAT;

#define NITERBW    MIN(10, niterlat/100+1)

#define CUTOFF     65536

char* protocolName;

void initialize_sndbuf (void *sbuf, int bytes)
{

  unsigned char c = 0x00;
  int i = bytes;
  unsigned char *cbuf = (unsigned char *)  sbuf;

  for (; i; i--)
  {
    cbuf[i-1] = c++;
  }
}

int check_rcvbuf (void *rbuf, int bytes)
{
  unsigned char c = 0x00;
  int i = bytes;
  unsigned char *cbuf = (unsigned char *)  rbuf;

  for (; i; i--)
  {
    if (cbuf[i-1] != c)
    {
      fprintf(stderr, "%s:Check(%d) failed <%p>rbuf[%d]=%.2u != %.2u \n", protocolName, bytes, rbuf, i - 1, cbuf[i-1], c);
      return -1;
    }

    c++;
  }

  return 0;
}

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context[PAMI_MAX_PROC_PER_NODE]; /* arbitrary max */
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

  /* Bcast variables */
  size_t               bcast_num_algorithm[2];
  pami_algorithm_t    *bcast_always_works_algo = NULL;
  pami_metadata_t     *bcast_always_works_md = NULL;
  pami_algorithm_t    *bcast_must_query_algo = NULL;
  pami_metadata_t     *bcast_must_query_md = NULL;
  pami_xfer_type_t     bcast_xfer = PAMI_XFER_BROADCAST;
  volatile unsigned    bcast_poll_flag = 0;

  int                  nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          broadcast;

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
  assert(num_contexts <= PAMI_MAX_PROC_PER_NODE);

  /*  Allocate buffer(s) */
  int err = 0;
  void* buf = NULL;
  err = posix_memalign(&buf, 128, max_count+buffer_offset);
  assert(err == 0);
  buf = (char*)buf + buffer_offset;

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

    /*  Query the world geometry for broadcast algorithms */
    rc |= query_geometry_world(client,
                              context[iContext],
                              &world_geometry,
                              bcast_xfer,
                              bcast_num_algorithm,
                              &bcast_always_works_algo,
                              &bcast_always_works_md,
                              &bcast_must_query_algo,
                              &bcast_must_query_md);

    if (rc == 1)
      return 1;

    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];
    blocking_coll(context[iContext], &barrier, &bar_poll_flag);

    for (nalg = 0; nalg < bcast_num_algorithm[1]; nalg++)
    {
      int         root = 0;
      broadcast.cb_done                      = cb_done;
      broadcast.cookie                       = (void*) & bcast_poll_flag;
      broadcast.algorithm                    = bcast_must_query_algo[nalg];
      broadcast.cmd.xfer_broadcast.root      = root;
      broadcast.cmd.xfer_broadcast.buf       = buf;
      broadcast.cmd.xfer_broadcast.type      = PAMI_TYPE_CONTIGUOUS;
      broadcast.cmd.xfer_broadcast.typecount = 0;

      protocolName = bcast_must_query_md[nalg].name;
      metadata_result_t result = {0};

      if (task_id == (size_t)root)
      {
        printf("# Broadcast Bandwidth Test -- context = %d, root = %d  protocol: %s, Metadata: range %zu <-> %zd, mask %#X\n",
               iContext, root, protocolName,
               bcast_must_query_md[nalg].range_lo, bcast_must_query_md[nalg].range_hi,
               bcast_must_query_md[nalg].check_correct.bitmask_correct);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(bcast_must_query_md[nalg].name, selected) == NULL) && selector) ||
          ((strstr(bcast_must_query_md[nalg].name, selected) != NULL) && !selector))  continue;

      unsigned checkrequired = bcast_must_query_md[nalg].check_correct.values.checkrequired; /*must query every time */
      assert(!checkrequired || bcast_must_query_md[nalg].check_fn); /* must have function if checkrequired. */

      int i, j;

      for (i = 1; i <= max_count; i *= 2)
      {
        long long dataSent = i;
        int          niter;

        if (dataSent < CUTOFF)
          niter = niterlat;
        else
          niter = NITERBW;

        broadcast.cmd.xfer_broadcast.typecount = i;

        if (bcast_must_query_md[nalg].check_fn)
        {  
          result = bcast_must_query_md[nalg].check_fn(&broadcast);
        }
        else // Must check parameters ourselves...
        {
          uint64_t  mask=0;
          result.bitmask = 0;
          if(bcast_must_query_md[nalg].check_correct.values.sendminalign)
          {
            mask  = bcast_must_query_md[nalg].send_min_align - 1; 
            result.check.align_send_buffer = (((uint64_t)broadcast.cmd.xfer_broadcast.buf & (uint64_t)mask) == 0) ? 0:1;
          }
          if(bcast_must_query_md[nalg].check_correct.values.recvminalign)
          {
            mask  = bcast_must_query_md[nalg].recv_min_align - 1; 
            result.check.align_recv_buffer = (((uint64_t)broadcast.cmd.xfer_broadcast.buf & (uint64_t)mask) == 0) ? 0:1;
          }
          if(bcast_must_query_md[nalg].check_correct.values.rangeminmax)
          {
            result.check.range = !((dataSent <= bcast_must_query_md[nalg].range_hi) &&
                                   (dataSent >= bcast_must_query_md[nalg].range_lo));
          }   
          if(bcast_must_query_md[nalg].check_correct.values.contigsflags)
            ; //   This test is always PAMI_TYPE_CONTIGUOUS
          if(bcast_must_query_md[nalg].check_correct.values.contigrflags)
            ; // This test is always PAMI_TYPE_CONTIGUOUS
          if(bcast_must_query_md[nalg].check_correct.values.continsflags)
            ; // This test is always PAMI_TYPE_CONTIGUOUS and continuous
          if(bcast_must_query_md[nalg].check_correct.values.continrflags)
              ; // This test is always PAMI_TYPE_CONTIGUOUS and continuous
        }
            //fprintf(stderr,"result.bitmask = %.8X\n",result.bitmask);
        if (result.bitmask) continue;

        if(bcast_must_query_md[nalg].check_correct.values.nonlocal)
        {
          fprintf(stderr,"Test does not support protocols with nonlocal metadata\n");
          continue;
        }

#ifdef CHECK_DATA

        if (task_id == (size_t)root)
          initialize_sndbuf (buf, i);
        else
          memset(buf, 0xFF, i);

#endif
        blocking_coll(context[iContext], &barrier, &bar_poll_flag);
        ti = timer();

        for (j = 0; j < niter; j++)
        {
          if (checkrequired) /* must query every time */
          {
            result = bcast_must_query_md[nalg].check_fn(&broadcast);
            if (result.bitmask) continue;
          }

          blocking_coll (context[iContext], &broadcast, &bcast_poll_flag);
        }

        blocking_coll(context[iContext], &barrier, &bar_poll_flag);
        tf = timer();
#ifdef CHECK_DATA
        check_rcvbuf (buf, i);
#endif
        usec = (tf - ti) / (double)niter;

        if (task_id == (size_t)root)
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
    free(bcast_always_works_algo);
    free(bcast_always_works_md);
    free(bcast_must_query_algo);
    free(bcast_must_query_md);

  } /*for(unsigned iContext = 0; iContext < num_contexts; ++iContexts)*/

  buf = (char*)buf - buffer_offset;
  free(buf);

  rc |= pami_shutdown(&client, context, &num_contexts);
  return rc;
};
