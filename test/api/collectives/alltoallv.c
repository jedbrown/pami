/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/alltoallv.c
 * \brief ???
 */
#include "../pami_util.h"

#define MAX_COMM_SIZE 128
#define MSGSIZE       4096
#define BUFSIZE       (MSGSIZE * MAX_COMM_SIZE)


/*#define INIT_BUFS(r) */
#define INIT_BUFS(r) init_bufs(r)

/*#define CHCK_BUFS */
#define CHCK_BUFS(s,r)    check_bufs(s,r)


char sbuf[BUFSIZE];
char rbuf[BUFSIZE];
size_t sndlens[ MAX_COMM_SIZE ];
size_t sdispls[ MAX_COMM_SIZE ];
size_t rcvlens[ MAX_COMM_SIZE ];
size_t rdispls[ MAX_COMM_SIZE ];

void init_bufs(size_t r)
{
  size_t k;

  for ( k = 0; k < sndlens[r]; k++ )
    {
      sbuf[ sdispls[r] + k ] = ((r + k) & 0xff);
      rbuf[ rdispls[r] + k ] = 0xff;
    }
}


void check_bufs(size_t sz, size_t myrank)
{
  size_t r, k;

  for ( r = 0; r < sz; r++ )
    for ( k = 0; k < rcvlens[r]; k++ )
      {
        if ( rbuf[ rdispls[r] + k ] != (char)((myrank + k) & 0xff) )
          {
            printf("%zu: (E) rbuf[%zu]:%02x instead of %02zx (r:%zu)\n",
                   myrank,
                   rdispls[r] + k,
                   rbuf[ rdispls[r] + k ],
                   ((r + k) & 0xff),
                   r );
            exit(1);
          }
      }
}


int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t       context;
  size_t               num_contexts = 1;
  pami_task_t          task_id;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  volatile unsigned    bar_poll_flag = 0;
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;

  /* Alltoallv variables */
  size_t               alltoallv_num_algorithm[2];
  pami_algorithm_t    *alltoallv_always_works_algo = NULL;
  pami_metadata_t     *alltoallv_always_works_md = NULL;
  pami_algorithm_t    *alltoallv_must_query_algo = NULL;
  pami_metadata_t     *alltoallv_must_query_md = NULL;
  pami_xfer_type_t     alltoallv_xfer = PAMI_XFER_ALLTOALLV;

  volatile unsigned    alltoallv_poll_flag = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          alltoallv;

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
  if(num_tasks > MAX_COMM_SIZE )
  {
    fprintf(stderr, "Number of tasks (%zu) > MAX_COMM_SIZE (%zu)\n",num_tasks, (size_t)MAX_COMM_SIZE);
    return 1;
  }

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

  /*  Query the world geometry for alltoallv algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            alltoallv_xfer,
                            alltoallv_num_algorithm,
                            &alltoallv_always_works_algo,
                            &alltoallv_always_works_md,
                            &alltoallv_must_query_algo,
                            &alltoallv_must_query_md);

  if (rc == 1)
    return 1;

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];

  alltoallv.cb_done    = cb_done;
  alltoallv.cookie     = (void*) & alltoallv_poll_flag;

  {
    int nalg = 0;

    for (nalg = 0; nalg < alltoallv_num_algorithm[0]; nalg++)
      {
        size_t i, j;

        if (task_id == 0)
          {
            printf("# Alltoallv Bandwidth Test(size:%zu) %p, protocol: %s\n", num_tasks, cb_done, alltoallv_always_works_md[nalg].name);
            printf("# Size(bytes)           cycles    bytes/sec      usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }
        if(((strstr(alltoallv_always_works_md[nalg].name,selected) == NULL) && selector) ||
           ((strstr(alltoallv_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

        alltoallv.algorithm  = alltoallv_always_works_algo[nalg];

        for (i = 1; i <= MSGSIZE; i *= 2)
          {
            long long dataSent = i;
            size_t niter = (i < 1024 ? 100 : 10);

            for ( j = 0; j < num_tasks; j++ )
              {
                sndlens[j] = rcvlens[j] = i;
                sdispls[j] = rdispls[j] = i * j;
                INIT_BUFS( j );
              }

            blocking_coll(context, &barrier, &bar_poll_flag);

            ti = timer();

            for (j = 0; j < niter; j++)
              {
                alltoallv.cmd.xfer_alltoallv.sndbuf        = sbuf;
                alltoallv.cmd.xfer_alltoallv.stype         = PAMI_TYPE_CONTIGUOUS;
                alltoallv.cmd.xfer_alltoallv.stypecounts   = sndlens;
                alltoallv.cmd.xfer_alltoallv.sdispls       = sdispls;
                alltoallv.cmd.xfer_alltoallv.rcvbuf        = rbuf;
                alltoallv.cmd.xfer_alltoallv.rtype         = PAMI_TYPE_CONTIGUOUS;
                alltoallv.cmd.xfer_alltoallv.rtypecounts   = rcvlens;
                alltoallv.cmd.xfer_alltoallv.rdispls       = rdispls;
                blocking_coll(context, &alltoallv, &alltoallv_poll_flag);
              }

            tf = timer();

            CHCK_BUFS(num_tasks, task_id);

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
  free(alltoallv_always_works_algo);
  free(alltoallv_always_works_md);
  free(alltoallv_must_query_algo);
  free(alltoallv_must_query_md);

  return 0;
}
