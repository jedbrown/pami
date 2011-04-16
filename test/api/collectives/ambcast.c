/**
 * \file test/api/collectives/ambcast.c
 * \brief Simple AMBcast test
 */

#include "../pami_util.h"

#define BUFSIZE 262144
#define NITER   100

volatile unsigned       _g_total_broadcasts;
char                   *_g_recv_buffer;

void cb_ambcast_done (void *context, void * clientdata, pami_result_t err)
{
  _g_total_broadcasts++;
  free(clientdata);
}

void cb_bcast_recv  (pami_context_t         context,      /**< IN:  communication context which invoked the dispatch function */
                     void                 * cookie,       /**< IN:  dispatch cookie */
                     const void           * header_addr,  /**< IN:  header address  */
                     size_t                 header_size,  /**< IN:  header size     */
                     const void           * pipe_addr,    /**< IN:  address of PAMI pipe  buffer, valid only if non-NULL        */
                     size_t                 data_size,    /**< IN:  number of bytes of message data */
                     pami_endpoint_t        origin,       /**< IN:  root initiating endpoint */
                     pami_geometry_t        geometry,     /**< IN:  Geometry */
                     pami_recv_t          * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
{
  if(gVerbose && !context)
    fprintf(stderr, "Error. Null context received on cb_done.\n");

  void *rcvbuf =  malloc(data_size);

  if(!recv)
  {
    memcpy(rcvbuf, pipe_addr, data_size);
    return;
  }

  recv->cookie      = (void*)rcvbuf;
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
  pami_context_t       context;
  pami_result_t        result = PAMI_ERROR;
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

  /* Ambcast variables */
  size_t               ambcast_num_algorithm[2];
  pami_algorithm_t    *ambcast_always_works_algo = NULL;
  pami_metadata_t     *ambcast_always_works_md = NULL;
  pami_algorithm_t    *ambcast_must_query_algo = NULL;
  pami_metadata_t     *ambcast_must_query_md = NULL;
  pami_xfer_type_t     ambcast_xfer = PAMI_XFER_AMBROADCAST;
  pami_xfer_t          ambroadcast;
  volatile unsigned    ambcast_poll_flag = 0;

  int                  i, j, root = 0, nalg = 0;
  double               ti, tf, usec;
  char                 buf[BUFSIZE];
  char                 rbuf[BUFSIZE];

  /* \note Test environment variable" TEST_VERBOSE=N     */
  char* sVerbose = getenv("TEST_VERBOSE");

  if(sVerbose) gVerbose=atoi(sVerbose); /* set the global defined in coll_util.h */

  /* \note Test environment variable" TEST_PROTOCOL={-}substring.       */
  /* substring is used to select, or de-select (with -) test protocols */
  unsigned selector = 1;
  char* selected = getenv("TEST_PROTOCOL");
  if(!selected) selected = "";
  else if(selected[0]=='-')
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

  /*  Query the world geometry for ambroadcast algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            ambcast_xfer,
                            ambcast_num_algorithm,
                            &ambcast_always_works_algo,
                            &ambcast_always_works_md,
                            &ambcast_must_query_algo,
                            &ambcast_must_query_md);

  if (rc == 1)
    return 1;

  _g_recv_buffer = rbuf;


  barrier.cb_done     = cb_done;
  barrier.cookie      = (void*) & bar_poll_flag;
  barrier.algorithm   = bar_always_works_algo[0];
  blocking_coll(context, &barrier, &bar_poll_flag);


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
      if (task_id == root)
        {
          printf("# Broadcast Bandwidth Test -- root = %d, protocol: %s\n", root, ambcast_always_works_md[nalg].name);
          printf("# Size(bytes)           cycles    bytes/sec    usec\n");
          printf("# -----------      -----------    -----------    ---------\n");
        }
      if(((strstr(ambcast_always_works_md[nalg].name,selected) == NULL) && selector) ||
         ((strstr(ambcast_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

      pami_collective_hint_t h = {0};
      pami_dispatch_callback_function fn;
      fn.ambroadcast = cb_bcast_recv;
      PAMI_AMCollective_dispatch_set(context,
                                     ambcast_always_works_algo[nalg],
                                     0,
                                     fn,
                                     NULL,
                                     h);
      ambroadcast.algorithm = ambcast_always_works_algo[nalg];
      blocking_coll(context, &barrier, &bar_poll_flag);

      for (i = 1; i <= BUFSIZE; i *= 2)
        {
          long long dataSent = i;
          unsigned     niter = NITER;

          if (task_id == root)
            {
              ti = timer();

              for (j = 0; j < niter; j++)
                {
                  ambroadcast.cmd.xfer_ambroadcast.stypecount = i;
                  blocking_coll (context, &ambroadcast, &ambcast_poll_flag);
                }

              while (ambcast_poll_flag)
                result = PAMI_Context_advance (context, 1);

              blocking_coll(context, &barrier, &bar_poll_flag);
              tf = timer();
              usec = (tf - ti) / (double)niter;
              printf("  %11lld %16lld %14.1f %12.2f\n",
                     dataSent,
                     0LL,
                     (double)1e6*(double)dataSent / (double)usec,
                     usec);
              fflush(stdout);
            }
          else
            {
              while (_g_total_broadcasts < niter)
                result = PAMI_Context_advance (context, 1);

              _g_total_broadcasts = 0;
              blocking_coll(context, &barrier, &bar_poll_flag);
            }
        }
    }

  rc = pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(ambcast_always_works_algo);
  free(ambcast_always_works_md);
  free(ambcast_must_query_algo);
  free(ambcast_must_query_md);
  return 0;
}
