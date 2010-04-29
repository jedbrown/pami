/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/allreduce.c
 * \brief ???
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <pami.h>

//define this if you want to validate the data for unsigned sums
//#define CHECK_DATA

#undef TRACE
#define TRACE(x) //fprintf x

// more verbose tracing
#undef TRACEV
#define TRACEV(x) //fprintf x

#include <assert.h>
#define TEST_abort()                       abort()
#define TEST_abortf(fmt...)                { fprintf(stderr, __FILE__ ":%d: \n", __LINE__); fprintf(stderr, fmt); abort(); }
#define TEST_assert(expr)                assert(expr)
#define TEST_assertf(expr, fmt...)       { if (!(expr)) TEST_abortf(fmt); }

#ifdef ENABLE_MAMBO_WORKAROUNDS
 //#define FULL_TEST
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
    PAMI_MAX,
    PAMI_MIN,
    PAMI_SUM,
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
    OP_MAX,
    OP_MIN,
    OP_SUM,
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
    PAMI_SIGNED_CHAR,
    PAMI_UNSIGNED_CHAR,
    PAMI_SIGNED_SHORT,
    PAMI_UNSIGNED_SHORT,
    PAMI_SIGNED_INT,
    PAMI_UNSIGNED_INT,
    PAMI_SIGNED_LONG_LONG,
    PAMI_UNSIGNED_LONG_LONG,
    PAMI_FLOAT,
    PAMI_DOUBLE,
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
    DT_SIGNED_CHAR,
    DT_UNSIGNED_CHAR,
    DT_SIGNED_SHORT,
    DT_UNSIGNED_SHORT,
    DT_SIGNED_INT,
    DT_UNSIGNED_INT,
    DT_SIGNED_LONG_LONG,
    DT_UNSIGNED_LONG_LONG,
    DT_FLOAT,
    DT_DOUBLE,
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
    "PAMI_MAX",
    "PAMI_MIN",
    "PAMI_SUM",
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
    "PAMI_SIGNED_CHAR",
    "PAMI_UNSIGNED_CHAR",
    "PAMI_SIGNED_SHORT",
    "PAMI_UNSIGNED_SHORT",
    "PAMI_SIGNED_INT",
    "PAMI_UNSIGNED_INT",
    "PAMI_SIGNED_LONG_LONG",
    "PAMI_UNSIGNED_LONG_LONG",
    "PAMI_FLOAT",
    "PAMI_DOUBLE",
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
    2, // PAMI_SIGNED_CHAR,
    2, // PAMI_UNSIGNED_CHAR,
    2, // PAMI_SIGNED_SHORT,
    2, // PAMI_UNSIGNED_SHORT,
    4, // PAMI_SIGNED_INT,
    4, // PAMI_UNSIGNED_INT,
    8, // PAMI_SIGNED_LONG_LONG,
    8, // PAMI_UNSIGNED_LONG_LONG,
    4, // PAMI_FLOAT,
    8, // PAMI_DOUBLE,
    8, // PAMI_LONG_DOUBLE,
    4, // PAMI_LOGICAL,
    8, // PAMI_SINGLE_COMPLEX,
    1, // PAMI_DOUBLE_COMPLEX
    8, // PAMI_LOC_2INT,
    6, // PAMI_LOC_SHORT_INT,
    8, // PAMI_LOC_FLOAT_INT,
    12, // PAMI_LOC_DOUBLE_INT,
    8,  // PAMI_LOC_2FLOAT,
    16, // PAMI_LOC_2DOUBLE,
  };

volatile unsigned       _g_barrier_active=0;
volatile unsigned       _g_allreduce_active=0;

void cb_barrier (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  TRACEV((stderr,"%s %p/%u, %u \n",__PRETTY_FUNCTION__,active, *active,_g_barrier_active));
  (*active)--;
}


static double timer()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}


void cb_allreduce (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  TRACEV((stderr,"%s %p/%u, %u \n",__PRETTY_FUNCTION__,active, *active,_g_allreduce_active));
  (*active)--;
}

void _barrier (pami_context_t context, pami_xfer_t *barrier)
{
  //static unsigned entryCount = 0;
  TRACEV((stderr,"%s<%u> %u\n",__PRETTY_FUNCTION__,++entryCount,_g_barrier_active));
  _g_barrier_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)barrier);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to issue barrier collective. result = %d\n", result);
    exit(1);
  }
  while (_g_barrier_active)
    result = PAMI_Context_advance (context, 1);
  TRACEV((stderr,"%s done<%u> active %u\n",__PRETTY_FUNCTION__,entryCount,_g_barrier_active));
}

