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
#include "sys/xmi.h"


#define FULL_TEST
#define COUNT      65536
#define MAXBUFSIZE COUNT*16
#define NITERLAT   1000
#define NITERBW    10
#define CUTOFF     65536

xmi_op op_array[] =
  {
    XMI_MAX,
    XMI_MIN,
    XMI_SUM,
    XMI_PROD,
    XMI_LAND,
    XMI_LOR,
    XMI_LXOR,
    XMI_BAND,
    XMI_BOR,
    XMI_BXOR,
    XMI_MAXLOC,
    XMI_MINLOC,
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

xmi_dt dt_array[] =
  {
    XMI_SIGNED_CHAR,
    XMI_UNSIGNED_CHAR,
    XMI_SIGNED_SHORT,
    XMI_UNSIGNED_SHORT,
    XMI_SIGNED_INT,
    XMI_UNSIGNED_INT,
    XMI_SIGNED_LONG_LONG,
    XMI_UNSIGNED_LONG_LONG,
    XMI_FLOAT,
    XMI_DOUBLE,
    XMI_LONG_DOUBLE,
    XMI_LOGICAL,
    XMI_SINGLE_COMPLEX,
    XMI_DOUBLE_COMPLEX,
    XMI_LOC_2INT,
    XMI_LOC_SHORT_INT,
    XMI_LOC_FLOAT_INT,
    XMI_LOC_DOUBLE_INT,
    XMI_LOC_2FLOAT,
    XMI_LOC_2DOUBLE,
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
    "XMI_MAX",
    "XMI_MIN",
    "XMI_SUM",
    "XMI_PROD",
    "XMI_LAND",
    "XMI_LOR",
    "XMI_LXOR",
    "XMI_BAND",
    "XMI_BOR",
    "XMI_BXOR",
    "XMI_MAXLOC",
    "XMI_MINLOC"
  };


const char * dt_array_str[] =
  {
    "XMI_SIGNED_CHAR",
    "XMI_UNSIGNED_CHAR",
    "XMI_SIGNED_SHORT",
    "XMI_UNSIGNED_SHORT",
    "XMI_SIGNED_INT",
    "XMI_UNSIGNED_INT",
    "XMI_SIGNED_LONG_LONG",
    "XMI_UNSIGNED_LONG_LONG",
    "XMI_FLOAT",
    "XMI_DOUBLE",
    "XMI_LONG_DOUBLE",
    "XMI_LOGICAL",
    "XMI_SINGLE_COMPLEX",
    "XMI_DOUBLE_COMPLEX",
    "XMI_LOC_2INT",
    "XMI_LOC_SHORT_INT",
    "XMI_LOC_FLOAT_INT",
    "XMI_LOC_DOUBLE_INT",
    "XMI_LOC_2FLOAT",
    "XMI_LOC_2DOUBLE"
  };

unsigned elemsize_array[] =
  {
    2, // XMI_SIGNED_CHAR,
    2, // XMI_UNSIGNED_CHAR,
    2, // XMI_SIGNED_SHORT,
    2, // XMI_UNSIGNED_SHORT,
    4, // XMI_SIGNED_INT,
    4, // XMI_UNSIGNED_INT,
    8, // XMI_SIGNED_LONG_LONG,
    8, // XMI_UNSIGNED_LONG_LONG,
    4, // XMI_FLOAT,
    8, // XMI_DOUBLE,
    8, // XMI_LONG_DOUBLE,
    4, // XMI_LOGICAL,
    8, // XMI_SINGLE_COMPLEX,
    1, // XMI_DOUBLE_COMPLEX
    8, // XMI_LOC_2INT,
    6, // XMI_LOC_SHORT_INT,
    8, // XMI_LOC_FLOAT_INT,
    12, // XMI_LOC_DOUBLE_INT,
    8,  // XMI_LOC_2FLOAT,
    16, // XMI_LOC_2DOUBLE,
  };

volatile unsigned       _g_barrier_active;
volatile unsigned       _g_allreduce_active;

void cb_barrier (void *ctxt, void * clientdata, xmi_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}


static double timer()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}


void cb_allreduce (void *ctxt, void * clientdata, xmi_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void _barrier (xmi_context_t context, xmi_barrier_t *barrier)
{
  _g_barrier_active++;
  xmi_result_t result;
  result = XMI_Collective(context, (xmi_xfer_t*)barrier);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to issue barrier collective. result = %d\n", result);
    exit(1);
  }
  while (_g_barrier_active)
    result = XMI_Context_advance (context, 1);

}

void _allreduce (xmi_context_t context, xmi_allreduce_t *allreduce)
{
  _g_allreduce_active++;
  xmi_result_t result;
  result = XMI_Collective(context, (xmi_xfer_t*)allreduce);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to issue allreduce collective. result = %d\n", result);
    exit(1);
  }
  while (_g_allreduce_active)
    result = XMI_Context_advance (context, 1);

}




