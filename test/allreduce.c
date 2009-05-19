#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/hl_collectives.h"


#define COUNT 262144
#define MAXBUFSIZE 262144*16

HL_Op op_array[] =
    {
	LL_MAX,
	LL_MIN,
	LL_SUM,
	LL_PROD,
//	LL_LAND,
//	LL_LOR,
//	LL_LXOR,
	LL_BAND,
	LL_BOR,
	LL_BXOR
//	LL_MAXLOC,
//	LL_MINLOC,
    };
int op_count = 7;

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
	LL_DOUBLE
//	LL_LONG_DOUBLE,
//	LL_LOGICAL,
//	LL_SINGLE_COMPLEX,
//	LL_DOUBLE_COMPLEX,
	//LL_2INT,
	//LL_SHORT_INT,
	//LL_FLOAT_INT,
	//LL_DOUBLE_INT,
	//LL_2REAL,
	//LL_2DOUBLE_PRECISION,
    };
int dt_count = 10;


const char * op_array_str[] =
    {
	"LL_MAX",
	"LL_MIN",
	"LL_SUM",
	"LL_PROD",
//	"LL_LAND",
//	"LL_LOR",
//	"LL_LXOR",
	"LL_BAND",
	"LL_BOR",
	"LL_BXOR"
//	LL_MAXLOC,
//	LL_MINLOC,
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
//	"LL_LONG_DOUBLE",
//	"LL_LOGICAL",
//	"LL_SINGLE_COMPLEX",
//	"LL_DOUBLE_COMPLEX"
	//"LL_2INT",
	//"LL_SHORT_INT",
	//"LL_FLOAT_INT",
	//"LL_DOUBLE_INT",
	//"LL_2REAL",
	//"LL_2DOUBLE_PRECISION"
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
	1, // LL_DOUBLE_COMPLEX,
	8, // LL_2INT,
	6, // LL_SHORT_INT,
	8, // LL_FLOAT_INT,
	12, // LL_DOUBLE_INT,
	8,  // LL_2REAL,
	16, // LL_2DOUBLE_PRECISION,
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
  barrier_config.cb_geometry = NULL;
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
  allreduce_config.cb_geometry = NULL;
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



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  char buf[MAXBUFSIZE];
  char rbuf[MAXBUFSIZE];
  int  op, dt;

  HL_Collectives_initialize(argc,argv);
  init__barriers();
  int rank = HL_Rank();
  int i,j,root = 0;
#if 1
  if (rank == root)
      {
	  printf("# Allreduce Bandwidth Test -- root = %d\n", root);
	  printf("# Size(bytes)           cycles    bytes/sec    usec\n");
	  printf("# -----------      -----------    -----------    ---------\n");
      }
  for(dt=0; dt<dt_count; dt++)
      for(op=0; op<op_count; op++)
	  {
	      if(rank == root)
		  printf("Running Allreduce: %s, %s\n",dt_array_str[dt], op_array_str[op]);
	      for(i=1; i<=COUNT; i*=2)
		  {
		      long long dataSent = i*elemsize_array[dt];
		      int          niter = 100;
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
#endif
  HL_Collectives_finalize();
  return 0;
}
