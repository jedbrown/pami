/**
   \file test/api/collectives/gatherv_int.c
   \brief Simple gatherv_int test 
*/

#define BUFSIZE 524288
#define NITER   100

#include "../pami_util.h"

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

  int j;
  for (j = 0; j < num_tasks-1; j++)
  {
    unsigned char c = 0x00 + j;
    unsigned char *cbuf = (unsigned char *)  buf + j * bytes;
    int i = bytes;
    for (; i; i--)
    {
      if (cbuf[i-1] != c)
      {
        fprintf(stderr, "Check(%d) failed <%p> rank=%.2u, buf[%d]=%.2u != %.2u \n",
                bytes,buf, i, i-1, cbuf[i-1], c);
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
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag = 0;

  /* Gatherv variables */
  size_t               gatherv_num_algorithm[2];
  pami_algorithm_t    *gatherv_always_works_algo = NULL;
  pami_metadata_t     *gatherv_always_works_md = NULL;
  pami_algorithm_t    *gatherv_must_query_algo = NULL;
  pami_metadata_t     *gatherv_must_query_md = NULL;
  pami_xfer_type_t     gatherv_xfer = PAMI_XFER_GATHERV_INT;
  volatile unsigned    gatherv_poll_flag = 0;

  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          gatherv;

  /* \note Test environment variable" TEST_VERBOSE=N     */
  char* sVerbose = getenv("TEST_VERBOSE");

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

  /*  Query the world geometry for gatherv algorithms */
  rc |= query_geometry_world(client,
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
  int *lengths   = (int*)malloc(num_tasks * sizeof(int));
  int *displs    = (int*)malloc(num_tasks * sizeof(int));
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];
  blocking_coll(context, &barrier, &bar_poll_flag);

  {
    int nalg = 0;

    for (nalg = 0; nalg < gatherv_num_algorithm[0]; nalg++)
    {
      pami_task_t root_zero = 0;

      if (task_id == root_zero)
      {
        printf("# Gatherv_int Bandwidth Test -- protocol: %s\n", gatherv_always_works_md[nalg].name);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }
      if (((strstr(gatherv_always_works_md[nalg].name,selected) == NULL) && selector) ||
          ((strstr(gatherv_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

      gatherv.cb_done                       = cb_done;
      gatherv.cookie                        = (void*) & gatherv_poll_flag;
      gatherv.algorithm                     = gatherv_always_works_algo[nalg];

      gatherv.cmd.xfer_gatherv_int.sndbuf      = buf;
      gatherv.cmd.xfer_gatherv_int.stype       = PAMI_TYPE_BYTE;
      gatherv.cmd.xfer_gatherv_int.stypecount  = 0;
      gatherv.cmd.xfer_gatherv_int.rcvbuf      = rbuf;
      gatherv.cmd.xfer_gatherv_int.rtype       = PAMI_TYPE_BYTE;
      gatherv.cmd.xfer_gatherv_int.rtypecounts = lengths;
      gatherv.cmd.xfer_gatherv_int.rdispls     = displs;

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
          root_zero = (root_zero + num_tasks - 1) % num_tasks;
          pami_endpoint_t root_ep;
          PAMI_Endpoint_create(client, root_zero, 0, &root_ep);
          gatherv.cmd.xfer_gatherv_int.root        = root_ep;

          initialize_sndbuf(task_id, buf, i);
          if (task_id == root_zero)
            memset(rbuf, 0xFF, i*num_tasks);

          if (task_id != num_tasks - 1)
            gatherv.cmd.xfer_gatherv_int.stypecount  = i;
          blocking_coll(context, &gatherv, &gatherv_poll_flag);

          if (task_id == root_zero)
            check_rcvbuf(num_tasks, rbuf, i);

        }

        tf = timer();
        blocking_coll(context, &barrier, &bar_poll_flag);

        usec = (tf - ti) / (double)niter;

        if (task_id == root_zero)
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
  rc |= pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(gatherv_always_works_algo);
  free(gatherv_always_works_md);
  free(gatherv_must_query_algo);
  free(gatherv_must_query_md);

  return rc;
};
