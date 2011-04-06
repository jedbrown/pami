/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/api/collectives/reduce.c
 * \brief Simple reduce on world geometry
 */

#include "../../../../api/pami_util.h"
#include <Arch.h> /* Don't use PAMI_MAX_PROC_PER_NODE in 'real' api test*/

/*define this if you want to validate the data */
#define CHECK_DATA

#define FULL_TEST 0 /* see envvar TEST_DT/TEST_OP for overrides */
unsigned full_test = FULL_TEST;

#define COUNT      65536        /* see envvar TEST_COUNT for overrides */
unsigned max_count = COUNT;

#define OFFSET     0            /* see envvar TEST_OFFSET for overrides */
unsigned buffer_offset = OFFSET;

#define MAXBUFSIZE max_count*16

#define NITERLAT   1            /* see envvar TEST_ITER for overrides */
unsigned niterlat  = NITERLAT;

#define NITERBW    MIN(10, niterlat/100+1)

#define CUTOFF     65536

pami_op op_array[] =
{
  PAMI_SUM,
  PAMI_MAX,
  PAMI_MIN,
  PAMI_PROD,
  PAMI_LAND,
  PAMI_LOR,
  PAMI_LXOR,
  PAMI_BAND,
  PAMI_BOR,
  PAMI_BXOR,
  PAMI_MAXLOC,
  PAMI_MINLOC,
};
enum opNum
{
  OP_SUM,
  OP_MAX,
  OP_MIN,
  OP_PROD,
  OP_LAND,
  OP_LOR,
  OP_LXOR,
  OP_BAND,
  OP_BOR,
  OP_BXOR,
  OP_MAXLOC,
  OP_MINLOC,
  OP_COUNT
};
int op_count = OP_COUNT;

pami_dt dt_array[] =
{
  PAMI_UNSIGNED_INT,
  PAMI_DOUBLE,
  PAMI_SIGNED_CHAR,
  PAMI_UNSIGNED_CHAR,
  PAMI_SIGNED_SHORT,
  PAMI_UNSIGNED_SHORT,
  PAMI_SIGNED_INT,
  PAMI_SIGNED_LONG_LONG,
  PAMI_UNSIGNED_LONG_LONG,
  PAMI_FLOAT,
  PAMI_LONG_DOUBLE,
  PAMI_LOGICAL,
  PAMI_SINGLE_COMPLEX,
  PAMI_DOUBLE_COMPLEX,
  PAMI_LOC_2INT,
  PAMI_LOC_SHORT_INT,
  PAMI_LOC_FLOAT_INT,
  PAMI_LOC_DOUBLE_INT,
  PAMI_LOC_2FLOAT,
  PAMI_LOC_2DOUBLE,
};

enum dtNum
{
  DT_UNSIGNED_INT,
  DT_DOUBLE,
  DT_SIGNED_CHAR,
  DT_UNSIGNED_CHAR,
  DT_SIGNED_SHORT,
  DT_UNSIGNED_SHORT,
  DT_SIGNED_INT,
  DT_SIGNED_LONG_LONG,
  DT_UNSIGNED_LONG_LONG,
  DT_FLOAT,
  DT_LONG_DOUBLE,
  DT_LOGICAL,
  DT_SINGLE_COMPLEX,
  DT_DOUBLE_COMPLEX,
  DT_LOC_2INT,
  DT_LOC_SHORT_INT,
  DT_LOC_FLOAT_INT,
  DT_LOC_DOUBLE_INT,
  DT_LOC_2FLOAT,
  DT_LOC_2DOUBLE,
  DT_COUNT
};
int dt_count = DT_COUNT;


const char * op_array_str[] =
{
  "PAMI_SUM",
  "PAMI_MAX",
  "PAMI_MIN",
  "PAMI_PROD",
  "PAMI_LAND",
  "PAMI_LOR",
  "PAMI_LXOR",
  "PAMI_BAND",
  "PAMI_BOR",
  "PAMI_BXOR",
  "PAMI_MAXLOC",
  "PAMI_MINLOC"
};


