/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/ambcast.c
 * \brief Simple AMBcast test on world geometry
 */

#define COUNT      262144
#define NITERLAT   100
/*
#define OFFSET     0
#define NITERBW    MIN(10, niterlat/100+1)
#define CUTOFF     65536
*/

#include "../pami_util.h"

int g_ambcast_dt;

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

volatile unsigned       _g_total_broadcasts;
char                   *_g_recv_buffer;
int                     _gRc = PAMI_SUCCESS;
typedef struct
{
  void *rbuf;
  int count;
  int root;
  int dt;
  char buffer;
} validation_t;

void cb_ambcast_done (void *context, void * clientdata, pami_result_t err)
{
  _g_total_broadcasts++;
  int rc_check;
  validation_t *v = (validation_t*)clientdata;

  _gRc |= rc_check = check_rcvbuf (v->rbuf, v->count, v->root, v->dt);
  if (rc_check) fprintf(stderr, "%s FAILED validation\n", gProtocolName);

  free(clientdata);
}

void cb_ambcast_recv  (pami_context_t         context,      /**< IN:  communication context which invoked the dispatch function */
                     void                 * cookie,       /**< IN:  dispatch cookie */
                     const void           * header_addr,  /**< IN:  header address  */
                     size_t                 header_size,  /**< IN:  header size     */
                     const void           * pipe_addr,    /**< IN:  address of PAMI pipe  buffer, valid only if non-NULL        */
                     size_t                 data_size,    /**< IN:  number of bytes of message data */
                     pami_endpoint_t        origin,       /**< IN:  root initiating endpoint */
                     pami_geometry_t        geometry,     /**< IN:  Geometry */
                     pami_recv_t          * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
{
  if (gVerbose && !context)
    fprintf(stderr, "Error. Null context received on cb_done.\n");


  validation_t *v =  malloc(data_size+sizeof(validation_t));

  void* rcvbuf = v->rbuf = &v->buffer;
  v->count = data_size/get_type_size(dt_array[g_ambcast_dt]);
  pami_task_t     task;
  size_t          offset;
  _gRc |= PAMI_Endpoint_query (origin,
                              &task,
                              &offset);

  v->root = task;
  v->dt   = g_ambcast_dt;

  if (!recv)
  {
    memcpy(rcvbuf, pipe_addr, data_size);
    return;
  }

  recv->cookie      = (void*)v;
  recv->local_fn    = cb_ambcast_done;
  recv->addr        = rcvbuf;
  recv->type        = PAMI_TYPE_BYTE;
  recv->offset      = 0;
  recv->data_fn     = PAMI_DATA_COPY;
  recv->data_cookie = NULL;
}

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_result_t        result = PAMI_ERROR;
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

  /* Ambcast variables */
  size_t               ambcast_num_algorithm[2];
  pami_algorithm_t    *ambcast_always_works_algo = NULL;
  pami_metadata_t     *ambcast_always_works_md = NULL;
  pami_algorithm_t    *ambcast_must_query_algo = NULL;
  pami_metadata_t     *ambcast_must_query_md = NULL;
  pami_xfer_type_t     ambcast_xfer = PAMI_XFER_AMBROADCAST;
  pami_xfer_t          ambroadcast;
  volatile unsigned    ambcast_poll_flag = 0;

  int                  nalg= 0;
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

  if (gNumRoots == -1) gNumRoots = num_tasks;

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

    /*  Query the world geometry for ambroadcast algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               ambcast_xfer,
                               ambcast_num_algorithm,
                               &ambcast_always_works_algo,
                               &ambcast_always_works_md,
                               &ambcast_must_query_algo,
                               &ambcast_must_query_md);

    if (rc == 1)
      return 1;

    _g_recv_buffer = buf;


    barrier.cb_done     = cb_done;
    barrier.cookie      = (void*) & bar_poll_flag;
    barrier.algorithm   = bar_always_works_algo[0];
    blocking_coll(context[iContext], &barrier, &bar_poll_flag);


    ambroadcast.cb_done   = cb_done;
    ambroadcast.cookie    = (void*) & ambcast_poll_flag;
    ambroadcast.algorithm = ambcast_always_works_algo[0];
    ambroadcast.cmd.xfer_ambroadcast.user_header  = NULL;
    ambroadcast.cmd.xfer_ambroadcast.headerlen    = 0;
    ambroadcast.cmd.xfer_ambroadcast.sndbuf       = buf;
    ambroadcast.cmd.xfer_ambroadcast.stype        = PAMI_TYPE_BYTE;
    ambroadcast.cmd.xfer_ambroadcast.stypecount   = 0;

    for (nalg = 0; nalg < ambcast_num_algorithm[0]; nalg++)
    {
      gProtocolName = ambcast_always_works_md[nalg].name;

      int k;
      for (k=0; k< gNumRoots; k++)
      {
        pami_task_t root_task = (pami_task_t)k;
        if (task_id == root_task)
        {
          printf("# Broadcast Bandwidth Test -- context = %d, root = %d, protocol: %s\n",
                 iContext, root_task, ambcast_always_works_md[nalg].name);
          printf("# Size(bytes)           cycles    bytes/sec    usec\n");
          printf("# -----------      -----------    -----------    ---------\n");
        }


        if (((strstr(ambcast_always_works_md[nalg].name,gSelected) == NULL) && gSelector) ||
            ((strstr(ambcast_always_works_md[nalg].name,gSelected) != NULL) && !gSelector))  continue;

        int i, j;
        pami_collective_hint_t h = {0};
        pami_dispatch_callback_function fn;
        fn.ambroadcast = cb_ambcast_recv;
        PAMI_AMCollective_dispatch_set(context[iContext],
                                       ambcast_always_works_algo[nalg],
                                       k,
                                       fn,
                                       NULL,
                                       h);
        ambroadcast.algorithm = ambcast_always_works_algo[nalg];
        memset(buf, 0xFF, gMax_count);
        blocking_coll(context[iContext], &barrier, &bar_poll_flag);

        int dt,op=4/*SUM*/;

        for (dt = 0; dt < dt_count; dt++)
        {
          if (gValidTable[op][dt])
          {
              if (task_id == 0)
                printf("Running Broadcast: %s\n", dt_array_str[dt]);
		      g_ambcast_dt = dt;
              for (i = 1; i <= gMax_count; i *= 2)
              {
                size_t  dataSent = i;
                int          niter;

                if (dataSent < CUTOFF)
                  niter = gNiterlat;
                else
                  niter = NITERBW;

                if (task_id == root_task)
                {
                  initialize_sndbuf (buf, i, root_task, dt);
                  ti = timer();

                  for (j = 0; j < niter; j++)
                  {
                    ambroadcast.cmd.xfer_ambroadcast.stypecount = i;
                    ambroadcast.cmd.xfer_ambroadcast.stype = dt_array[dt];
                    blocking_coll (context[iContext], &ambroadcast, &ambcast_poll_flag);
                  }

                  while (ambcast_poll_flag)
                    result = PAMI_Context_advance (context[iContext], 1);

                  blocking_coll(context[iContext], &barrier, &bar_poll_flag);
                  tf = timer();
                  usec = (tf - ti) / (double)niter;
                  printf("  %11lld %16lld %14.1f %12.2f\n",
                   (long long)dataSent,
                   0LL,
                   (double)1e6*(double)dataSent / (double)usec,
                   usec);
                  fflush(stdout);
                }
                else
                {
                  while (_g_total_broadcasts < niter)
                  result = PAMI_Context_advance (context[iContext], 1);

                  rc |= _gRc; /* validation return code done in cb_ambcast_done */

                  _g_total_broadcasts = 0;
                  blocking_coll(context[iContext], &barrier, &bar_poll_flag);
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
    free(ambcast_always_works_algo);
    free(ambcast_always_works_md);
    free(ambcast_must_query_algo);
  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  buf = (char*)buf - gBuffer_offset;
  free(buf);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}