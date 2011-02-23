/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/api/collectives/bcast_subcomm.c
 * \brief Simple Bcast test on sub-geometries
 */

#include "../../../../api/pami_util.h"

/*define this if you want to validate the data */
#define CHECK_DATA

#define COUNT     (1048576*8)   // see envvar TEST_COUNT for overrides
unsigned max_count = COUNT;

#define OFFSET     0            // see envvar TEST_OFFSET for overrides
unsigned buffer_offset = OFFSET;

#define NITERLAT   1            // see envvar TEST_ITER for overrides
unsigned niterlat  = NITERLAT;

#define NITERBW    MIN(10, niterlat/100+1)

#define CUTOFF     65536

char *protocolName;

void initialize_sndbuf (void *sbuf, int bytes, int root)
{

  unsigned char c = root;
  int i = bytes;
  unsigned char *cbuf = (unsigned char *)  sbuf;

  for (; i; i--)
  {
    cbuf[i-1] = (c++);
  }
}

int check_rcvbuf (void *rbuf, int bytes, int root)
{
  unsigned char c = root;
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
  pami_xfer_t          barrier;
  volatile unsigned    bar_poll_flag = 0;
  volatile unsigned    newbar_poll_flag = 0;

  /* Bcast variables */
  pami_xfer_type_t     bcast_xfer = PAMI_XFER_BROADCAST;
  volatile unsigned    bcast_poll_flag = 0;

  double               ti, tf, usec;

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

  // Override COUNT
  if (sCount) max_count = atoi(sCount);

  /* \note Test environment variable" TEST_OFFSET=N buffer offset/alignment*/
  char* sOffset = getenv("TEST_OFFSET");

  // Override OFFSET
  if (sOffset) buffer_offset = atoi(sOffset);

  /* \note Test environment variable" TEST_ITER=N iterations      */
  char* sIter = getenv("TEST_ITER");

  // Override NITERLAT
  if (sIter) niterlat = atoi(sIter);

  /* \note Test environment variable" TEST_PARENTLESS=0 or 1, defaults to 0.
     0 - world_geometry is the parent
     1 - parentless                                                      */
  char* sParentless = getenv("TEST_PARENTLESS");
  unsigned parentless = 0; /*Not parentless*/

  if (sParentless) parentless = atoi(sParentless);

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
  if (num_tasks == 1)
  {
    fprintf(stderr,"No subcomms on 1 node\n");
    return 0;
  }

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

    /*  Create the subgeometry */
    pami_geometry_range_t *range;
    int                    rangecount;
    pami_geometry_t        newgeometry;
    size_t                 newbar_num_algo[2];
    size_t                 newbcast_num_algo[2];
    pami_algorithm_t      *newbar_algo        = NULL;
    pami_metadata_t       *newbar_md          = NULL;
    pami_algorithm_t      *q_newbar_algo      = NULL;
    pami_metadata_t       *q_newbar_md        = NULL;

    pami_algorithm_t      *newbcast_algo      = NULL;
    pami_metadata_t       *newbcast_md        = NULL;
    pami_algorithm_t      *q_newbcast_algo    = NULL;
    pami_metadata_t       *q_newbcast_md      = NULL;
    pami_xfer_t            newbarrier;
    pami_xfer_t            newbcast;

    size_t                 set[2];
    int                    id, root = 0;
    size_t                 half        = num_tasks / 2;
    range     = (pami_geometry_range_t *)malloc(((num_tasks + 1) / 2) * sizeof(pami_geometry_range_t));

    char *method = getenv("TEST_SPLIT_METHOD");

    // Default or TEST_SPLIT_METHOD=0 : divide in half
    if ((!method || !strcmp(method, "0")))
    {
      if (task_id >= 0 && task_id <= half - 1)
      {
        range[0].lo = 0;
        range[0].hi = half - 1;
        set[0]   = 1;
        set[1]   = 0;
        id       = 1;
        root     = 0;
      }
      else
      {
        range[0].lo = half;
        range[0].hi = num_tasks - 1;
        set[0]   = 0;
        set[1]   = 1;
        id       = 2;
        root     = half;
      }

      rangecount = 1;
    }
    // TEST_SPLIT_METHOD=-1 : alternate ranks
    else if ((method && !strcmp(method, "-1")))
    {
      int i = 0;
      int iter = 0;;

      if ((task_id % 2) == 0)
      {
        for (i = 0; i < num_tasks; i++)
        {
          if ((i % 2) == 0)
          {
            range[iter].lo = i;
            range[iter].hi = i;
            iter++;
          }
        }

        set[0]   = 1;
        set[1]   = 0;
        id       = 2;
        root     = 0;
        rangecount = iter;
      }
      else
      {
        for (i = 0; i < num_tasks; i++)
        {
          if ((i % 2) != 0)
          {
            range[iter].lo = i;
            range[iter].hi = i;
            iter++;
          }
        }

        set[0]   = 0;
        set[1]   = 1;
        id       = 2;
        root     = 1;
        rangecount = iter;
      }

    }
    // TEST_SPLIT_METHOD=N : Split the first "N" processes into a communicator
    else
    {
      half = atoi(method);
      if (task_id >= 0 && task_id <= half - 1)
      {
        range[0].lo = 0;
        range[0].hi = half - 1;
        set[0]   = 1;
        set[1]   = 0;
        id       = 1;
        root     = 0;
      }
      else
      {
        range[0].lo = half;
        range[0].hi = num_tasks - 1;
        set[0]   = 0;
        set[1]   = 1;
        id       = 2;
        root     = half;
      }

      rangecount = 1;
    }

    /* Delay root tasks, and emulate that he's doing "other"
       message passing.  This will cause the geometry_create
       request from other nodes to be unexpected when doing
       parentless geometries and won't affect parented.      */
    if (task_id == root)
    {
      delayTest(1);
      unsigned ii = 0;

      for (; ii < num_contexts; ++ii)
        PAMI_Context_advance (context[ii], 1000);
    }

    rc |= create_and_query_geometry(client,
                                   context[iContext],
                                   parentless ? PAMI_NULL_GEOMETRY : world_geometry,
                                   &newgeometry,
                                   range,
                                   rangecount,
                                   id + iContext, // Unique id for each context
                                   barrier_xfer,
                                   newbar_num_algo,
                                   &newbar_algo,
                                   &newbar_md,
                                   &q_newbar_algo,
                                   &q_newbar_md);

    if (rc == 1)
      return 1;

    rc |= query_geometry(client,
                        context[iContext],
                        newgeometry,
                        bcast_xfer,
                        newbcast_num_algo,
                        &newbcast_algo,
                        &newbcast_md,
                        &q_newbcast_algo,
                        &q_newbcast_md);

    if (rc == 1)
      return 1;

    /*  Set up world barrier */
    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];

    /*  Set up sub geometry barrier */
    newbarrier.cb_done   = cb_done;
    newbarrier.cookie    = (void*) & newbar_poll_flag;
    newbarrier.algorithm = newbar_algo[0];

    int nalg;

    for (nalg = 0; nalg < newbcast_num_algo[0]; nalg++)
    {

      /*  Set up sub geometry bcast */
      newbcast.cb_done                      = cb_done;
      newbcast.cookie                       = (void*) & bcast_poll_flag;
      newbcast.algorithm                    = newbcast_algo[nalg];
      newbcast.cmd.xfer_broadcast.root      = root;
      newbcast.cmd.xfer_broadcast.buf       = buf;
      newbcast.cmd.xfer_broadcast.type      = PAMI_TYPE_CONTIGUOUS;
      newbcast.cmd.xfer_broadcast.typecount = 0;


      int             i, j, k;

      protocolName = newbcast_md[nalg].name;

      for (k = 1; k >= 0; k--)
      {
        if (set[k])
        {
          if (task_id == root)
          {
            printf("# Broadcast Bandwidth Test -- context = %d, root = %d  protocol: %s\n",
                   iContext, root, newbcast_md[nalg].name);
            printf("# Size(bytes)           cycles    bytes/sec    usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }

          fflush(stdout);

          if (((strstr(newbcast_md[nalg].name, selected) == NULL) && selector) ||
              ((strstr(newbcast_md[nalg].name, selected) != NULL) && !selector))  continue;

          blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);

          for (i = 1; i <= max_count; i *= 2)
          {
            long long dataSent = i;
            int          niter;

            if (dataSent < CUTOFF)
              niter = niterlat;
            else
              niter = NITERBW;

            newbcast.cmd.xfer_broadcast.root      = root;
            newbcast.cmd.xfer_broadcast.buf       = buf;
            newbcast.cmd.xfer_broadcast.typecount = i;

#ifdef CHECK_DATA

            if (task_id == (size_t)root)
              initialize_sndbuf (buf, i, root);
            else
              memset(buf, 0xFF, i);

#endif
            blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);
            ti = timer();

            for (j = 0; j < niter; j++)
            {
              blocking_coll(context[iContext], &newbcast, &bcast_poll_flag);
            }

            tf = timer();
            blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);
#ifdef CHECK_DATA
            check_rcvbuf (buf, i, root);
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

        blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);
        fflush(stderr);
      }
    }

    blocking_coll(context[iContext], &barrier, &bar_poll_flag);

    free(bar_always_works_algo);
    free(bar_always_works_md);
    free(bar_must_query_algo);
    free(bar_must_query_md);

    free(newbar_algo);
    free(newbar_md);
    free(q_newbar_algo);
    free(q_newbar_md);

    free(newbcast_algo);
    free(newbcast_md);
    free(q_newbcast_algo);
    free(q_newbcast_md);

  } /*for(unsigned iContext = 0; iContext < num_contexts; ++iContexts)*/

  buf = (char*)buf - buffer_offset;
  free(buf);

  rc |= pami_shutdown(&client, context, &num_contexts);
  return rc;
}
