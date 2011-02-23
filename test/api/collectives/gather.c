/**
   \file test/api/collectives/gather.c
   \brief Simple gather test 
*/

#include "../pami_util.h"
#define CHECK_DATA
#define NITER   10
#define BUFSIZE 524288

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

  for (int j = 0; j < num_tasks; j++) 
  { 
  unsigned char *cbuf = (unsigned char *)  buf + j *bytes;
  unsigned char c = 0x00 + j;
  int i = bytes;
  for (; i; i--)
    {
      if (cbuf[i-1] != c)
        {
          fprintf(stderr, "Check(%d) failed <%p> rank=%.2u, buf[%d]=%.2u != %.2u \n",bytes,buf, i, i-1, cbuf[i-1], c);
          assert(0);
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
  pami_xfer_t          barrier;
  volatile unsigned    bar_poll_flag = 0;

  /* Gather variables */
  size_t               gather_num_algorithm[2];
  pami_algorithm_t    *gather_always_works_algo = NULL;
  pami_metadata_t     *gather_always_works_md = NULL;
  pami_algorithm_t    *gather_must_query_algo = NULL;
  pami_metadata_t     *gather_must_query_md = NULL;
  pami_xfer_type_t     gather_xfer = PAMI_XFER_GATHER;
  pami_xfer_t          gather;
  volatile unsigned    gather_poll_flag = 0;

  double               ti, tf, usec;

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

  /*  Query the world geometry for gather algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            gather_xfer,
                            gather_num_algorithm,
                            &gather_always_works_algo,
                            &gather_always_works_md,
                            &gather_must_query_algo,
                            &gather_must_query_md);

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

    for (nalg = 0; nalg < gather_num_algorithm[0]; nalg++)
      {
        size_t root = 0;

        if (task_id == root)
          {
            printf("# Gather Bandwidth Test -- protocol: %s\n", gather_always_works_md[nalg].name);
            printf("#  Bandwidth Test -- \n");
            printf("# Size(bytes)           cycles    bytes/sec    usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }
        if(((strstr(gather_always_works_md[nalg].name,selected) == NULL) && selector) ||
           ((strstr(gather_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

        gather.cb_done    = cb_done;
        gather.cookie     = (void*) & gather_poll_flag;
        gather.algorithm  = gather_always_works_algo[nalg];
        gather.cmd.xfer_gather.root       = root;
        gather.cmd.xfer_gather.sndbuf     = buf;
        gather.cmd.xfer_gather.stype      = PAMI_TYPE_CONTIGUOUS;
        gather.cmd.xfer_gather.stypecount = 0;
        gather.cmd.xfer_gather.rcvbuf     = rbuf;
        gather.cmd.xfer_gather.rtype      = PAMI_TYPE_CONTIGUOUS;
        gather.cmd.xfer_gather.rtypecount = 0;

        int i, j;

        for (i = 1; i <= BUFSIZE; i *= 2)
          {
            long long dataSent = i;
            int          niter = NITER; 
            blocking_coll(context, &barrier, &bar_poll_flag);
            ti = timer();

            for (j = 0; j < niter; j++)
              {
                root = (root + num_tasks - 1) % num_tasks; 
#ifdef CHECK_DATA
                initialize_sndbuf (task_id, buf, i);
                if (task_id == root) 
                 memset(rbuf, 0xFF, i*num_tasks);
#endif
                gather.cmd.xfer_gather.stypecount = i;
                gather.cmd.xfer_gather.rtypecount = i;
                gather.cmd.xfer_gather.root       = root;
                blocking_coll(context, &gather, &gather_poll_flag);
#ifdef CHECK_DATA
                if (task_id == root) 
                 check_rcvbuf(task_id, rbuf, i);
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
  free(gather_always_works_algo);
  free(gather_always_works_md);
  free(gather_must_query_algo);
  free(gather_must_query_md);
  return 0;
};
