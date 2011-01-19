/**
 * \file test/api/collectives/allgather.c
 * \brief Simple Barrier test
 */

#include "../pami_util.h"

#define BUFSIZE 524288

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context;
  size_t               num_contexts = 1;
  pami_task_t          task_id;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo;
  pami_metadata_t     *bar_always_works_md;
  pami_algorithm_t    *bar_must_query_algo;
  pami_metadata_t     *bar_must_query_md;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag = 0;

  /* Allgather variables */
  size_t               allgather_num_algorithm[2];
  pami_algorithm_t    *allgather_always_works_algo;
  pami_metadata_t     *allgather_always_works_md;
  pami_algorithm_t    *allgather_must_query_algo;
  pami_metadata_t     *allgather_must_query_md;
  pami_xfer_type_t     allgather_xfer = PAMI_XFER_ALLGATHER;
  volatile unsigned    allgather_poll_flag = 0;

  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          allgather;

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

  /*  Query the world geometry for allgather algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            allgather_xfer,
                            allgather_num_algorithm,
                            &allgather_always_works_algo,
                            &allgather_always_works_md,
                            &allgather_must_query_algo,
                            &allgather_must_query_md);

  if (rc == 1)
    return 1;

  char *buf         = (char*)malloc(BUFSIZE * num_tasks);
  char *rbuf        = (char*)malloc(BUFSIZE * num_tasks);
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];
  blocking_coll(context, &barrier, &bar_poll_flag);

  {
    int nalg = 0;

    for (nalg = 0; nalg < allgather_num_algorithm[0]; nalg++)
      {
        if (task_id == 0)
          {
            printf("# Allgather Bandwidth Test -- protocol: %s\n", allgather_always_works_md[nalg].name);
            printf("# Size(bytes)           cycles    bytes/sec    usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }
        if(((strstr(allgather_always_works_md[nalg].name,selected) == NULL) && selector) ||
           ((strstr(allgather_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

        allgather.cb_done    = cb_done;
        allgather.cookie     = (void*) & allgather_poll_flag;
        allgather.algorithm  = allgather_always_works_algo[nalg];
        allgather.cmd.xfer_allgather.sndbuf     = buf;
        allgather.cmd.xfer_allgather.stype      = PAMI_TYPE_CONTIGUOUS;
        allgather.cmd.xfer_allgather.stypecount = 0;
        allgather.cmd.xfer_allgather.rcvbuf     = rbuf;
        allgather.cmd.xfer_allgather.rtype      = PAMI_TYPE_CONTIGUOUS;
        allgather.cmd.xfer_allgather.rtypecount = 0;

        int i, j;

        for (i = 1; i <= BUFSIZE; i *= 2)
          {
            long long dataSent = i;
            int          niter = 100;
            blocking_coll(context, &barrier, &bar_poll_flag);
            ti = timer();

            for (j = 0; j < niter; j++)
              {
                allgather.cmd.xfer_allgather.stypecount = i;
                allgather.cmd.xfer_allgather.rtypecount = i;
                blocking_coll (context, &allgather, &allgather_poll_flag);
              }

            tf = timer();
            blocking_coll(context, &barrier, &bar_poll_flag);

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
  rc = pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(allgather_always_works_algo);
  free(allgather_always_works_md);
  free(allgather_must_query_algo);
  free(allgather_must_query_md);

  return 0;
};
