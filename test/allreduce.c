#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include "../interface/hl_collectives.h"


#define COUNT      1048576
#define MAXBUFSIZE COUNT*16
#define NITERLAT   1000
#define NITERBW    10
#define CUTOFF     65536

HL_Op op_array[] =
    {
	LL_MAX,
	LL_MIN,
	LL_SUM,
	LL_PROD,
	LL_LAND,
	LL_LOR,
	LL_LXOR,
	LL_BAND,
	LL_BOR,
	LL_BXOR,
	LL_MAXLOC,
	LL_MINLOC,
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

HL_Dt dt_array[] =
    {
	LL_SIGNED_CHAR,
	LL_UNSIGNED_CHAR,
	LL_SIGNED_SHORT,
	LL_UNSIGNED_SHORT,
	LL_SIGNED_INT,
	LL_UNSIGNED_INT,
	LL_SIGNED_LONG_LONG,
	LL_UNSIGNED_LONG_LONG,
	LL_FLOAT,
	LL_DOUBLE,
 	LL_LONG_DOUBLE,
	LL_LOGICAL,
	LL_SINGLE_COMPLEX,
	LL_DOUBLE_COMPLEX,
	LL_LOC_2INT,
	LL_LOC_SHORT_INT,
	LL_LOC_FLOAT_INT,
	LL_LOC_DOUBLE_INT,
	LL_LOC_2FLOAT,
	LL_LOC_2DOUBLE,
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
	"LL_MAX",
	"LL_MIN",
	"LL_SUM",
	"LL_PROD",
	"LL_LAND",
	"LL_LOR",
	"LL_LXOR",
	"LL_BAND",
	"LL_BOR",
	"LL_BXOR",
	"LL_MAXLOC",
	"LL_MINLOC"
    };


const char * dt_array_str[] =
    {
	"LL_SIGNED_CHAR",
	"LL_UNSIGNED_CHAR",
	"LL_SIGNED_SHORT",
	"LL_UNSIGNED_SHORT",
	"LL_SIGNED_INT",
	"LL_UNSIGNED_INT",
	"LL_SIGNED_LONG_LONG",
	"LL_UNSIGNED_LONG_LONG",
	"LL_FLOAT",
	"LL_DOUBLE",
	"LL_LONG_DOUBLE",
	"LL_LOGICAL",
	"LL_SINGLE_COMPLEX",
	"LL_DOUBLE_COMPLEX",
	"LL_LOC_2INT",
	"LL_LOC_SHORT_INT",
	"LL_LOC_FLOAT_INT",
	"LL_LOC_DOUBLE_INT",
	"LL_LOC_2FLOAT",
	"LL_LOC_2DOUBLE"
    };

unsigned elemsize_array[] =
    {
	2, // LL_SIGNED_CHAR,
	2, // LL_UNSIGNED_CHAR,
	2, // LL_SIGNED_SHORT,
	2, // LL_UNSIGNED_SHORT,
	4, // LL_SIGNED_INT,
	4, // LL_UNSIGNED_INT,
	8, // LL_SIGNED_LONG_LONG,
	8, // LL_UNSIGNED_LONG_LONG,
	4, // LL_FLOAT,
	8, // LL_DOUBLE,
	8, // LL_LONG_DOUBLE,
	4, // LL_LOGICAL,
	8, // LL_SINGLE_COMPLEX,
	1, // LL_DOUBLE_COMPLEX
	8, // LL_LOC_2INT,
	6, // LL_LOC_SHORT_INT,
	8, // LL_LOC_FLOAT_INT,
	12, // LL_LOC_DOUBLE_INT,
	8,  // LL_LOC_2FLOAT,
	16, // LL_LOC_2DOUBLE,
    };

void cb_barrier (void * clientdata);
void cb_allreduce (void * clientdata);
// Barrier Data
HL_CollectiveProtocol_t _g_barrier;
volatile unsigned       _g_barrier_active;
HL_CollectiveRequest_t  _g_barrier_request;
HL_Callback_t _cb_barrier   = {(void (*)(void*,LL_Error_t*))cb_barrier,
			       (void *) &_g_barrier_active };
hl_barrier_t  _xfer_barrier =
    {
	HL_XFER_BARRIER,
	&_g_barrier,
	&_g_barrier_request,
	_cb_barrier,
	&HL_World_Geometry
    };

// Allreduce
HL_CollectiveProtocol_t _g_allreduce;
volatile unsigned       _g_allreduce_active;
HL_CollectiveRequest_t  _g_allreduce_request;
HL_Callback_t _cb_allreduce   = {(void (*)(void*,LL_Error_t*))cb_allreduce,
			       (void *) &_g_allreduce_active };
hl_allreduce_t  _xfer_allreduce =
    {
	HL_XFER_ALLREDUCE,
	&_g_allreduce,
	&_g_allreduce_request,
	_cb_allreduce,
	&HL_World_Geometry,
	NULL,
	NULL,
	0,
	(HL_Dt)-1,
	(HL_Op)-1
    };

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

HL_Geometry_t *cb_geometry (int comm)
{
    if(comm == 0)
	return &HL_World_Geometry;
    else
	assert(0);
}

void cb_barrier (void * clientdata)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_allreduce (void * clientdata)
{
    int * active = (int *) clientdata;
    (*active)--;
}

void init__barriers ()
{
  HL_Barrier_Configuration_t barrier_config;
  barrier_config.cfg_type    = HL_CFG_BARRIER;
  barrier_config.protocol    = HL_DEFAULT_BARRIER_PROTOCOL;
  HL_register(&_g_barrier,
	      (HL_CollectiveConfiguration_t*)&barrier_config,
	      0);
  _g_barrier_active = 0;
}

void init__allreduces ()
{
  HL_Allreduce_Configuration_t allreduce_config;
  allreduce_config.cfg_type    = HL_CFG_ALLREDUCE;
  allreduce_config.protocol    = HL_DEFAULT_ALLREDUCE_PROTOCOL;
  HL_register(&_g_allreduce,
	      (HL_CollectiveConfiguration_t*)&allreduce_config,
	      0);
  _g_allreduce_active = 0;
}

void _barrier ()
{
  _g_barrier_active++;
  HL_Xfer (NULL, (hl_xfer_t*)&_xfer_barrier);
  while (_g_barrier_active)
      HL_Poll();
}

void _allreduce (char            * src,
		 char            * dst,
		 unsigned          count,
		 LL_Dt             dt,
		 LL_Op             op)
{
    _g_allreduce_active++;
    _xfer_allreduce.src   = src;
    _xfer_allreduce.dst   = dst;
    _xfer_allreduce.count = count;
    _xfer_allreduce.dt    = dt;
    _xfer_allreduce.op    = op;
    HL_Xfer (NULL, (hl_xfer_t*)&_xfer_allreduce);
    while (_g_allreduce_active)
	HL_Poll();
}


bool ** alloc2DContig(int nrows, int ncols)
{
    int i, j;
    bool **array;

    array        = (bool**)malloc(nrows*sizeof(bool*));
    array[0]     = (bool *)calloc(sizeof(bool), nrows*ncols);
    for(i = 1; i<nrows; i++)
	array[i]   = array[0]+i*ncols;

    return array;
}


int main(int argc, char*argv[])
{
  double tf,ti,usec;
  char buf[MAXBUFSIZE];
  char rbuf[MAXBUFSIZE];
  int  op, dt;

  HL_Collectives_initialize(argc,argv,cb_geometry);
  init__barriers();
  init__allreduces();
  int rank = HL_Rank();
  int i,j,root = 0;


  bool** validTable=
  alloc2DContig(op_count,dt_count);
  for(i=0;i<op_count;i++)
      for(j=0;j<dt_count;j++)
	  validTable[i][j]=true;

  /* Not testing minloc/maxloc/logical,etc */
  for(i=OP_MINLOC,j=0; j<DT_COUNT;j++)validTable[i][j]=false;
  for(i=OP_MAXLOC,j=0; j<DT_COUNT;j++)validTable[i][j]=false;
  for(i=0,j=DT_LOGICAL; i<OP_COUNT;i++)validTable[i][j]=false;
  for(i=0,j=DT_SINGLE_COMPLEX; i<OP_COUNT;i++)validTable[i][j]=false;
  for(i=0,j=DT_LONG_DOUBLE; i<OP_COUNT;i++)validTable[i][j]=false;
  for(i=0,j=DT_DOUBLE_COMPLEX; i<OP_COUNT;i++)validTable[i][j]=false;
  for(i=0,j=DT_LOC_2INT; i<OP_COUNT;i++)validTable[i][j]=false;
  for(i=0,j=DT_LOC_SHORT_INT; i<OP_COUNT;i++)validTable[i][j]=false;
  for(i=0,j=DT_LOC_FLOAT_INT; i<OP_COUNT;i++)validTable[i][j]=false;
  for(i=0,j=DT_LOC_DOUBLE_INT; i<OP_COUNT;i++)validTable[i][j]=false;
  for(i=0,j=DT_LOC_2FLOAT; i<OP_COUNT;i++)validTable[i][j]=false;
  for(i=0,j=DT_LOC_2DOUBLE; i<OP_COUNT;i++)validTable[i][j]=false;


  validTable[OP_MAX][DT_DOUBLE_COMPLEX]=false;
  validTable[OP_MIN][DT_DOUBLE_COMPLEX]=false;
  validTable[OP_PROD][DT_DOUBLE_COMPLEX]=false;

  /* Now add back the minloc/maxloc stuff */
  for(i=OP_MAXLOC; i<=OP_MINLOC; i++)
      for(j=DT_LOC_2INT; j<=DT_LOC_2DOUBLE; j++)
	  validTable[i][j]=true;


#if 1
  if (rank == root)
      {
	  printf("# Allreduce Bandwidth Test -- root = %d\n", root);
	  printf("# Size(bytes)           cycles    bytes/sec    usec\n");
	  printf("# -----------      -----------    -----------    ---------\n");
      }
  _barrier();

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
			      _barrier ();
			      ti = timer();
			      for (j=0; j<niter; j++)
				  {
				      _allreduce (buf,rbuf,i,dt_array[dt],op_array[op]);
				  }
			      tf = timer();
			      _barrier ();

			      usec = (tf - ti)/(double)niter;
			      if (rank == root)
				  {
				      printf("  %11lld %16lld %14.1f %12.2f\n",
					     dataSent,
					     0,
					     (double)1e6*(double)dataSent/(double)usec,
					     usec);
				      fflush(stdout);
				  }
			  }
		  }
	  }
#endif
  HL_Collectives_finalize();
  return 0;
}
