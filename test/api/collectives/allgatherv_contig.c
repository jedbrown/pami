/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/allgatherv.c
 */

#define BUFSIZE 524288

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
        if (rcvbuf[i] !=  j)
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

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context;
  size_t               num_contexts = 1;
  pami_task_t          task_id, task_zero=0;;
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

  /* Allgatherv variables */
  size_t               allgatherv_num_algorithm[2];
  pami_algorithm_t    *allgatherv_always_works_algo = NULL;
  pami_metadata_t     *allgatherv_always_works_md = NULL;
  pami_algorithm_t    *allgatherv_must_query_algo = NULL;
  pami_metadata_t     *allgatherv_must_query_md = NULL;
  pami_xfer_type_t     allgatherv_xfer = PAMI_XFER_ALLGATHERV;

  volatile unsigned    allgatherv_poll_flag = 0;

  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          allgatherv;

  /* \note Test environment variable" TEST_VERBOSE=N     */
  char* sVerbose = getenv("TEST_VERBOSE");
  setup_env();
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

  /*  Query the world geometry for barrier algorithms */
  rc |= query_geometry_world(client,
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

  /*  Query the world geometry for allgatherv algorithms */
  rc |= query_geometry_world(client,
                            context,
                            &world_geometry,
                            allgatherv_xfer,
                            allgatherv_num_algorithm,
                            &allgatherv_always_works_algo,
                            &allgatherv_always_works_md,
                            &allgatherv_must_query_algo,
                            &allgatherv_must_query_md);

  if (rc == 1)
    return 1;


  /*  Allocate buffer(s) */
  int err = 0;
  void* buf = NULL;
  err = posix_memalign(&buf, 128, (MAXBUFSIZE * num_tasks) + gBuffer_offset);
  assert(err == 0);
  buf = (char*)buf + gBuffer_offset;

  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, (MAXBUFSIZE * num_tasks) + gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;

  size_t *lengths   = (size_t*)malloc(num_tasks * sizeof(size_t));
  size_t *displs    = (size_t*)malloc(num_tasks * sizeof(size_t));
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];
  blocking_coll(context, &barrier, &bar_poll_flag);

  {
    int nalg = 0;

    for (nalg = 0; nalg < allgatherv_num_algorithm[0]; nalg++)
    {
      if (task_id == 0)
      {
        printf("# Allgatherv Bandwidth Test -- protocol: %s\n", allgatherv_always_works_md[nalg].name);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }
      if (((strstr(allgatherv_always_works_md[nalg].name,selected) == NULL) && selector) ||
          ((strstr(allgatherv_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

      allgatherv.cb_done    = cb_done;
      allgatherv.cookie     = (void*) & allgatherv_poll_flag;
      allgatherv.algorithm  = allgatherv_always_works_algo[nalg];
      allgatherv.cmd.xfer_allgatherv.sndbuf     = buf;
      allgatherv.cmd.xfer_allgatherv.stype      = PAMI_TYPE_BYTE;
      allgatherv.cmd.xfer_allgatherv.stypecount = 0;
      allgatherv.cmd.xfer_allgatherv.rcvbuf     = rbuf;
      allgatherv.cmd.xfer_allgatherv.rtype      = PAMI_TYPE_BYTE;
      allgatherv.cmd.xfer_allgatherv.rtypecounts = lengths;
      allgatherv.cmd.xfer_allgatherv.rdispls     = displs;

      unsigned i, j, k;

      int dt,op=4/*SUM*/;

      for (dt = 0; dt < dt_count; dt++)
      {
          if (gValidTable[op][dt])
          {
            if (task_id == task_zero)
              printf("Running Allgatherv: %s\n", dt_array_str[dt]);

            for (i = 1; i <= BUFSIZE/get_type_size(dt_array[dt]); i *= 2)
            {
              long long dataSent = i;
              unsigned  niter    = 100;

              for (k = 0; k < num_tasks; k++)lengths[k] = i;
              for (k = 0; k < num_tasks; k++)displs[k]  = k*i*get_type_size(dt_array[dt]);

              allgatherv.cmd.xfer_allgatherv.stypecount       = i;
              allgatherv.cmd.xfer_allgatherv.stype            = dt_array[dt];
              allgatherv.cmd.xfer_allgatherv.rtype            = dt_array[dt];

              initialize_sndbuf (buf, i, task_id, dt);
              memset(rbuf, 0xFF, i);


              blocking_coll(context, &barrier, &bar_poll_flag);
              ti = timer();
              for (j = 0; j < niter; j++)
              {
                blocking_coll(context, &allgatherv, &allgatherv_poll_flag);
              }

              tf = timer();
              blocking_coll(context, &barrier, &bar_poll_flag);

              int rc_check;
              rc |= rc_check = check_rcvbuf (rbuf, i, num_tasks, dt);

              if (rc_check) fprintf(stderr, "%s FAILED validation\n", gProtocolName);

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
      }
    }
  }
  rc |= pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(allgatherv_always_works_algo);
  free(allgatherv_always_works_md);
  free(allgatherv_must_query_algo);
  free(allgatherv_must_query_md);

  return rc;
};
