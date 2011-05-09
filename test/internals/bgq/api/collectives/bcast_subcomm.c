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

#define COUNT     (1048576*8)   /* see envvar TEST_COUNT for overrides */
/*
#define OFFSET     0
#define NITERLAT   1
#define NITERBW    MIN(10, niterlat/100+1)
#define CUTOFF     65536
*/

#include "../../../../api/pami_util.h"

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
      fprintf(stderr, "%s:Check(%d) failed <%p>rbuf[%d]=%.2u != %.2u \n", gProtocolName, bytes, rbuf, i - 1, cbuf[i-1], c);
      return 1;
    }

    c++;
  }

  return 0;
}

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_task_t          task_id, local_task_id;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;
  int                  root = 0, nalg = 0;

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

  /* Process environment variables and setup globals */
  setup_env();

  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);

  /*  Initialize PAMI */
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

  if (num_tasks == 1)
  {
    fprintf(stderr, "No subcomms on 1 node\n");
    return 0;
  }

  /*  Allocate buffer(s) */
  int err = 0;
  void* buf = NULL;
  err = posix_memalign(&buf, 128, gMax_count + gBuffer_offset);
  assert(err == 0);
  buf = (char*)buf + gBuffer_offset;


  /*  Query the world geometry for barrier algorithms */
  rc |= query_geometry_world(client,
                             context[0],
                             &world_geometry,
                             barrier_xfer,
                             barrier_num_algorithm,
                             &bar_always_works_algo,
                             &bar_always_works_md,
                             &bar_must_query_algo,
                             &bar_must_query_md);

  if (rc == 1)
    return 1;

  /*  Set up world barrier */
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];

  unsigned iContext = 0;

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
  int                    id;

  range     = (pami_geometry_range_t *)malloc(((num_tasks + 1) / 2) * sizeof(pami_geometry_range_t));

  int unused_non_root[2];
  get_split_method(&num_tasks, task_id, &rangecount, range, &local_task_id, set, &id, &root,unused_non_root);

  for (; iContext < gNum_contexts; ++iContext)
  {

    if (task_id == root)
      printf("# Context: %u\n", iContext);

    /* Delay root tasks, and emulate that he's doing "other"
       message passing.  This will cause the geometry_create
       request from other nodes to be unexpected when doing
       parentless geometries and won't affect parented.      */
    if (task_id == root)
    {
      delayTest(1);
      unsigned ii = 0;

      for (; ii < gNum_contexts; ++ii)
        PAMI_Context_advance (context[ii], 1000);
    }

    rc |= create_and_query_geometry(client,
                                    context[0],
                                    context[iContext],
                                    gParentless ? PAMI_GEOMETRY_NULL : world_geometry,
                                    &newgeometry,
                                    range,
                                    rangecount,
                                    id + iContext, /* Unique id for each context */
                                    barrier_xfer,
                                    newbar_num_algo,
                                    &newbar_algo,
                                    &newbar_md,
                                    &q_newbar_algo,
                                    &q_newbar_md);

    if (rc == 1)
      return 1;

    /*  Query the sub geometry for bcast algorithms */
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

    /*  Set up sub geometry barrier */
    newbarrier.cb_done   = cb_done;
    newbarrier.cookie    = (void*) & newbar_poll_flag;
    newbarrier.algorithm = newbar_algo[0];


    for (nalg = 0; nalg < newbcast_num_algo[0]; nalg++)
    {

      /*  Set up sub geometry bcast */
      newbcast.cb_done                      = cb_done;
      newbcast.cookie                       = (void*) & bcast_poll_flag;
      newbcast.algorithm                    = newbcast_algo[nalg];
      newbcast.cmd.xfer_broadcast.root      = root;
      newbcast.cmd.xfer_broadcast.buf       = buf;
      newbcast.cmd.xfer_broadcast.type      = PAMI_TYPE_BYTE;
      newbcast.cmd.xfer_broadcast.typecount = 0;


      int             k;

      gProtocolName = newbcast_md[nalg].name;

      for (k = 1; k >= 0; k--)
      {
        if (set[k])
        {
          if (task_id == root)
          {
            printf("# Broadcast Bandwidth Test -- context = %d, root = %d  protocol: %s\n",
                   iContext, root, gProtocolName);
            printf("# Size(bytes)           cycles    bytes/sec    usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }

          if (((strstr(newbcast_md[nalg].name,gSelected) == NULL) && gSelector) ||
              ((strstr(newbcast_md[nalg].name,gSelected) != NULL) && !gSelector))  continue;

          blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);

          int i, j;

          for (i = 1; i <= gMax_count; i *= 2)
          {
            size_t  dataSent = i;
            int          niter;

            if (dataSent < CUTOFF)
              niter = gNiterlat;
            else
              niter = NITERBW;

            newbcast.cmd.xfer_broadcast.root      = root;
            newbcast.cmd.xfer_broadcast.buf       = buf;
            newbcast.cmd.xfer_broadcast.typecount = i;

            if (task_id == (size_t)root)
              initialize_sndbuf (buf, i, root);
            else
              memset(buf, 0xFF, i);

            blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);
            ti = timer();

            for (j = 0; j < niter; j++)
            {
              blocking_coll(context[iContext], &newbcast, &bcast_poll_flag);
            }

            tf = timer();
            blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);
            int rc_check;
            rc |= rc_check = check_rcvbuf (buf, i, root);

            if (rc_check) fprintf(stderr, "%s FAILED validation\n", gProtocolName);

            usec = (tf - ti) / (double)niter;

            if (task_id == (size_t)root)
            {
              printf("  %11lld %16d %14.1f %12.2f\n",
                     (long long)dataSent,
                     niter,
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

    /* We aren't testing world barrier itself, so use context 0.*/
    blocking_coll(context[0], &barrier, &bar_poll_flag);

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

  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  buf = (char*)buf - gBuffer_offset;
  free(buf);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}