void _allreduce (pami_context_t context, pami_xfer_t *allreduce)
{
  //static unsigned entryCount = 0;
  TRACEV((stderr,"%s<%u> %u\n",__PRETTY_FUNCTION__,++entryCount,_g_allreduce_active));
  _g_allreduce_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)allreduce);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to issue allreduce collective. result = %d\n", result);
    exit(1);
  }
  while (_g_allreduce_active)
    result = PAMI_Context_advance (context, 1);
  TRACEV((stderr,"%s done<%u> active %u\n",__PRETTY_FUNCTION__,entryCount,_g_allreduce_active));
}




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
void initialize_sndbuf (void *buf, int count, int op, int dt) {

  int i;
  if (op == PAMI_SUM && dt == PAMI_UNSIGNED_INT) {
    uint *ibuf = (uint *)  buf;
    for (i = 0; i < count; i++) {
      ibuf[i] = i;
    }
  }
}

int check_rcvbuf (void *buf, int count, int op, int dt, int nranks) {

  int i;
  if (op == PAMI_SUM && dt == PAMI_UNSIGNED_INT) {
    uint *rbuf = (uint *)  buf;
    for (i = 0; i < count; i++) {
      if (rbuf[i] != i * nranks)
        return -1;
    }
    TRACE((stderr,"Check Passes for count %d, op %d, dt %d\n", count, op, dt));
  }

  return 0;
}
#endif

int main(int argc, char*argv[])
{
  double tf,ti,usec;
  char sbuf[MAXBUFSIZE];
  char rbuf[MAXBUFSIZE];
  int  op, dt;
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  char          cl_string[] = "TEST";
  TRACE((stderr,"%s\n",__PRETTY_FUNCTION__));
  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }
  TRACE((stderr,"%s<%d>\n",__PRETTY_FUNCTION__,__LINE__));

        { size_t _n = 1; result = PAMI_Context_createv(client, NULL, 0, &context, _n); }
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
    return 1;
  }

  pami_configuration_t configuration;
  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;

#ifdef CHECK_DATA
  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t nranks  = configuration.value.intval;
#endif

  int    rank    = task_id;
  int i,j,root   = 0;
  TRACE((stderr,"%s<%d>\n",__PRETTY_FUNCTION__,__LINE__));

  pami_geometry_t  world_geometry;

  result = PAMI_Geometry_world (client, &world_geometry);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
    return 1;
  }
  TRACE((stderr,"%s<%d>\n",__PRETTY_FUNCTION__,__LINE__));

  int algorithm_type = 0;
  pami_algorithm_t *algorithm=NULL;
  int num_algorithm[2] = {0};
  result = PAMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       PAMI_XFER_BARRIER,
                                       num_algorithm);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query barrier algorithm. result = %d\n",
             result);
    return 1;
  }
  TRACE((stderr,"%s<%d>\n",__PRETTY_FUNCTION__,__LINE__));

  if (num_algorithm[0])
  {
    algorithm = (pami_algorithm_t*)
                malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
    result = PAMI_Geometry_query(context,
                                          world_geometry,
                                          PAMI_XFER_BARRIER,
                                          algorithm,
                                          (pami_metadata_t*)NULL,
                                          num_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

  }
  TRACE((stderr,"%s<%d>\n",__PRETTY_FUNCTION__,__LINE__));

  pami_algorithm_t *allreducealgorithm=NULL;
  pami_metadata_t *metas=NULL;
  int allreducenum_algorithm[2] = {0};
  result = PAMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       PAMI_XFER_ALLREDUCE,
                                       allreducenum_algorithm);

  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query allreduce algorithm. result = %d\n",
             result);
    return 1;
  }
  TRACE((stderr,"%s<%d>\n",__PRETTY_FUNCTION__,__LINE__));

  if (allreducenum_algorithm[0])
  {
    allreducealgorithm = (pami_algorithm_t*)
      malloc(sizeof(pami_algorithm_t) * allreducenum_algorithm[0]);
    metas = (pami_metadata_t*)
      malloc(sizeof(pami_metadata_t) * allreducenum_algorithm[0]);

    result = PAMI_Geometry_query(context,
                                          world_geometry,
                                          PAMI_XFER_ALLREDUCE,
                                          allreducealgorithm,
                                          metas,
                                          allreducenum_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable to query allreduce algorithm attributes."
              "result = %d\n", result);
      return 1;
    }
  }
  TRACE((stderr,"%s<%d>\n",__PRETTY_FUNCTION__,__LINE__));

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
  /// \todo These fail using core math on bgq.
  validTable[OP_LAND][DT_FLOAT]=0;
  validTable[OP_LOR][DT_FLOAT]=0;
  validTable[OP_LXOR][DT_FLOAT]=0;
  validTable[OP_BAND][DT_FLOAT]=0;
  validTable[OP_BOR][DT_FLOAT]=0;
  validTable[OP_BXOR][DT_FLOAT]=0;
  validTable[OP_LAND][DT_DOUBLE]=0;
  validTable[OP_LOR][DT_DOUBLE]=0;
  validTable[OP_LXOR][DT_DOUBLE]=0;
  validTable[OP_BOR][DT_DOUBLE]=0;
  validTable[OP_BXOR][DT_DOUBLE]=0;
  validTable[OP_MAXLOC][DT_LOC_SHORT_INT]=0;
  validTable[OP_MINLOC][DT_LOC_SHORT_INT]=0;
  validTable[OP_MAXLOC][DT_LOC_DOUBLE_INT]=0;
  validTable[OP_MINLOC][DT_LOC_DOUBLE_INT]=0;
