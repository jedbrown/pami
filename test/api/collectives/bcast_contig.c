/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/bcast.c
 * \brief Simple Bcast test on world geometry
 */

#define COUNT     (1048576*8)   /* see envvar TEST_COUNT for overrides */
#define NITERLAT   10
/*
#define OFFSET     0
#define NITERBW    MIN(10, niterlat/100+1)
#define CUTOFF     65536
*/

#include "../pami_util.h"

void initialize_sndbuf (void *sbuf, int count, int root, int dt)
{
  int i = count;

  if (dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *ibuf = (unsigned int *)  sbuf;
    unsigned int u = root;
    for (; i; i--)
    {
      ibuf[i-1] = (u++);
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *dbuf = (double *)  sbuf;
    double d = root * 1.0;
    for (; i; i--)
    {
      dbuf[i-1] = d;
      d = d + 1.0;
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *fbuf = (float *)  sbuf;
    float f = root * 1.0;
    for (; i; i--)
    {
      fbuf[i-1] = f;
      f = f + 1.0;
    }
  }
  else
  {
    char *cbuf = (char *)  sbuf;
    char c = root;
    for (; i; i--)
    {
      cbuf[i-1] = (c++);
    }
  }
}

int check_rcvbuf (void *rbuf, int count, int root, int dt)
{
  int i = count;

  if (dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *ibuf = (unsigned int *)  rbuf;
    unsigned int u = root;
    for (; i; i--)
    {
      if (ibuf[i-1] != u)
      {
        fprintf(stderr, "%s:Check(%d) failed <%p>rbuf[%d]=%.2u != %.2u \n", gProtocolName, count, rbuf, i - 1, ibuf[i-1], u);
        return 1;
      }

      u++;
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *dbuf = (double *)  rbuf;
    double d = root * 1.0;
    for (; i; i--)
    {
      if (dbuf[i-1] != d)
      {
        fprintf(stderr, "%s:Check(%d) failed <%p>rbuf[%d]=%.2f != %.2f \n", gProtocolName, count, rbuf, i - 1, dbuf[i-1], d);
        return 1;
      }

      d = d + 1.0;
    }
  }
  else   if (dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *fbuf = (float *)  rbuf;
    float f = root * 1.0;
    for (; i; i--)
    {
      if (fbuf[i-1] != f)
      {
        fprintf(stderr, "%s:Check(%d) failed <%p>rbuf[%d]=%.2f != %.2f \n", gProtocolName, count, rbuf, i - 1, fbuf[i-1], f);
        return 1;
      }

      f = f + 1.0;
    }
  }
  else
  {
    unsigned char c = root;
    unsigned char *cbuf = (unsigned char *)  rbuf;

    for (; i; i--)
    {
      if (cbuf[i-1] != c)
      {
        fprintf(stderr, "%s:Check(%d) failed <%p>rbuf[%d]=%.2u != %.2u \n", gProtocolName, count, rbuf, i - 1, cbuf[i-1], c);
        return 1;
      }

      c++;
    }
  }

  return 0;
}

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
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
  pami_metadata_t     *bcast_always_works_md   = NULL;
  pami_algorithm_t    *bcast_must_query_algo   = NULL;
  pami_metadata_t     *bcast_must_query_md     = NULL;
  pami_xfer_type_t     bcast_xfer = PAMI_XFER_BROADCAST;
  volatile unsigned    bcast_poll_flag = 0;

  int                  nalg= 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          broadcast;

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

  if(gNumRoots == -1) gNumRoots = num_tasks;

  /*  Allocate buffer(s) */
  int err = 0;
  void* buf = NULL;
  err = posix_memalign(&buf, 128, MAXBUFSIZE + gBuffer_offset);
  assert(err == 0);
  buf = (char*)buf + gBuffer_offset;


  unsigned iContext = 0;

  for (; iContext < gNum_contexts; ++iContext)
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

    for (nalg = 0; nalg < bcast_num_algorithm[0]; nalg++)
    {

      broadcast.cb_done                      = cb_done;
      broadcast.cookie                       = (void*) & bcast_poll_flag;
      broadcast.algorithm                    = bcast_always_works_algo[nalg];
      broadcast.cmd.xfer_broadcast.buf       = buf;
      broadcast.cmd.xfer_broadcast.type      = PAMI_TYPE_BYTE;
      broadcast.cmd.xfer_broadcast.typecount = 0;

      gProtocolName = bcast_always_works_md[nalg].name;

      int k;
      for (k=0; k< gNumRoots; k++)
      {
        pami_endpoint_t    root_ep;
        pami_task_t root_task = (pami_task_t)k;
        PAMI_Endpoint_create(client, root_task, 0, &root_ep);
        broadcast.cmd.xfer_broadcast.root = root_ep;
        if (task_id == root_task)
        {
          printf("# Broadcast Bandwidth Test -- context = %d, root = %d  protocol: %s\n",
                 iContext, root_task, gProtocolName);
          printf("# Size(bytes)           cycles    bytes/sec    usec\n");
          printf("# -----------      -----------    -----------    ---------\n");
        }

        if (((strstr(bcast_always_works_md[nalg].name,gSelected) == NULL) && gSelector) ||
            ((strstr(bcast_always_works_md[nalg].name,gSelected) != NULL) && !gSelector))  continue;

        int i, j;
        int dt,op=4/*SUM*/;

        for (dt = 0; dt < dt_count; dt++)
        {
          if (gValidTable[op][dt])
          {
              if (task_id == 0)
                printf("Running Broadcast: %s\n", dt_array_str[dt]);

              for (i = 1; i <= gMax_count; i *= 2)
              {
                size_t  dataSent = i;
                int          niter;

                if (dataSent < CUTOFF)
                  niter = gNiterlat;
                else
                  niter = NITERBW;

                broadcast.cmd.xfer_broadcast.typecount = i;
                broadcast.cmd.xfer_broadcast.type      = dt_array[dt];

                if (task_id == root_task)
                  initialize_sndbuf (buf, i, root_task, dt);
                else
                  memset(buf, 0xFF, i);

                blocking_coll(context[iContext], &barrier, &bar_poll_flag);
                ti = timer();

                for (j = 0; j < niter; j++)
                {
                  blocking_coll (context[iContext], &broadcast, &bcast_poll_flag);
                }

                blocking_coll(context[iContext], &barrier, &bar_poll_flag);
                tf = timer();
                int rc_check;
                rc |= rc_check = check_rcvbuf (buf, i, root_task, dt);

                if (rc_check) fprintf(stderr, "%s FAILED validation\n", gProtocolName);

                usec = (tf - ti) / (double)niter;

                if (task_id == root_task)
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

  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  buf = (char*)buf - gBuffer_offset;
  free(buf);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}
