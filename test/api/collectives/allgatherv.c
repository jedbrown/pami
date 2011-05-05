/**
 * \file test/api/collectives/allgatherv.c
 */

#include "../pami_util.h"

#define BUFSIZE 524288
#define CHECK_DATA

void initialize_sndbuf (void *sbuf, int bytes, pami_task_t task)
{
  unsigned char c = 0xFF & task;
  memset(sbuf,c,bytes);
}

int check_rcvbuf (void *rbuf, int bytes, size_t ntasks)
{
  int i,j;
  unsigned char *cbuf = (unsigned char *)  rbuf;
  for (j=0; j<ntasks; j++)
  {
    unsigned char c = 0xFF & j;
    for (i=j*bytes; i<(j+1)*bytes; i++)
      if (cbuf[i] != c)
      {
        fprintf(stderr, "Check(%d) failed <%p>rbuf[%d]=%.2u != %.2u \n", bytes, cbuf,i,cbuf[i], c);
        return 1;
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

  /*  Query the world geometry for allgatherv algorithms */
  rc = query_geometry_world(client,
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

    for (nalg = 0; nalg < allgatherv_num_algorithm[0]; nalg++)
      {
        if (task_id == 0)
          {
            printf("# Allgatherv Bandwidth Test -- protocol: %s\n", allgatherv_always_works_md[nalg].name);
            printf("# Size(bytes)           cycles    bytes/sec    usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }
        if(((strstr(allgatherv_always_works_md[nalg].name,selected) == NULL) && selector) ||
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

        for (i = 1; i <= BUFSIZE; i *= 2)
          {
            long long dataSent = i;
            unsigned  niter    = 100;

            for (k = 0; k < num_tasks; k++)lengths[k] = i;
            for (k = 0; k < num_tasks; k++)displs[k]  = k*i;

            allgatherv.cmd.xfer_allgatherv.stypecount       = i;
#ifdef CHECK_DATA
            initialize_sndbuf (buf, i, task_id);
            memset(rbuf, 0xFF, i);
#endif

            blocking_coll(context, &barrier, &bar_poll_flag);
            ti = timer();
            for (j = 0; j < niter; j++)
              {
                blocking_coll(context, &allgatherv, &allgatherv_poll_flag);
              }

            tf = timer();
            blocking_coll(context, &barrier, &bar_poll_flag);
#ifdef CHECK_DATA
            rc |= check_rcvbuf (rbuf, i, num_tasks);
#endif
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
  free(allgatherv_always_works_algo);
  free(allgatherv_always_works_md);
  free(allgatherv_must_query_algo);
  free(allgatherv_must_query_md);

  return 0;
};
