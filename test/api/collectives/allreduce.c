/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/allreduce.c
 * \brief simple allreduce on world geometry
 */

#include "../pami_util.h"

/*define this if you want to validate the data */
#define CHECK_DATA
#define FULL_TEST
#define COUNT      65536
#define MAXBUFSIZE COUNT*16
#define NITERLAT   1000
#define NITERBW    10
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

  /* Allreduce variables */
  size_t               allreduce_num_algorithm[2];
  pami_algorithm_t    *allreduce_always_works_algo = NULL;
  pami_metadata_t     *allreduce_always_works_md = NULL;
  pami_algorithm_t    *allreduce_must_query_algo = NULL;
  pami_metadata_t     *allreduce_must_query_md = NULL;
  pami_xfer_type_t     allreduce_xfer = PAMI_XFER_ALLREDUCE;
  volatile unsigned    allreduce_poll_flag = 0;

  int                  root = 0, i, j, nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          allreduce;


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

  /*  Query the world geometry for allreduce algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            allreduce_xfer,
                            allreduce_num_algorithm,
                            &allreduce_always_works_algo,
                            &allreduce_always_works_md,
                            &allreduce_must_query_algo,
                            &allreduce_must_query_md);

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
  /* Disable NULL and byte operations     */
  for (i = 0, j = DT_NULL; i < OP_COUNT; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_BYTE; i < OP_COUNT; i++)validTable[i][j] = 0;
  for (j = 0, i = OP_COPY; j < DT_COUNT; j++) validTable[i][j] = 0;
  for (j = 0, i = OP_NOOP; j < DT_COUNT; j++) validTable[i][j] = 0;

  /*--------------------------------------*/
  /* Disable non-LOC ops on LOC dt's      */
  for (i = 0, j = DT_LOC_2INT      ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_SHORT_INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_FLOAT_INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_DOUBLE_INT; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_2FLOAT    ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
  for (i = 0, j = DT_LOC_2DOUBLE   ; i < OP_MAXLOC; i++)validTable[i][j] = 0;

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

  for (nalg = 0; nalg < allreduce_num_algorithm[0]; nalg++)
    {
      if (task_id == root)
        {
          printf("# Allreduce Bandwidth Test -- root = %d protocol: %s\n", root, allreduce_always_works_md[nalg].name);
          printf("# Size(bytes)           cycles    bytes/sec    usec\n");
          printf("# -----------      -----------    -----------    ---------\n");
        }
      if(((strstr(allreduce_always_works_md[nalg].name,selected) == NULL) && selector) ||
         ((strstr(allreduce_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

      barrier.cb_done   = cb_done;
      barrier.cookie    = (void*) & bar_poll_flag;
      barrier.algorithm = bar_always_works_algo[0];
      blocking_coll(context, &barrier, &bar_poll_flag);

      allreduce.cb_done   = cb_done;
      allreduce.cookie    = (void*) & allreduce_poll_flag;
      allreduce.algorithm = allreduce_always_works_algo[nalg];
      allreduce.cmd.xfer_allreduce.sndbuf    = sbuf;
      allreduce.cmd.xfer_allreduce.stype     = PAMI_TYPE_BYTE;
      allreduce.cmd.xfer_allreduce.stypecount = 0;
      allreduce.cmd.xfer_allreduce.rcvbuf    = rbuf;
      allreduce.cmd.xfer_allreduce.rtype     = PAMI_TYPE_BYTE;
      allreduce.cmd.xfer_allreduce.rtypecount = 0;

      for (dt = 0; dt < dt_count; dt++)
      {
        for (op = 0; op < op_count; op++)
          {
            if (validTable[op][dt])
              {
                if (task_id == root)
                  printf("Running Allreduce: %s, %s\n", dt_array_str[dt], op_array_str[op]);

                for (i = 1; i <= COUNT; i *= 2)
                  {
                    size_t sz=get_type_size(dt_array[dt]);
                    long long dataSent = i * sz;
                    int niter;

                    if (dataSent < CUTOFF)
                      niter = NITERLAT;
                    else
                      niter = NITERBW;

                    allreduce.cmd.xfer_allreduce.stypecount = i;
                    allreduce.cmd.xfer_allreduce.rtypecount = dataSent;
                    allreduce.cmd.xfer_allreduce.stype = dt_array[dt];
                    allreduce.cmd.xfer_allreduce.op    = op_array[op];

#ifdef CHECK_DATA
                    reduce_initialize_sndbuf (sbuf, i, op, dt, task_id);
#endif
                    blocking_coll(context, &barrier, &bar_poll_flag);
                    ti = timer();

                    for (j = 0; j < niter; j++)
                      {
                        blocking_coll(context, &allreduce, &allreduce_poll_flag);
                      }

                    tf = timer();
                    blocking_coll(context, &barrier, &bar_poll_flag);

#ifdef CHECK_DATA
                    int rc = reduce_check_rcvbuf (rbuf, i, op, dt, num_tasks);

                    if (rc) fprintf(stderr, "FAILED validation\n");

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
  }

  rc = pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(allreduce_always_works_algo);
  free(allreduce_always_works_md);
  free(allreduce_must_query_algo);
  free(allreduce_must_query_md);

  return 0;
}
