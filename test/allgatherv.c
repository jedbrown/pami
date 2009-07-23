#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/hl_collectives.h"


#define BUFSIZE 131072

void cb_barrier (void * clientdata);
void cb_allgatherv (void * clientdata);

// Barrier Data
CM_CollectiveProtocol_t _g_barrier;
volatile unsigned       _g_barrier_active;
CM_CollectiveRequest_t  _g_barrier_request;
CM_Callback_t _cb_barrier   = {(void (*)(void*,LL_Error_t*))cb_barrier,
			       (void *) &_g_barrier_active };
hl_barrier_t  _xfer_barrier =
    {
	HL_XFER_BARRIER,
	&_g_barrier,
	&_g_barrier_request,
	_cb_barrier,
	&HL_World_Geometry
    };

// Allgatherv
CM_CollectiveProtocol_t _g_allgatherv;
volatile unsigned       _g_allgatherv_active;
CM_CollectiveRequest_t  _g_allgatherv_request;
CM_Callback_t _cb_allgatherv   = {(void (*)(void*,LL_Error_t*))cb_allgatherv,
			       (void *) &_g_allgatherv_active };
hl_allgatherv_t  _xfer_allgatherv =
    {
	HL_XFER_ALLGATHERV,
	&_g_allgatherv,
	&_g_allgatherv_request,
	_cb_allgatherv,
	&HL_World_Geometry,
	NULL,
	NULL,
	NULL,
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

void cb_allgatherv (void * clientdata)
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

void init__allgathervs ()
{
  HL_Allgatherv_Configuration_t allgatherv_config;
  allgatherv_config.cfg_type    = HL_CFG_ALLGATHERV;
  allgatherv_config.protocol    = HL_DEFAULT_ALLGATHERV_PROTOCOL;
  HL_register(&_g_allgatherv,
	      (HL_CollectiveConfiguration_t*)&allgatherv_config,
	      0);
  _g_allgatherv_active = 0;
}

void _barrier ()
{
  _g_barrier_active++;
  HL_Xfer (NULL, (hl_xfer_t*)&_xfer_barrier);
  while (_g_barrier_active)
      HL_Poll();
}

void _allgatherv (char      * src,
		  char      * dst,
		  size_t    * lengths)
{
    _g_allgatherv_active++;
    _xfer_allgatherv.src     = src;
    _xfer_allgatherv.dst     = dst;
    _xfer_allgatherv.lengths = lengths;
    HL_Xfer (NULL, (hl_xfer_t*)&_xfer_allgatherv);
    while (_g_allgatherv_active)
	HL_Poll();
}



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  HL_Collectives_initialize(&argc,&argv,cb_geometry);
  init__barriers();
  init__allgathervs();
  int     rank    = HL_Rank();
  int     sz      = HL_Size();
  size_t *lengths = (size_t*)malloc(sz*sizeof(size_t));
  char   *buf     = (char*)malloc(BUFSIZE*sz);
  char   *rbuf    = (char*)malloc(BUFSIZE*sz);


  int i,j,root = 0;
#if 1
  if (rank == root)
      {
	  printf("# Allgatherv Bandwidth Test -- root = %d\n", root);
	  printf("# Size(bytes)           cycles    bytes/sec    usec\n");
	  printf("# -----------      -----------    -----------    ---------\n");
      }

  for(i=1; i<=BUFSIZE; i*=2)
      {
	  long long dataSent = i;
	  int          niter = 100;
	  int              k = 0;
	  for(k=0;k<sz;k++)
	      lengths[k] = i;
	  _barrier ();
	  ti = timer();
	  for (j=0; j<niter; j++)
	      {
		  _allgatherv (buf,rbuf, &lengths[0]);
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
//		  fprintf(stderr,"allgatherv: time=%f usec\n", usec/(double)niter);
		  fflush(stdout);
	      }
      }
#endif
  HL_Collectives_finalize();
  return 0;
}
