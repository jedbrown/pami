/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/api/collectives/allreduce_subcomm.c
 * \brief Simple Allreduce test on sub-geometries
 */

#include "../../../../api/pami_util.h"

/*define this if you want to validate the data */
#define CHECK_DATA

#define FULL_TEST 0 // see envvar TEST_DT/TEST_OP for overrides
unsigned full_test = FULL_TEST;

#define COUNT      65536        // see envvar TEST_COUNT for overrides
unsigned max_count = COUNT;

#define OFFSET     0            // see envvar TEST_OFFSET for overrides
unsigned buffer_offset = OFFSET;

#define MAXBUFSIZE max_count*16

#define NITERLAT   1            // see envvar TEST_ITER for overrides
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
      dbuf[i] = 1.0 * i;
    }
  }
  else if ((op_array[op] == PAMI_MAX || op_array[op] == PAMI_MIN) && dt_array[dt] == PAMI_DOUBLE)
  {
    memset(buf,  0,  count * sizeof(double));
    double *dbuf = (double *)  buf;

    for (i = 0; i < count; i += num_tasks)
    {
      dbuf[i+task_id] = 1.0 * task_id;
//      fprintf(stderr, "Init(%d) buf[%u]=%f\n",count,i+task_id, dbuf[i+task_id]);
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
//      fprintf(stderr, "Check(%d) buf[%u]=%f\n",count, i, rcvbuf[i]);
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
//      fprintf(stderr, "Check(%d) buf[%u]=%f\n",count, i, rcvbuf[i]);
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
  pami_task_t          task_id, local_task_id;
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
  volatile unsigned    newbar_poll_flag = 0;

  /* Allreduce variables */
  size_t               allreduce_num_algorithm[2];
  pami_algorithm_t    *allreduce_always_works_algo;
  pami_metadata_t     *allreduce_always_works_md;
  pami_algorithm_t    *allreduce_must_query_algo;
  pami_metadata_t     *allreduce_must_query_md;
  pami_xfer_type_t     allreduce_xfer = PAMI_XFER_ALLREDUCE;
  volatile unsigned    allreduce_poll_flag = 0;

  int                  root = 0, i, j, nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          allreduce;


  int op, dt;

  /* \note Test environment variable" TEST_PROTOCOL={-}substring.       */
  /* substring is used to select, or de-select (with -) test protocols */
  unsigned selector = 1;
  char* selected = getenv("TEST_PROTOCOL");

  if (!selected) selected = "";
  else if (selected[0] == '-')
  {
    selector = 0 ;
    ++selected;
  }

  /* \note Test environment variable" TEST_DT=pami datatype string       */
  char* sDt = getenv("TEST_DT");

  /* \note Test environment variable" TEST_OP=pami operation string      */
  char* sOp = getenv("TEST_OP");

  // Override FULL_TEST with 'ALL'
  if ((sDt && !strcmp(sDt, "ALL")) || (sOp && !strcmp(sOp, "ALL"))) full_test = 1;

  /* \note Test environment variable" TEST_COUNT=N max count     */
  char* sCount = getenv("TEST_COUNT");

  // Override COUNT
  if (sCount) max_count = atoi(sCount);

  /* \note Test environment variable" TEST_OFFSET=N buffer offset/alignment*/
  char* sOffset = getenv("TEST_OFFSET");

  // Override OFFSET
  if (sOffset) buffer_offset = atoi(sOffset);

  /* \note Test environment variable" TEST_ITER=N iterations      */
  char* sIter = getenv("TEST_ITER");

  // Override NITERLAT
  if (sIter) niterlat = atoi(sIter);

  /* \note Test environment variable" TEST_PARENTLESS=0 or 1, defaults to 0.
     0 - world_geometry is the parent
     1 - parentless                                                      */
  char* sParentless = getenv("TEST_PARENTLESS");
  unsigned parentless = 0; /*Not parentless*/

  if (sParentless) parentless = atoi(sParentless);

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
  if (num_tasks == 1)
  {
    fprintf(stderr,"No subcomms on 1 node\n");
    return 0;
  }
  assert(task_id >=0);
  assert(task_id < num_tasks);

  unsigned iContext = 0;

  for (; iContext < num_contexts; ++iContext)
  {

    if (task_id == 0)
      printf("# Context: %u\n", iContext);

    /*  Query the world geometry for barrier algorithms */
    rc |= query_geometry_world(client,
                              context[iContext],
                              &world_geometry,
                              barrier_xfer,
                              barrier_num_algorithm,
                              &bar_always_works_algo,
                              &bar_always_works_md,
                              &bar_must_query_algo,
                              &bar_must_query_md);

    if (rc == 1)
      return 1;

    /*  Create the subgeometry */
    pami_geometry_range_t *range;
    int                    rangecount;
    pami_geometry_t        newgeometry;
    size_t                 newbar_num_algo[2];
    pami_algorithm_t      *newbar_algo        = NULL;
    pami_metadata_t       *newbar_md          = NULL;
    pami_algorithm_t      *q_newbar_algo      = NULL;
    pami_metadata_t       *q_newbar_md        = NULL;

    pami_xfer_t            newbarrier;

    size_t                 set[2];
    int                    id;
    size_t                 half        = num_tasks / 2;
    range     = (pami_geometry_range_t *)malloc(((num_tasks + 1) / 2) * sizeof(pami_geometry_range_t));


    char *method = getenv("TEST_SPLIT_METHOD");

    // Default or TEST_SPLIT_METHOD=0 : divide in half
    if ((!method || !strcmp(method, "0")))
    {
      if (task_id < half)
      {
        range[0].lo = 0;
        range[0].hi = half - 1;
        set[0]   = 1;
        set[1]   = 0;
        id       = 1;
        root     = 0;
        num_tasks = half;
        local_task_id = task_id;
      }
      else
      {
        range[0].lo = half;
        range[0].hi = num_tasks - 1;
        set[0]   = 0;
        set[1]   = 1;
        id       = 2;
        root     = half;
        num_tasks = num_tasks - half;
        local_task_id = task_id - root;
      }

      rangecount = 1;
    }
    // TEST_SPLIT_METHOD=-1 : alternate ranks
    else if ((method && !strcmp(method, "-1")))
    {
      int i = 0;
      int iter = 0;;

      if ((task_id % 2) == 0)
      {
        for (i = 0; i < num_tasks; i++)
        {
          if ((i % 2) == 0)
          {
            range[iter].lo = i;
            range[iter].hi = i;
            iter++;
          }
        }

        set[0]   = 1;
        set[1]   = 0;
        id       = 2;
        root     = 0;
        rangecount = iter;
      }
      else
      {
        for (i = 0; i < num_tasks; i++)
        {
          if ((i % 2) != 0)
          {
            range[iter].lo = i;
            range[iter].hi = i;
            iter++;
          }
        }

        set[0]   = 0;
        set[1]   = 1;
        id       = 2;
        root     = 1;
        rangecount = iter;
      }

      num_tasks = iter;
      local_task_id = task_id/2;
    }
    // TEST_SPLIT_METHOD=N : Split the first "N" processes into a communicator
    else
    {
      half = atoi(method);
      if (task_id < half)
      {
        range[0].lo = 0;
        range[0].hi = half - 1;
        set[0]   = 1;
        set[1]   = 0;
        id       = 1;
        root     = 0;
        num_tasks = half;
        local_task_id = task_id;
      }
      else
      {
        range[0].lo = half;
        range[0].hi = num_tasks - 1;
        set[0]   = 0;
        set[1]   = 1;
        id       = 2;
        root     = half;
        num_tasks = num_tasks - half;
        local_task_id = task_id - root;
      }

      rangecount = 1;
    }

    /* Delay root tasks, and emulate that he's doing "other"
       message passing.  This will cause the geometry_create
       request from other nodes to be unexpected when doing
       parentless geometries and won't affect parented.      */
    if (task_id == root)
    {
      delayTest(1);
      unsigned ii = 0;

      for (; ii < num_contexts; ++ii)
        PAMI_Context_advance (context[ii], 1000);
    }

    rc |= create_and_query_geometry(client,
                                   context[iContext],
                                   parentless ? PAMI_NULL_GEOMETRY : world_geometry,
                                   &newgeometry,
                                   range,
                                   rangecount,
                                   id + iContext, // Unique id for each context
                                   barrier_xfer,
                                   newbar_num_algo,
                                   &newbar_algo,
                                   &newbar_md,
                                   &q_newbar_algo,
                                   &q_newbar_md);

    if (rc == 1)
      return 1;

    /*  Query the sub geometry for reduce algorithms */
    rc |= query_geometry(client,
                        context[iContext],
                        newgeometry,
                        allreduce_xfer,
                        allreduce_num_algorithm,
                        &allreduce_always_works_algo,
                        &allreduce_always_works_md,
                        &allreduce_must_query_algo,
                        &allreduce_must_query_md);

    if (rc == 1)
      return 1;

    /*  Set up world barrier */
    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];

    /*  Set up sub geometry barrier */
    newbarrier.cb_done   = cb_done;
    newbarrier.cookie    = (void*) & newbar_poll_flag;
    newbarrier.algorithm = newbar_algo[0];


    unsigned** validTable =
    alloc2DContig(op_count, dt_count);

    if (full_test)
    {
      /* Setup operation and datatype tables*/

      for (i = 0; i < op_count; i++)
        for (j = 0; j < dt_count; j++)
          validTable[i][j] = 1;

      /* Not testing minloc/maxloc/logical,etc */
      for (i = OP_MINLOC, j = 0; j < DT_COUNT; j++)validTable[i][j] = 0;

      for (i = OP_MAXLOC, j = 0; j < DT_COUNT; j++)validTable[i][j] = 0;

      for (i = 0, j = DT_LOGICAL; i < OP_COUNT; i++)validTable[i][j] = 0;

      for (i = 0, j = DT_SINGLE_COMPLEX; i < OP_COUNT; i++)validTable[i][j] = 0;

      for (i = 0, j = DT_LONG_DOUBLE; i < OP_COUNT; i++)validTable[i][j] = 0;

      for (i = 0, j = DT_DOUBLE_COMPLEX; i < OP_COUNT; i++)validTable[i][j] = 0;

      for (i = 0, j = DT_LOC_2INT; i < OP_COUNT; i++)validTable[i][j] = 0;

      for (i = 0, j = DT_LOC_SHORT_INT; i < OP_COUNT; i++)validTable[i][j] = 0;

      for (i = 0, j = DT_LOC_FLOAT_INT; i < OP_COUNT; i++)validTable[i][j] = 0;

      for (i = 0, j = DT_LOC_DOUBLE_INT; i < OP_COUNT; i++)validTable[i][j] = 0;

      for (i = 0, j = DT_LOC_2FLOAT; i < OP_COUNT; i++)validTable[i][j] = 0;

      for (i = 0, j = DT_LOC_2DOUBLE; i < OP_COUNT; i++)validTable[i][j] = 0;


      validTable[OP_MAX][DT_DOUBLE_COMPLEX] = 0;
      validTable[OP_MIN][DT_DOUBLE_COMPLEX] = 0;
      validTable[OP_PROD][DT_DOUBLE_COMPLEX] = 0;

      /* Now add back the minloc/maxloc stuff */
      for (i = OP_MAXLOC; i <= OP_MINLOC; i++)
        for (j = DT_LOC_2INT; j <= DT_LOC_2DOUBLE; j++)
          validTable[i][j] = 1;

      /** \todo These long long types reportedly fail in pgas, so disable for now.
      for (i = 0, j = DT_SIGNED_LONG_LONG; i < OP_COUNT; i++)validTable[i][j] = 0;*/

      for (i = 0, j = DT_UNSIGNED_LONG_LONG; i < OP_COUNT; i++)validTable[i][j] = 0;

      /** \todo These fail using core math...we should find this bug.
      validTable[OP_BAND][DT_DOUBLE] = 0;*/

    }
    else if (sDt && sOp)
    {
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
    else  // minimal/default test
    {

      for (i = 0; i < op_count; i++)
        for (j = 0; j < dt_count; j++)
          validTable[i][j] = 0;

      validTable[OP_SUM][DT_UNSIGNED_INT] = 1;
      validTable[OP_SUM][DT_DOUBLE] = 1;
      validTable[OP_MAX][DT_DOUBLE] = 1;
      validTable[OP_MIN][DT_DOUBLE] = 1;

    }

    for (nalg = 0; nalg < allreduce_num_algorithm[0]; nalg++)
    {
      int             i, j, k;

      for (k = 1; k >= 0; k--)
      {
        if (set[k])
        {
          if (task_id == root)
          {
            printf("# Allreduce Bandwidth Test -- context = %d, root = %d protocol: %s\n",
                   iContext, root, allreduce_always_works_md[nalg].name);
            printf("# Size(bytes)           cycles    bytes/sec    usec\n");
            printf("# -----------      -----------    -----------    ---------\n");
          }

          if (((strstr(allreduce_always_works_md[nalg].name, selected) == NULL) && selector) ||
              ((strstr(allreduce_always_works_md[nalg].name, selected) != NULL) && !selector))  continue;

          protocolName = allreduce_always_works_md[nalg].name;

          blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);

          allreduce.cb_done   = cb_done;
          allreduce.cookie    = (void*) & allreduce_poll_flag;
          allreduce.algorithm = allreduce_always_works_algo[nalg];
          allreduce.cmd.xfer_allreduce.sndbuf    = sbuf;
          allreduce.cmd.xfer_allreduce.stype     = PAMI_TYPE_CONTIGUOUS;
          allreduce.cmd.xfer_allreduce.stypecount = 0;
          allreduce.cmd.xfer_allreduce.rcvbuf    = rbuf;
          allreduce.cmd.xfer_allreduce.rtype     = PAMI_TYPE_CONTIGUOUS;
          allreduce.cmd.xfer_allreduce.rtypecount = 0;

          for (dt = 0; dt < dt_count; dt++)
            for (op = 0; op < op_count; op++)
            {
              if (validTable[op][dt])
              {
                if (task_id == root)
                  printf("Running Allreduce: %s, %s\n", dt_array_str[dt], op_array_str[op]);

                for (i = 2; i <= max_count; i *= 2)
                {
                  size_t sz;
                  PAMI_Dt_query (dt_array[dt], &sz);
                  long long dataSent = i * sz;
                  int niter;

                  if (dataSent < CUTOFF)
                    niter = niterlat;
                  else
                    niter = NITERBW;

                  allreduce.cmd.xfer_allreduce.stypecount = dataSent;
                  allreduce.cmd.xfer_allreduce.rtypecount = dataSent;
                  allreduce.cmd.xfer_allreduce.dt = dt_array[dt];
                  allreduce.cmd.xfer_allreduce.op = op_array[op];

#ifdef CHECK_DATA
                  initialize_sndbuf (sbuf, i, op, dt, local_task_id, num_tasks);
#endif
                  blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);
                  ti = timer();

                  for (j = 0; j < niter; j++)
                  {
                    blocking_coll(context[iContext], &allreduce, &allreduce_poll_flag);
                  }

                  tf = timer();
                  blocking_coll(context[iContext], &newbarrier, &newbar_poll_flag);

#ifdef CHECK_DATA
                  int rc = check_rcvbuf (rbuf, i, op, dt, local_task_id, num_tasks);

                  if (rc) fprintf(stderr, "%s FAILED validation\n", protocolName);

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
    }

    blocking_coll(context[iContext], &barrier, &bar_poll_flag);


    free(bar_always_works_algo);
    free(bar_always_works_md);
    free(bar_must_query_algo);
    free(bar_must_query_md);
    free(allreduce_always_works_algo);
    free(allreduce_always_works_md);
    free(allreduce_must_query_algo);
    free(allreduce_must_query_md);

  } /*for(unsigned iContext = 0; iContext < num_contexts; ++iContexts)*/

  sbuf = (char*)sbuf - buffer_offset;
  free(rbuf);
  rbuf = (char*)rbuf - buffer_offset;
  free(sbuf);

  rc |= pami_shutdown(&client, context, &num_contexts);
  return rc;
}
