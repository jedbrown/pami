/**
 * \file test/api/collectives/scatter.c
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
  pami_xfer_t          barrier;
  volatile unsigned    bar_poll_flag = 0;

  /* Scatter variables */
  size_t               scatter_num_algorithm[2];
  pami_algorithm_t    *scatter_always_works_algo;
  pami_metadata_t     *scatter_always_works_md;
  pami_algorithm_t    *scatter_must_query_algo;
  pami_metadata_t     *scatter_must_query_md;
  pami_xfer_type_t     scatter_xfer = PAMI_XFER_SCATTER;
  pami_xfer_t          scatter;
  volatile unsigned    scatter_poll_flag = 0;

  double               ti, tf, usec;

  char* selected = getenv("TEST_PROTOCOL");
  if(!selected) selected = "";

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

  /*  Query the world geometry for scatter algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            scatter_xfer,
                            scatter_num_algorithm,
                            &scatter_always_works_algo,
                            &scatter_always_works_md,
                            &scatter_must_query_algo,
                            &scatter_must_query_md);

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

    for (nalg = 0; nalg < scatter_num_algorithm[0]; nalg++)
      {
        size_t root = 0;

        if (task_id == root)
          {
            printf("# Scatter Bandwidth Test -- protocol: %s\n", scatter_always_works_md[nalg].name);
            printf("#  Bandwidth Test -- \n");
            printf("# Size(bytes)           cycles    bytes/sec    usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }
        if(strncmp(scatter_always_works_md[nalg].name,selected, strlen(selected))) continue;

        scatter.cb_done    = cb_done;
        scatter.cookie     = (void*) & scatter_poll_flag;
        scatter.algorithm  = scatter_always_works_algo[nalg];
        scatter.cmd.xfer_scatter.root       = root;
        scatter.cmd.xfer_scatter.sndbuf     = buf;
        scatter.cmd.xfer_scatter.stype      = PAMI_BYTE;
        scatter.cmd.xfer_scatter.stypecount = 0;
        scatter.cmd.xfer_scatter.rcvbuf     = rbuf;
        scatter.cmd.xfer_scatter.rtype      = PAMI_BYTE;
        scatter.cmd.xfer_scatter.rtypecount = 0;

        int i, j;

        for (i = 1; i <= BUFSIZE; i *= 2)
          {
            long long dataSent = i;
            int          niter = 100;
            blocking_coll(context, &barrier, &bar_poll_flag);
            ti = timer();

            for (j = 0; j < niter; j++)
              {
                scatter.cmd.xfer_scatter.stypecount = i;
                scatter.cmd.xfer_scatter.rtypecount = i;
                blocking_coll(context, &scatter, &scatter_poll_flag);
              }

            tf = timer();
            blocking_coll(context, &barrier, &bar_poll_flag);

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
  }
  rc = pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(scatter_always_works_algo);
  free(scatter_always_works_md);
  free(scatter_must_query_algo);
  free(scatter_must_query_md);
  return 0;
};
