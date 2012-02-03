/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/allgather.c
 * \brief Simple Allgather test on world geometry (only gathers bytes)
 */

#define COUNT     (524288/16)
/*
#define OFFSET     0
#define NITERLAT   1
#define NITERBW    MIN(10, niterlat/100+1)
#define CUTOFF     65536
*/

#include "../pami_util.h"

void initialize_sndbuf (void *sbuf, int count, pami_task_t task, int dt)
{
  int i;

  if (dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *ibuf = (unsigned int *)  sbuf;

    for (i = 0; i < count; i++)
    {
      ibuf[i] = task;
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_SIGNED_INT)
  {
    int *ibuf = (int *)  sbuf;

    for (i = 0; i < count; i++)
    {
      ibuf[i] = task;
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *dbuf = (double *)  sbuf;

    for (i = 0; i < count; i++)
    {
      dbuf[i] = 1.0 * task;
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *dbuf = (float *)  sbuf;

    for (i = 0; i < count; i++)
    {
      dbuf[i] = 1.0 * task;
    }
  }
  else
  {
    size_t sz=get_type_size(dt_array[dt]);
    memset(sbuf,  task,  count * sz);
  }
}

int check_rcvbuf (void *rbuf, int count, size_t ntasks, int dt)
{
  int err = 0;

  int i,j;


  if (dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *rcvbuf = (unsigned int *)  rbuf;

    for (j = 0; j < ntasks; j++)
    {
      for(i=j*count; i<(j+1)*count; i++)
      {
        if (rcvbuf[i] != (unsigned) j)
        {
          fprintf(stderr, "%s:Check %s(%d) failed rcvbuf[%d] %u != %u\n", gProtocolName, dt_array_str[dt], count, i, rcvbuf[1], j);
          err = -1;
          return err;
        }
      }
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_SIGNED_INT)
  {
    int *rcvbuf = (int *)  rbuf;

    for (j = 0; j < ntasks; j++)
    {
      for(i=j*count; i<(j+1)*count; i++)
      {
        if (rcvbuf[i] != (int) j)
        {
          fprintf(stderr, "%s:Check %s(%d) failed rcvbuf[%d] %u != %u\n", gProtocolName, dt_array_str[dt], count, i, rcvbuf[1], j);
          err = -1;
          return err;
        }
      }
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *rcvbuf = (double *)  rbuf;

    for (j = 0; j < ntasks; j++)
    {
      for(i=j*count; i<(j+1)*count; i++)
      {
        if (rcvbuf[i] != (double) j)
        {
          fprintf(stderr, "%s:Check %s(%d) failed rcvbuf[%d] %f != %f\n", gProtocolName, dt_array_str[dt], count, i, rcvbuf[1], (double)j);
          err = -1;
          return err;
        }
      }
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *rcvbuf = (float *)  rbuf;

    for (j = 0; j < ntasks; j++)
    {
      for(i=j*count; i<(j+1)*count; i++)
      {
        if (rcvbuf[i] != (float) j)
        {
          fprintf(stderr, "%s:Check %s(%d) failed rcvbuf[%d] %f != %f\n", gProtocolName, dt_array_str[dt], count, i, rcvbuf[1], (float)j);
          err = -1;
          return err;
        }
      }
    }
  }
  else
  {
    unsigned char *cbuf = (unsigned char *)  rbuf;

    for (j=0; j<ntasks; j++)
    {
      unsigned char c = 0xFF & j;

      for (i=j*count; i<(j+1)*count; i++)
      if (cbuf[i] != c)
      {
        fprintf(stderr, "%s:Check(%d) failed <%p>rbuf[%d]=%.2u != %.2u \n", gProtocolName, count, cbuf, i, cbuf[i], c);
        return -1;
      }
    }
  }

  return err;
}

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_task_t          task_id, task_zero=0;
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

  /* Allgather variables */
  size_t               allgather_num_algorithm[2];
  pami_algorithm_t    *allgather_always_works_algo = NULL;
  pami_metadata_t     *allgather_always_works_md = NULL;
  pami_algorithm_t    *allgather_must_query_algo = NULL;
  pami_metadata_t     *allgather_must_query_md = NULL;
  pami_xfer_type_t     allgather_xfer = PAMI_XFER_ALLGATHER;
  volatile unsigned    allgather_poll_flag = 0;

  int                  nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          allgather;

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

  /*  Allocate buffer(s) */
  int err = 0;
  void* buf = NULL;
  err = posix_memalign(&buf, 128, gMax_count + gBuffer_offset);
  assert(err == 0);
  buf = (char*)buf + gBuffer_offset;

  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, (gMax_count * num_tasks) + gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;


  unsigned iContext = 0;

  for (; iContext < gNum_contexts; ++iContext)
  {

    if (task_id == task_zero)
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

    /*  Query the world geometry for allgather algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               allgather_xfer,
                               allgather_num_algorithm,
                               &allgather_always_works_algo,
                               &allgather_always_works_md,
                               &allgather_must_query_algo,
                               &allgather_must_query_md);

    if (rc == 1)
      return 1;

    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];
    blocking_coll(context[iContext], &barrier, &bar_poll_flag);

    for (nalg = 0; nalg < allgather_num_algorithm[0]; nalg++)
    {
      allgather.cb_done    = cb_done;
      allgather.cookie     = (void*) & allgather_poll_flag;
      allgather.algorithm  = allgather_always_works_algo[nalg];
      allgather.cmd.xfer_allgather.sndbuf     = buf;
      allgather.cmd.xfer_allgather.stype      = PAMI_TYPE_BYTE;
      allgather.cmd.xfer_allgather.stypecount = 0;
      allgather.cmd.xfer_allgather.rcvbuf     = rbuf;
      allgather.cmd.xfer_allgather.rtype      = PAMI_TYPE_BYTE;
      allgather.cmd.xfer_allgather.rtypecount = 0;

      gProtocolName = allgather_always_works_md[nalg].name;

      if (task_id == task_zero)
      {
        printf("# Allgather Bandwidth Test(size:%zu) -- context = %d, protocol: %s\n",num_tasks,
               iContext, gProtocolName);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(allgather_always_works_md[nalg].name,gSelected) == NULL) && gSelector) ||
          ((strstr(allgather_always_works_md[nalg].name,gSelected) != NULL) && !gSelector))  continue;

      int i, j;

      int dt,op=4/*SUM*/;

      for (dt = 0; dt < dt_count; dt++)
      {
          if (gValidTable[op][dt])
          {
            if (task_id == task_zero)
              printf("Running Allgather: %s\n", dt_array_str[dt]);

              for (i = gMin_count; i <= gMax_count/get_type_size(dt_array[dt]); i *= 2)
              {
                size_t  dataSent = i;
                int          niter;

                if (dataSent < CUTOFF)
                  niter = gNiterlat;
                else
                  niter = NITERBW;

                allgather.cmd.xfer_allgather.stype      = dt_array[dt];
                allgather.cmd.xfer_allgather.rtype      = dt_array[dt];
                allgather.cmd.xfer_allgather.stypecount = i;
                allgather.cmd.xfer_allgather.rtypecount = i;

                initialize_sndbuf (buf, i, task_id, dt);
                memset(rbuf, 0xFF, i);

                blocking_coll(context[iContext], &barrier, &bar_poll_flag);
                ti = timer();

                for (j = 0; j < niter; j++)
                {
                  blocking_coll (context[iContext], &allgather, &allgather_poll_flag);
                }

                tf = timer();
                blocking_coll(context[iContext], &barrier, &bar_poll_flag);

                int rc_check;
                rc_check = check_rcvbuf (rbuf, i, num_tasks, dt);

                if (rc_check) fprintf(stderr, "%s FAILED validation\n", gProtocolName);

                usec = (tf - ti) / (double)niter;

                if (task_id == task_zero)
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
    free(bar_always_works_algo);
    free(bar_always_works_md);
    free(bar_must_query_algo);
    free(bar_must_query_md);
    free(allgather_always_works_algo);
    free(allgather_always_works_md);
    free(allgather_must_query_algo);
    free(allgather_must_query_md);
  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  buf = (char*)buf - gBuffer_offset;
  free(buf);

  rbuf = (char*)rbuf - gBuffer_offset;
  free(rbuf);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}
