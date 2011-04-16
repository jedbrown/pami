/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/reduce.c
 * \brief simple reduce on world geometry
 */

#include "../pami_util.h"

/*define this if you want to validate the data */
#define CHECK_DATA
#define FULL_TEST
#define COUNT      65536
#define MAXBUFSIZE COUNT*16
#define NITERLAT   10
#define NITERBW    1
#define CUTOFF     65536


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
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag = 0;

  /* Reduce variables */
  size_t               reduce_num_algorithm[2];
  pami_algorithm_t    *reduce_always_works_algo = NULL;
  pami_metadata_t     *reduce_always_works_md = NULL;
  pami_algorithm_t    *reduce_must_query_algo = NULL;
  pami_metadata_t     *reduce_must_query_md = NULL;
  pami_xfer_type_t     reduce_xfer = PAMI_XFER_REDUCE;
  volatile unsigned    reduce_poll_flag = 0;

  int                  root = 0, i, j, nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          reduce;


  char sbuf[MAXBUFSIZE];
  char rbuf[MAXBUFSIZE];
  int op, dt;

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

  /*  Query the world geometry for reduce algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            reduce_xfer,
                            reduce_num_algorithm,
                            &reduce_always_works_algo,
                            &reduce_always_works_md,
                            &reduce_must_query_algo,
                            &reduce_must_query_md);

  if (rc == 1)
    return 1;

  size_t** validTable =
    alloc2DContig(op_count, dt_count);
#ifdef FULL_TEST

  for (i = 0; i < op_count; i++)
    for (j = 0; j < dt_count; j++)
      validTable[i][j] = 1;
  /*--------------------------------------*/
  /* Disable unsupported ops on complex   */
  /* Only sum, prod                       */
  for (i = 0, j = DT_SINGLE_COMPLEX; i < OP_COUNT; i++)if(i!=OP_SUM && i!=OP_PROD) validTable[i][j] = 0;
  for (i = 0, j = DT_DOUBLE_COMPLEX; i < OP_COUNT; i++)if(i!=OP_SUM && i!=OP_PROD) validTable[i][j] = 0; 

  /*--------------------------------------*/
  /* Disable non-LOC ops on LOC dt's      */
  for (i = 0, j = DT_LOC_2INT      ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_SHORT_INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_FLOAT_INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_DOUBLE_INT; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_2FLOAT    ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_2DOUBLE   ; i < OP_MAXLOC; i++)validTable[i][j] = 0;

  /*--------------------------------------*/
  /* Disable NULL and byte operations     */
  for (i = 0, j = DT_NULL; i < OP_COUNT; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_BYTE; i < OP_COUNT; i++)validTable[i][j] = 0;
  for (j = 0, i = OP_COPY; j < DT_COUNT; j++) validTable[i][j] = 0;
  for (j = 0, i = OP_NOOP; j < DT_COUNT; j++) validTable[i][j] = 0;


  /*--------------------------------------*/
  /* Disable LOC ops on non-LOC dt's      */
  for (j = 0, i = OP_MAXLOC; j < DT_LOC_2INT; j++) validTable[i][j] = 0;
  for (j = 0, i = OP_MINLOC; j < DT_LOC_2INT; j++) validTable[i][j] = 0;

  /*---------------------------------------*/
  /* Disable unsupported ops on logical dt */
  /* Only land, lor, lxor, band, bor, bxor */
  for (i = 0,         j = DT_LOGICAL; i < OP_LAND ; i++) validTable[i][j] = 0;
  for (i = OP_BXOR+1, j = DT_LOGICAL; i < OP_COUNT; i++) validTable[i][j] = 0;

  /*---------------------------------------*/
  /* Disable unsupported ops on long double*/
  /* Only max,min,sum,prod                 */
  for (i = OP_PROD+1, j = DT_LONG_DOUBLE; i < OP_COUNT; i++) validTable[i][j] = 0;