const char * dt_array_str[] =
{
  "PAMI_UNSIGNED_INT",
  "PAMI_DOUBLE",
  "PAMI_SIGNED_CHAR",
  "PAMI_UNSIGNED_CHAR",
  "PAMI_SIGNED_SHORT",
  "PAMI_UNSIGNED_SHORT",
  "PAMI_SIGNED_INT",
  "PAMI_SIGNED_LONG_LONG",
  "PAMI_UNSIGNED_LONG_LONG",
  "PAMI_FLOAT",
  "PAMI_LONG_DOUBLE",
  "PAMI_LOGICAL",
  "PAMI_SINGLE_COMPLEX",
  "PAMI_DOUBLE_COMPLEX",
  "PAMI_LOC_2INT",
  "PAMI_LOC_SHORT_INT",
  "PAMI_LOC_FLOAT_INT",
  "PAMI_LOC_DOUBLE_INT",
  "PAMI_LOC_2FLOAT",
  "PAMI_LOC_2DOUBLE"
};

char *protocolName;

unsigned ** alloc2DContig(int nrows, int ncols)
{
  int i;
  unsigned **array;

  array        = (unsigned**)malloc(nrows * sizeof(unsigned*));
  array[0]     = (unsigned *)calloc(sizeof(unsigned), nrows * ncols);

  for (i = 1; i < nrows; i++)
    array[i]   = array[0] + i * ncols;

  return array;
}

#ifdef CHECK_DATA
void initialize_sndbuf (void *buf, int count, int op, int dt, int task_id, int num_tasks)
{

  int i;

  if (op_array[op] == PAMI_SUM && dt_array[dt] == PAMI_UNSIGNED_INT)
  {
    unsigned int *ibuf = (unsigned int *)  buf;

    for (i = 0; i < count; i++)
    {
      ibuf[i] = i;
    }
  }
  else if (op_array[op] == PAMI_SUM && dt_array[dt] == PAMI_DOUBLE)
  {
    double *dbuf = (double *)  buf;

    for (i = 0; i < count; i++)
    {
      dbuf[i] = 1.0*i;
    }
  }
  else if ((op_array[op] == PAMI_MAX || op_array[op] == PAMI_MIN) && dt_array[dt] == PAMI_DOUBLE)
  {
    memset(buf,  0,  count * sizeof(double));
    double *dbuf = (double *)  buf;

    for (i = task_id; i < count; i += num_tasks)
    {
      dbuf[i] = 1.0 * task_id;
    }
  }
  else
  {
    size_t sz;
    PAMI_Dt_query (dt_array[dt], &sz);
    memset(buf,  task_id,  count * sz);
  }
}