unsigned ** alloc2DContig(int nrows, int ncols)
{
  int i, j;
  unsigned **array;

  array        = (unsigned**)malloc(nrows*sizeof(unsigned*));
  array[0]     = (unsigned *)calloc(sizeof(unsigned), nrows*ncols);
  for(i = 1; i<nrows; i++)
    array[i]   = array[0]+i*ncols;

  return array;
}


int main(int argc, char*argv[])
{
  double tf,ti,usec;
  char sbuf[MAXBUFSIZE];
  char rbuf[MAXBUFSIZE];
  int  op, dt;
  xmi_client_t  client;
  xmi_context_t context;
  xmi_result_t  result = XMI_ERROR;
  char          cl_string[] = "TEST";
  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

	{ int _n = 1; result = XMI_Context_createv(client, NULL, 0, &context, &_n); }
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
    return 1;
  }

  xmi_configuration_t configuration;
  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query(client, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  int    rank    = task_id;
  int i,j,root   = 0;

  xmi_geometry_t  world_geometry;

  result = XMI_Geometry_world (context, &world_geometry);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
    return 1;
  }

  int algorithm_type = 0;
  xmi_algorithm_t *algorithm=NULL;
  int num_algorithm[2] = {0};
  result = XMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       XMI_XFER_BARRIER,
                                       num_algorithm);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query barrier algorithm. result = %d\n",
             result);
    return 1;
  }

  if (num_algorithm[0])
  {
    algorithm = (xmi_algorithm_t*)
                malloc(sizeof(xmi_algorithm_t) * num_algorithm[0]);
    result = XMI_Geometry_algorithms_info(context,
                                          world_geometry,
                                          XMI_XFER_BROADCAST,
                                          algorithm,
                                          (xmi_metadata_t*)NULL,
                                          algorithm_type,
                                          num_algorithm[0]);

  }

  xmi_algorithm_t *allreducealgorithm=NULL;
  xmi_metadata_t *metas=NULL;
  int allreducenum_algorithm[2] = {0};
  result = XMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       XMI_XFER_BROADCAST,
                                       allreducenum_algorithm);

  if (result != XMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query allreduce algorithm. result = %d\n",
             result);
    return 1;
  }

  if (allreducenum_algorithm[0])
  {
    allreducealgorithm = (xmi_algorithm_t*)
      malloc(sizeof(xmi_algorithm_t) * allreducenum_algorithm[0]);
    metas = (xmi_metadata_t*)
      malloc(sizeof(xmi_metadata_t) * allreducenum_algorithm[0]);

    result = XMI_Geometry_algorithms_info(context,
                                          world_geometry,
                                          XMI_XFER_ALLREDUCE,
                                          allreducealgorithm,
                                          metas,
                                          algorithm_type = 0,
                                          allreducenum_algorithm[0]);

    if (result != XMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable to query allreduce algorithm attributes."
              "result = %d\n", result);
      return 1;
    }
  }

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

  //  This one is failing using core math...we should find this bug.
  validTable[OP_BAND][DT_DOUBLE]=0;

  /* Now add back the minloc/maxloc stuff */
  for(i=OP_MAXLOC; i<=OP_MINLOC; i++)
    for(j=DT_LOC_2INT; j<=DT_LOC_2DOUBLE; j++)
      validTable[i][j]=1;
#else
  for(i=0;i<op_count;i++)
    for(j=0;j<dt_count;j++)
      validTable[i][j]=0;

  validTable[OP_SUM][DT_SIGNED_INT]=1;
#endif

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

    xmi_barrier_t barrier;
    barrier.xfer_type = XMI_XFER_BARRIER;
    barrier.cb_done   = cb_barrier;
    barrier.cookie    = (void*)&_g_barrier_active;
    barrier.geometry  = world_geometry;
    barrier.algorithm = algorithm[0];
    _barrier(context, &barrier);

    xmi_allreduce_t allreduce;
    allreduce.xfer_type = XMI_XFER_ALLREDUCE;
    allreduce.cb_done   = cb_allreduce;
    allreduce.cookie    = (void*)&_g_allreduce_active;
    allreduce.geometry  = world_geometry;
    allreduce.algorithm = allreducealgorithm[nalg];
    allreduce.sndbuf    = sbuf;
    allreduce.stype     = XMI_BYTE;
    allreduce.stypecount= 0;
    allreduce.rcvbuf    = rbuf;
    allreduce.rtype     = XMI_BYTE;
    allreduce.rtypecount= 0;



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

            _barrier(context, &barrier);
            ti = timer();
            for (j=0; j<niter; j++)
            {
              allreduce.stypecount=i;
              allreduce.rtypecount=i;
              allreduce.dt=dt_array[dt];
              allreduce.op=op_array[op];
              _allreduce(context, &allreduce);
            }
            tf = timer();
            _barrier(context, &barrier);

            usec = (tf - ti)/(double)niter;
            if (rank == root)
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
#endif

  result = XMI_Context_destroy (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }
  free(metas);
  free(algorithm);
  free(allreducealgorithm);
  return 0;
}