#else

  for (i = 0; i < op_count; i++)
    for (j = 0; j < dt_count; j++)
      validTable[i][j] = 0;

  validTable[OP_SUM][DT_UNSIGNED_INT] = 1;
  validTable[OP_SUM][DT_DOUBLE] = 1;

#endif

  for (nalg = 0; nalg < reduce_num_algorithm[0]; nalg++)
    {
      if (task_id == 0) /* root not set yet */
        {
          printf("# Reduce Bandwidth Test -- root varies, protocol: %s\n", 
                 reduce_always_works_md[nalg].name);
          printf("# Size(bytes)           cycles    bytes/sec    usec\n");
          printf("# -----------      -----------    -----------    ---------\n");
        }
      if(((strstr(reduce_always_works_md[nalg].name,selected) == NULL) && selector) ||
         ((strstr(reduce_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

      barrier.cb_done   = cb_done;
      barrier.cookie    = (void*) & bar_poll_flag;
      barrier.algorithm = bar_always_works_algo[0];
      blocking_coll(context, &barrier, &bar_poll_flag);

      reduce.cb_done   = cb_done;
      reduce.cookie    = (void*) & reduce_poll_flag;
      reduce.algorithm = reduce_always_works_algo[nalg];
      reduce.cmd.xfer_reduce.sndbuf    = sbuf;
      reduce.cmd.xfer_reduce.stype     = PAMI_TYPE_BYTE;
      reduce.cmd.xfer_reduce.stypecount = 0;
      reduce.cmd.xfer_reduce.rtype     = PAMI_TYPE_BYTE;
      reduce.cmd.xfer_reduce.rtypecount = 0;



      for (dt = 0; dt < dt_count; dt++)
        for (op = 0; op < op_count; op++)
          {
            if (validTable[op][dt])
              {
                if (task_id == 0) /* root not set yet */
                  printf("Running Reduce: %s, %s\n", dt_array_str[dt], op_array_str[op]);

                for (i = 1; i <= COUNT; i *= 2)
                  {
                    size_t sz=get_type_size(dt_array[dt]);
                    long long dataSent = i * sz;
                    int niter;

                    if (dataSent < CUTOFF)
                      niter = NITERLAT;
                    else
                      niter = NITERBW;

                    root = 0;


                    reduce.cmd.xfer_reduce.stypecount = i;
                    reduce.cmd.xfer_reduce.rtypecount = dataSent;
                    reduce.cmd.xfer_reduce.stype      = dt_array[dt];
                    reduce.cmd.xfer_reduce.op         = op_array[op];

#ifdef CHECK_DATA
                    reduce_initialize_sndbuf (sbuf, i, op, dt, task_id);
#endif
                    blocking_coll(context, &barrier, &bar_poll_flag);
                    ti = timer();

                    for (j = 0; j < niter; j++)
                    {
                      reduce.cmd.xfer_reduce.root    = root;

                      if (task_id == root)
                        reduce.cmd.xfer_reduce.rcvbuf    = rbuf;
                      else
                        reduce.cmd.xfer_reduce.rcvbuf    = NULL;
                      blocking_coll(context, &reduce, &reduce_poll_flag);

                      root = (root + 1) % num_tasks;
                    }

                    tf = timer();
                    blocking_coll(context, &barrier, &bar_poll_flag);

#ifdef CHECK_DATA

                    if (task_id < niter) /* was root at least once in niter loop */
                    {
                      int rc = reduce_check_rcvbuf (rbuf, i, op, dt, num_tasks);

                      if (rc) fprintf(stderr, "FAILED validation\n");
                    }

#endif

                    usec = (tf - ti) / (double)niter;

                    if (task_id == root)
                      {
                        printf("  %11lld %16d %14.1f %12.2f\n",
                               dataSent,
                               niter,
                               (double)1e6*(double)dataSent / (double)usec,
                               usec);
                        fflush(stdout);
                      }
                  }
              }
          }
    }

  rc = pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(reduce_always_works_algo);
  free(reduce_always_works_md);
  free(reduce_must_query_algo);
  free(reduce_must_query_md);

  return 0;
}