int check_rcvbuf (void *buf, int count, int op, int dt, int task_id, int num_tasks)
{

  int i, err = 0;

  if (op_array[op] == PAMI_SUM && dt_array[dt] == PAMI_UNSIGNED_INT)
  {
    unsigned int *rcvbuf = (unsigned int *)  buf;

    for (i = 0; i < count; i++)
    {
      if (rcvbuf[i] != i * num_tasks)
      {
        fprintf(stderr, "%s:Check %s/%s(%d) failed rcvbuf[%d] %u != %u\n", protocolName, dt_array_str[dt], op_array_str[op], count, i, rcvbuf[1], i*num_tasks);
        err = -1;
        return err;
      }
    }
  }
  else if (op_array[op] == PAMI_SUM && dt_array[dt] == PAMI_DOUBLE)
  {
    double *rcvbuf = (double *)  buf;

    for (i = 0; i < count; i++)
    {
      if (rcvbuf[i] != 1.0 * i * num_tasks)
      {
        fprintf(stderr, "%s:Check %s/%s(%d) failed rcvbuf[%d] %f != %f\n", protocolName, dt_array_str[dt], op_array_str[op], count, i, rcvbuf[i], (double)1.0*i*num_tasks);
        err = -1;
        return err;
      }
    }
  }
  else if (op_array[op] == PAMI_MIN && dt_array[dt] == PAMI_DOUBLE)
  {
    double *rcvbuf = (double *)  buf;

    for (i = 0; i < count; i++)
    {
      if (rcvbuf[i] != 0.0)
      {
        fprintf(stderr, "%s:Check %s/%s(%d) failed rcvbuf[%d] %f != %f\n", protocolName, dt_array_str[dt], op_array_str[op], count, i, rcvbuf[i], (double)0.0);
        err = -1;
        return err;
      }
    }
  }
  else if (op_array[op] == PAMI_MAX && dt_array[dt] == PAMI_DOUBLE)
  {
    double *rcvbuf = (double *)  buf;

    for (i = 0; i < count; i++)
    {
      if (rcvbuf[i] != 1.0 * (i % num_tasks))
      {
        fprintf(stderr, "%s:Check %s/%s(%d) failed rcvbuf[%d] %f != %f\n", protocolName, dt_array_str[dt], op_array_str[op], count, i, rcvbuf[i], (double)1.0*(i % num_tasks));
        err = -1;
        return err;
      }
    }
  }

  return err;
}
#endif

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context[PAMI_MAX_PROC_PER_NODE]; /* arbitrary max */
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


  int op, dt;

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

  /* \note Test environment variable" TEST_DT=pami datatype string       */
  char* sDt = getenv("TEST_DT");

  /* \note Test environment variable" TEST_OP=pami operation string      */
  char* sOp = getenv("TEST_OP");

  /* Override FULL_TEST with 'ALL' */
  if ((sDt && !strcmp(sDt, "ALL")) || (sOp && !strcmp(sOp, "ALL"))) full_test = 1;

  /* \note Test environment variable" TEST_COUNT=N max count     */
  char* sCount = getenv("TEST_COUNT");

  /* Override COUNT */
  if (sCount) max_count = atoi(sCount);

  /* \note Test environment variable" TEST_OFFSET=N buffer offset/alignment*/
  char* sOffset = getenv("TEST_OFFSET");

  /* Override OFFSET */
  if (sOffset) buffer_offset = atoi(sOffset);

  /* \note Test environment variable" TEST_ITER=N iterations      */
  char* sIter = getenv("TEST_ITER");

  /* Override NITERLAT */
  if (sIter) niterlat = atoi(sIter);

  /* \note Test environment variable" TEST_NUM_CONTEXTS=N, defaults to 1.*/
  char* snum_contexts = getenv("TEST_NUM_CONTEXTS");

  if (snum_contexts) num_contexts = atoi(snum_contexts);

  assert(num_contexts > 0);
  assert(num_contexts <= PAMI_MAX_PROC_PER_NODE);

  /*  Allocate buffer(s) */
  int err = 0;
  void* sbuf = NULL;
  err = posix_memalign(&sbuf, 128, MAXBUFSIZE+buffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + buffer_offset;
  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, MAXBUFSIZE+buffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + buffer_offset;

  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     context,        /* Context            */
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
                             context[0],
                             &world_geometry,
                             barrier_xfer,
                             barrier_num_algorithm,
                             &bar_always_works_algo,
                             &bar_always_works_md,
                             &bar_must_query_algo,
                             &bar_must_query_md);

  if (rc == 1)
    return 1;

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];

  unsigned iContext = 0;

  for (; iContext < num_contexts; ++iContext)
  {

    if (task_id == 0)
      printf("# Context: %u\n", iContext);

    /*  Query the world geometry for reduce algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               reduce_xfer,
                               reduce_num_algorithm,
                               &reduce_always_works_algo,
                               &reduce_always_works_md,
                               &reduce_must_query_algo,
                               &reduce_must_query_md);

    if (rc == 1)
      return 1;

    unsigned** validTable =
    alloc2DContig(op_count, dt_count);


    /* Setup operation and datatype tables*/
    unsigned force = 0; /* don't force the dt/op selected */
    if (full_test)
    {
      for (i = 0; i < op_count; i++)
        for (j = 0; j < dt_count; j++)
          validTable[i][j] = 1;

    }
    else if (sDt && sOp)
    {
      force = 1; /* force the dt/op*/
      for (i = 0; i < op_count; i++)
        for (j = 0; j < dt_count; j++)
          if (!strcmp(sDt, dt_array_str[j]) &&
              !strcmp(sOp, op_array_str[i]))
            validTable[i][j] = 1;
          else
            validTable[i][j] = 0;
    }
    else if (sOp)
    {
      for (i = 0; i < op_count; i++)
        for (j = 0; j < dt_count; j++)
          if (!strcmp(sOp, op_array_str[i]))
            validTable[i][j] = 1;
          else
            validTable[i][j] = 0;
    }
    else if (sDt)
    {
      for (i = 0; i < op_count; i++)
        for (j = 0; j < dt_count; j++)
          if (!strcmp(sDt, dt_array_str[j]))
            validTable[i][j] = 1;
          else
            validTable[i][j] = 0;
    }
    else  /* minimal/default test */
    {

      for (i = 0; i < op_count; i++)
        for (j = 0; j < dt_count; j++)
          validTable[i][j] = 0;

      validTable[OP_SUM][DT_UNSIGNED_INT] = 1;
      validTable[OP_SUM][DT_DOUBLE] = 1;
      validTable[OP_MAX][DT_DOUBLE] = 1;
      validTable[OP_MIN][DT_DOUBLE] = 1;

    }
    if (!force) /* not forcing the op/dt*/
    {
      /*--------------------------------------*/
      /* Disable unsupported ops on complex   */
      /* Only sum, prod                       */
      for (i = 0, j = DT_SINGLE_COMPLEX; i < OP_COUNT; i++)if (i!=OP_SUM && i!=OP_PROD) validTable[i][j] = 0;
      for (i = 0, j = DT_DOUBLE_COMPLEX; i < OP_COUNT; i++)if (i!=OP_SUM && i!=OP_PROD) validTable[i][j] = 0;

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
    }

    for (nalg = 0; nalg < reduce_num_algorithm[0]; nalg++)
    {
      if (task_id == 0) /* root not set yet */
      {
        printf("# Reduce Bandwidth Test -- context = %d, root varies, protocol: %s\n", 
               iContext, reduce_always_works_md[nalg].name);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(reduce_always_works_md[nalg].name,selected) == NULL) && selector) ||
          ((strstr(reduce_always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

      protocolName = reduce_always_works_md[nalg].name;

      reduce.cb_done   = cb_done;
      reduce.cookie    = (void*) & reduce_poll_flag;
      reduce.algorithm = reduce_always_works_algo[nalg];
      reduce.cmd.xfer_reduce.sndbuf    = sbuf;
      reduce.cmd.xfer_reduce.stype     = PAMI_TYPE_CONTIGUOUS;
      reduce.cmd.xfer_reduce.stypecount = 0;
      reduce.cmd.xfer_reduce.rtype     = PAMI_TYPE_CONTIGUOUS;
      reduce.cmd.xfer_reduce.rtypecount = 0;

      for (dt = 0; dt < dt_count; dt++)
      {
        for (op = 0; op < op_count; op++)
        {
          if (validTable[op][dt])
          {
            if (task_id == 0) /* root not set yet */
              printf("Running Reduce: %s, %s\n", dt_array_str[dt], op_array_str[op]);

            for (i = 1; i <= max_count; i *= 2)
            {
              size_t sz;
              PAMI_Dt_query (dt_array[dt], &sz);
              long long dataSent = i * sz;
              int niter;

              if (dataSent < CUTOFF)
                niter = niterlat;
              else
                niter = NITERBW;

              root = 0;


              reduce.cmd.xfer_reduce.stypecount = dataSent;
              reduce.cmd.xfer_reduce.rtypecount = dataSent;
              reduce.cmd.xfer_reduce.dt = dt_array[dt];
              reduce.cmd.xfer_reduce.op = op_array[op];

#ifdef CHECK_DATA
              initialize_sndbuf (sbuf, i, op, dt, task_id, num_tasks);
#endif
              /* We aren't testing barrier itself, so use context 0. */
              blocking_coll(context[0], &barrier, &bar_poll_flag);
              ti = timer();

              for (j = 0; j < niter; j++)
              {
                reduce.cmd.xfer_reduce.root    = root;

                if (task_id == root)
                  reduce.cmd.xfer_reduce.rcvbuf    = rbuf;
                else
                  reduce.cmd.xfer_reduce.rcvbuf    = NULL;
                blocking_coll(context[iContext], &reduce, &reduce_poll_flag);

                root = (root + 1) % num_tasks;
              }

              tf = timer();
              /* We aren't testing barrier itself, so use context 0. */
              blocking_coll(context[0], &barrier, &bar_poll_flag);

#ifdef CHECK_DATA

              if (task_id < niter) /* was root at least once in niter loop */
              {
                rc |= check_rcvbuf (rbuf, i, op, dt, task_id, num_tasks);

                if (rc) fprintf(stderr, "%s FAILED validation\n", protocolName);
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
    }

    free(reduce_always_works_algo);
    free(reduce_always_works_md);
    free(reduce_must_query_algo);
    free(reduce_must_query_md);

  } /*for(unsigned iContext = 0; iContext < num_contexts; ++iContexts)*/

  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);

  sbuf = (char*)sbuf - buffer_offset;
  free(sbuf);
  rbuf = (char*)rbuf - buffer_offset;
  free(rbuf);

  rc |= pami_shutdown(&client, context, &num_contexts);
  return rc;
}