#endif

#else
  for(i=0;i<op_count;i++)
    for(j=0;j<dt_count;j++)
      validTable[i][j]=0;

#ifdef ENABLE_MAMBO_WORKAROUNDS
  validTable[OP_BOR][DT_UNSIGNED_INT]=1;
#else
  validTable[OP_SUM][DT_UNSIGNED_INT]=1;
#endif

#endif
  TRACE((stderr,"%s<%d>\n",__PRETTY_FUNCTION__,__LINE__));

#if 1
  int nalg;
  for(nalg=0; nalg<allreducenum_algorithm[algorithm_type]; nalg++)
  {
    if (rank == root)
    {
      printf("# Allreduce Bandwidth Test -- root = %d protocol: %s\n", root,
             metas[nalg].name);
      printf("# Size(bytes)           cycles    bytes/sec    usec\n");
      printf("# -----------      -----------    -----------    ---------\n");
    }

    pami_xfer_t barrier;
    barrier.cb_done   = cb_barrier;
    barrier.cookie    = (void*)&_g_barrier_active;
    barrier.algorithm = algorithm[0];
    _barrier(context, &barrier);

    pami_xfer_t allreduce;
    allreduce.cb_done   = cb_allreduce;
    allreduce.cookie    = (void*)&_g_allreduce_active;
    allreduce.algorithm = allreducealgorithm[nalg];
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
          if(rank == root)
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
            initialize_sndbuf (sbuf, i, op_array[op], dt_array[dt]);
#endif
            _barrier(context, &barrier);
            ti = timer();
            for (j=0; j<niter; j++)
            {
              allreduce.cmd.xfer_allreduce.stypecount=dataSent;
              allreduce.cmd.xfer_allreduce.rtypecount=dataSent;
              allreduce.cmd.xfer_allreduce.dt=dt_array[dt];
              allreduce.cmd.xfer_allreduce.op=op_array[op];
              _allreduce(context, &allreduce);
            }
            tf = timer();
            _barrier(context, &barrier);

#ifdef CHECK_DATA
            int rc = check_rcvbuf (rbuf, i, op_array[op], dt_array[dt], nranks);
            //assert (rc == 0);
            if(rc) fprintf(stderr, "FAILED validation\n");
#endif

            usec = (tf - ti)/(double)niter;
            if (rank == root)
            {
#ifdef ENABLE_MAMBO_WORKAROUNDS
              fprintf(stderr,"  %11lld %16d \n",
                     dataSent,
                     niter);
#else
              printf("  %11lld %16d %14.1f %12.2f\n",
                     dataSent,
                     niter,
                     (double)1e6*(double)dataSent/(double)usec,
                     usec);
              fflush(stdout);
#endif
            }
          }
        }
      }
  }
#endif

  result = PAMI_Context_destroy (context);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy (client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }
  free(metas);
  free(algorithm);
  free(allreducealgorithm);
  return 0;
}
