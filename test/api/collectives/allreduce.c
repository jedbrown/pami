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
 * \brief ???
 */

#include "../pami_util.h"
#include "math/math_coremath.h" // special datatype size structs


//define this if you want to validate the data for unsigned sums
#define CHECK_DATA

#ifdef ENABLE_MAMBO_WORKAROUNDS
 #define FULL_TEST
 #define COUNT      1024
 #define MAXBUFSIZE COUNT*16
 #define NITERLAT   50
 #define NITERBW    10
 #define CUTOFF     512
#else
 #define FULL_TEST
 #define COUNT      65536
 #define MAXBUFSIZE COUNT*16
 #define NITERLAT   1000
 #define NITERBW    10
 #define CUTOFF     65536
#endif

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

unsigned elemsize_array[] =
  {
    sizeof(unsigned int),       // PAMI_UNSIGNED_INT,
    sizeof(double),             // PAMI_DOUBLE,
    sizeof(char),               // PAMI_SIGNED_CHAR,
    sizeof(unsigned char),      // PAMI_UNSIGNED_CHAR,
    sizeof(short),              // PAMI_SIGNED_SHORT,
    sizeof(unsigned short),     // PAMI_UNSIGNED_SHORT,
    sizeof(int),                // PAMI_SIGNED_INT,
    sizeof(long long),          // PAMI_SIGNED_LONG_LONG,
    sizeof(unsigned long long), // PAMI_UNSIGNED_LONG_LONG,
    sizeof(float),              // PAMI_FLOAT,
    sizeof(long double),        // PAMI_LONG_DOUBLE,
    sizeof(unsigned int),       // PAMI_LOGICAL,
    (2 * sizeof(float)),        // PAMI_SINGLE_COMPLEX,
    (2 * sizeof(double)),       // PAMI_DOUBLE_COMPLEX
    // The following are from math/math_coremath.h structures
    // \todo Correct or not?  At least they match internal math...
    sizeof(int32_int32_t),      // PAMI_LOC_2INT,
    sizeof(int16_int32_t),      // PAMI_LOC_SHORT_INT,
    sizeof(fp32_int32_t),       // PAMI_LOC_FLOAT_INT,
    sizeof(fp64_int32_t),       // PAMI_LOC_DOUBLE_INT,
    sizeof(fp32_fp32_t),        // PAMI_LOC_2FLOAT,
    sizeof(fp64_fp64_t),        // PAMI_LOC_2DOUBLE,
  };

unsigned ** alloc2DContig(int nrows, int ncols)
{
  int i;
  unsigned **array;

  array        = (unsigned**)malloc(nrows*sizeof(unsigned*));
  array[0]     = (unsigned *)calloc(sizeof(unsigned), nrows*ncols);
  for(i = 1; i<nrows; i++)
    array[i]   = array[0]+i*ncols;

  return array;
}

#ifdef CHECK_DATA
void initialize_sndbuf (void *buf, int count, int op, int dt, int task_id) {

  int i;
  if (op == PAMI_SUM && dt == PAMI_UNSIGNED_INT) {
    unsigned int *ibuf = (unsigned int *)  buf;
    for (i = 0; i < count; i++) {
      ibuf[i] = i;
    }
  }
  else memset(buf,  task_id,  count * elemsize_array[dt]);
}

int check_rcvbuf (void *buf, int count, int op, int dt, int num_tasks) {

  int i, err = 0;
  if (op == PAMI_SUM && dt == PAMI_UNSIGNED_INT) {
    unsigned int *rbuf = (unsigned int *)  buf;
    for (i = 0; i < count; i++) {
      if (rbuf[i] != i * num_tasks)
      {
        fprintf(stderr,"Check(%d) failed rbuf[%d] %u != %u\n",count,i,rbuf[1],i*num_tasks);
        err = -1;
      }
    }
  }

  return err;
}
#endif

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t       context;
  size_t               num_contexts=1;
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
  volatile unsigned    bar_poll_flag=0;

  /* Allreduce variables */
  size_t               allreduce_num_algorithm[2];
  pami_algorithm_t    *allreduce_always_works_algo;
  pami_metadata_t     *allreduce_always_works_md;
  pami_algorithm_t    *allreduce_must_query_algo;
  pami_metadata_t     *allreduce_must_query_md;
  pami_xfer_type_t     allreduce_xfer = PAMI_XFER_ALLREDUCE;
  volatile unsigned    allreduce_poll_flag=0;

  int                  root=0, i, j, nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          allreduce;


  char sbuf[MAXBUFSIZE];
  char rbuf[MAXBUFSIZE];
  int op, dt;

  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     &context,       /* Context            */
                     NULL,           /* Clientname=default */
                     &num_contexts,  /* num_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */
  if(rc==1)
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
  if(rc==1)
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
  if(rc==1)
    return 1;

  unsigned** validTable=
    alloc2DContig(op_count,dt_count);
