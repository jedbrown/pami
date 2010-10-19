///
/// \file test/api/collectives/gatherv.c
/// \brief Simple gatherv test
///

#include "../pami_util.h"

#define CHECK_DATA
#define BUFSIZE 524288
#define NITER   100

void initialize_sndbuf (pami_task_t task_id, void *buf, int bytes )
{

  unsigned char *cbuf = (unsigned char *)  buf;
  unsigned char c = 0x00 + task_id;
  int i = bytes;

    for (; i; i--)
    {
      cbuf[i-1] = c++;
    }
}

int check_rcvbuf (size_t num_tasks, void *buf, int bytes)
{


  for (int j = 0; j < num_tasks-1; j++)
  {
  unsigned char c = 0x00 + j;
  unsigned char *cbuf = (unsigned char *)  buf + j * bytes;
  int i = bytes;
  for (; i; i--)
    {
      if (cbuf[i-1] != c)
        {
          fprintf(stderr, "Check(%d) failed <%p> rank=%.2u, buf[%d]=%.2u != %.2u \n",bytes,buf, i, i-1, cbuf[i-
1], c);
          return -1;
        }

      c++;
    }
  }
  return 0;

}


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

  /* Gatherv variables */
  size_t               gatherv_num_algorithm[2];
  pami_algorithm_t    *gatherv_always_works_algo;
  pami_metadata_t     *gatherv_always_works_md;
  pami_algorithm_t    *gatherv_must_query_algo;
  pami_metadata_t     *gatherv_must_query_md;
  pami_xfer_type_t     gatherv_xfer = PAMI_XFER_GATHERV;
  volatile unsigned    gatherv_poll_flag = 0;

  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          gatherv;

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

  /*  Query the world geometry for gatherv algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            gatherv_xfer,
                            gatherv_num_algorithm,
                            &gatherv_always_works_algo,
                            &gatherv_always_works_md,
                            &gatherv_must_query_algo,
                            &gatherv_must_query_md);

  if (rc == 1)
    return 1;


  char   *buf       = (char*)malloc(BUFSIZE * num_tasks);
  char   *rbuf      = (char*)malloc(BUFSIZE * num_tasks);
  size_t *lengths   = (size_t*)malloc(num_tasks * sizeof(size_t));
  size_t *displs    = (size_t*)malloc(num_tasks * sizeof(size_t));
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];
  blocking_coll(context, &barrier, &bar_poll_flag);

  {
    int nalg = 0;

    for (nalg = 0; nalg < gatherv_num_algorithm[0]; nalg++)
      {
        size_t root = 0;

        if (task_id == root)
          {
            printf("# Gatherv Bandwidth Test -- protocol: %s\n", gatherv_always_works_md[nalg].name);
            printf("# Size(bytes)           cycles    bytes/sec    usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }

        gatherv.cb_done                       = cb_done;
        gatherv.cookie                        = (void*) & gatherv_poll_flag;
        gatherv.algorithm                     = gatherv_always_works_algo[nalg];
        gatherv.cmd.xfer_gatherv.root        = root;
        gatherv.cmd.xfer_gatherv.sndbuf      = buf;
        gatherv.cmd.xfer_gatherv.stype       = PAMI_BYTE;
        gatherv.cmd.xfer_gatherv.stypecount  = 0;
        gatherv.cmd.xfer_gatherv.rcvbuf      = rbuf;
        gatherv.cmd.xfer_gatherv.rtype       = PAMI_BYTE;
        gatherv.cmd.xfer_gatherv.rtypecounts = lengths;
        gatherv.cmd.xfer_gatherv.rdispls     = displs;

        size_t i, j;

        for (i = 1; i <= BUFSIZE; i *= 2)
          {
            long long dataSent = i;
            size_t       niter = NITER;
            size_t           k = 0;

            for (k = 0; k < num_tasks; k++)
              {
                lengths[k] = i;
                displs[k]  = k * i;
              }
            lengths[k-1] = 0;

            blocking_coll(context, &barrier, &bar_poll_flag);
            ti = timer();

            for (j = 0; j < niter; j++)
              {
                root = (root + num_tasks - 1) % num_tasks;
#ifdef CHECK_DATA
                initialize_sndbuf(num_tasks, buf, i);
                if (task_id == root) 
                  memset(rbuf, 0xFF, i*num_tasks);                              
#endif
                gatherv.cmd.xfer_gatherv.root        = root;
                if (task_id != num_tasks - 1) 
                  gatherv.cmd.xfer_gatherv.stypecount  = i;
                blocking_coll(context, &gatherv, &gatherv_poll_flag);
#ifdef CHECK_DATA
                if (task_id == root)
                  check_rcvbuf(num_tasks, rbuf, i);
#endif
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
  free(gatherv_always_works_algo);
  free(gatherv_always_works_md);
  free(gatherv_must_query_algo);
  free(gatherv_must_query_md);

  return 0;
};
