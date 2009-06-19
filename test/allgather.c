#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include "../interface/hl_collectives.h"


#define BUFSIZE 131072

void cb_barrier (void * clientdata);
void cb_allgather (void * clientdata);

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

// Allgather
HL_CollectiveProtocol_t _g_allgather;
volatile unsigned       _g_allgather_active;
HL_CollectiveRequest_t  _g_allgather_request;

HL_Callback_t _cb_allgather   = {(void (*)(void*,LL_Error_t*))cb_allgather,
			       (void *) &_g_allgather_active };
hl_allgather_t  _xfer_allgather =
    {
	HL_XFER_ALLGATHER,
	&_g_allgather,
	&_g_allgather_request,
	_cb_allgather,
	&HL_World_Geometry,
	NULL,
	NULL,
	0
    };

HL_Geometry_t *cb_geometry (int comm)
{
    if(comm == 0)
	return &HL_World_Geometry;
    else
	assert(0);
}

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

void cb_allgather (void * clientdata)
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

void init__allgathers ()
{
  HL_Allgather_Configuration_t allgather_config;
  allgather_config.cfg_type    = HL_CFG_ALLGATHER;
  allgather_config.protocol    = HL_DEFAULT_ALLGATHER_PROTOCOL;
  HL_register(&_g_allgather,
	      (HL_CollectiveConfiguration_t*)&allgather_config,
	      0);
  _g_allgather_active = 0;
}

void _barrier ()
{
  _g_barrier_active++;
  HL_Xfer (NULL, (hl_xfer_t*)&_xfer_barrier);
  while (_g_barrier_active)
      HL_Poll();
}

void _allgather (char            * src,
		 char            * dst,
		 unsigned          bytes)
{
    _g_allgather_active++;
    _xfer_allgather.src   = src;
    _xfer_allgather.dst   = dst;
    _xfer_allgather.bytes = bytes;
    HL_Xfer (NULL, (hl_xfer_t*)&_xfer_allgather);
    while (_g_allgather_active)
	HL_Poll();
}



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  HL_Collectives_initialize(&argc,&argv,cb_geometry);
  init__barriers();
  init__allgathers();
  int rank  = HL_Rank();
  int sz    = HL_Size();
  char *buf = (char*)malloc(BUFSIZE*sz);
  char *rbuf= (char*)malloc(BUFSIZE*sz);
  int i,j,root = 0;
#if 1
  if (rank == root)
      {
	  printf("# Allgather Bandwidth Test -- root = %d\n", root);
	  printf("# Size(bytes)           cycles    bytes/sec    usec\n");
	  printf("# -----------      -----------    -----------    ---------\n");
      }

  for(i=1; i<=BUFSIZE; i*=2)
      {
	  long long dataSent = i;
	  int          niter = 100;
	  _barrier ();
	  ti = timer();
	  for (j=0; j<niter; j++)
	      {
		_allgather (buf,rbuf, i);
	      }
	  tf = timer();
	  _barrier ();

	  usec = (tf - ti)/(double)niter;
	  if (rank == root)
	      {

		  printf("  %11lld %16lld %14.1f %12.2f\n",
			 dataSent,
			 0LL,
			 (double)1e6*(double)dataSent/(double)usec,
			 usec);
//		  fprintf(stderr,"allgather: time=%f usec\n", usec/(double)niter);
		  fflush(stdout);
	      }
      }
#endif
  HL_Collectives_finalize();
  return 0;
}