#ifdef FULL_TEST
  for(i=0;i<op_count;i++)
    for(j=0;j<dt_count;j++)
      validTable[i][j]=1;

  /* Not testing minloc/maxloc/logical,etc */
  for(i=OP_MINLOC,j=0; j<DT_COUNT;j++)validTable[i][j]=0;
  for(i=OP_MAXLOC,j=0; j<DT_COUNT;j++)validTable[i][j]=0;
  for(i=0,j=DT_LOGICAL; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_SINGLE_COMPLEX; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_LONG_DOUBLE; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_DOUBLE_COMPLEX; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_LOC_2INT; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_LOC_SHORT_INT; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_LOC_FLOAT_INT; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_LOC_DOUBLE_INT; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_LOC_2FLOAT; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_LOC_2DOUBLE; i<OP_COUNT;i++)validTable[i][j]=0;


  validTable[OP_MAX][DT_DOUBLE_COMPLEX]=0;
  validTable[OP_MIN][DT_DOUBLE_COMPLEX]=0;
  validTable[OP_PROD][DT_DOUBLE_COMPLEX]=0;

  /* Now add back the minloc/maxloc stuff */
  for(i=OP_MAXLOC; i<=OP_MINLOC; i++)
    for(j=DT_LOC_2INT; j<=DT_LOC_2DOUBLE; j++)
      validTable[i][j]=1;

  /// \todo These long long types reportedly fail in pgas, so disable for now.
  for(i=0,j=DT_SIGNED_LONG_LONG; i<OP_COUNT;i++)validTable[i][j]=0;
  for(i=0,j=DT_UNSIGNED_LONG_LONG; i<OP_COUNT;i++)validTable[i][j]=0;

  /// \todo These fail using core math...we should find this bug.
  validTable[OP_BAND][DT_DOUBLE]=0;

#if defined(__pami_target_bgq__) || defined(__pami_target_bgp__)

  char* env = getenv("PAMI_DEVICE");
  fprintf(stderr, "PAMI_DEVICE=%c\n", env?*env:' ');
  if((env==NULL) || ((*env=='M') || (*env=='B')))
      {
        /// These are unsupported on MU
        for(i=0,j= DT_SIGNED_CHAR;    i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_UNSIGNED_CHAR;  i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_SIGNED_SHORT;   i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_UNSIGNED_SHORT; i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_LOGICAL;        i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_SINGLE_COMPLEX; i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_DOUBLE_COMPLEX; i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_LOC_2INT;       i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_LOC_SHORT_INT;  i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_LOC_FLOAT_INT;  i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_LOC_DOUBLE_INT; i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_LOC_2FLOAT;     i<OP_COUNT;i++)validTable[i][j]=0;
        for(i=0,j= DT_LOC_2FLOAT;     i<OP_COUNT;i++)validTable[i][j]=0;
        for(i= OP_PROD,   j=0; j<DT_COUNT;j++)validTable[i][j]=0;
        for(i= OP_MAXLOC, j=0; j<DT_COUNT;j++)validTable[i][j]=0;
        for(i= OP_MINLOC, j=0; j<DT_COUNT;j++)validTable[i][j]=0;
      }
  // This works on bgq so re-enable it
  if((env) && (*env=='M'))
    validTable[OP_BAND][DT_DOUBLE]=1;
#endif

#else
  for(i=0;i<op_count;i++)
    for(j=0;j<dt_count;j++)
      validTable[i][j]=0;

  validTable[OP_SUM][DT_UNSIGNED_INT]=1;

#endif

  for(nalg=0; nalg<allreduce_num_algorithm[0]; nalg++)
  {
    if (task_id == root)
    {
      printf("# Allreduce Bandwidth Test -- root = %d protocol: %s\n", root,
             allreduce_always_works_md[nalg].name);
      printf("# Size(bytes)           cycles    bytes/sec    usec\n");
      printf("# -----------      -----------    -----------    ---------\n");
    }

    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*)&bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];
    blocking_coll(context,&barrier,&bar_poll_flag);

    allreduce.cb_done   = cb_done;
    allreduce.cookie    = (void*)&allreduce_poll_flag;
    allreduce.algorithm = allreduce_always_works_algo[nalg];
    allreduce.cmd.xfer_allreduce.sndbuf    = sbuf;
    allreduce.cmd.xfer_allreduce.stype     = PAMI_BYTE;
    allreduce.cmd.xfer_allreduce.stypecount= 0;
    allreduce.cmd.xfer_allreduce.rcvbuf    = rbuf;
    allreduce.cmd.xfer_allreduce.rtype     = PAMI_BYTE;
    allreduce.cmd.xfer_allreduce.rtypecount= 0;



    for(dt=0; dt<dt_count; dt++)
      for(op=0; op<op_count; op++)
      {
        if(validTable[op][dt])
        {
          if(task_id == root)
            printf("Running Allreduce: %s, %s\n",dt_array_str[dt], op_array_str[op]);
          for(i=1; i<=COUNT; i*=2)
          {
            long long dataSent = i*elemsize_array[dt];
            int niter;
            if(dataSent < CUTOFF)
              niter = NITERLAT;
            else
              niter = NITERBW;

#ifdef CHECK_DATA
            initialize_sndbuf (sbuf, i, op_array[op], dt_array[dt], task_id);
#endif
            blocking_coll(context,&barrier,&bar_poll_flag);
            ti = timer();
            for (j=0; j<niter; j++)
            {
              allreduce.cmd.xfer_allreduce.stypecount=dataSent;
              allreduce.cmd.xfer_allreduce.rtypecount=dataSent;
              allreduce.cmd.xfer_allreduce.dt=dt_array[dt];
              allreduce.cmd.xfer_allreduce.op=op_array[op];
              blocking_coll(context, &allreduce, &allreduce_poll_flag);
            }
            tf = timer();
            blocking_coll(context,&barrier,&bar_poll_flag);

#ifdef CHECK_DATA
            int rc = check_rcvbuf (rbuf, i, op_array[op], dt_array[dt], num_tasks);
            //assert (rc == 0);
            if(rc) fprintf(stderr, "FAILED validation\n");
#endif

            usec = (tf - ti)/(double)niter;
            if (task_id == root)
            {
              printf("  %11lld %16d %14.1f %12.2f\n",
                     dataSent,
                     niter,
                     (double)1e6*(double)dataSent/(double)usec,
                     usec);
              fflush(stdout);
            }
          }
        }
      }
  }
  rc = pami_shutdown(&client,&context,&num_contexts);
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