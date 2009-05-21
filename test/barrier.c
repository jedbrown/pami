#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/hl_collectives.h"

HL_CollectiveProtocol_t _g_barrier;
volatile unsigned       _g_barrier_active;
HL_CollectiveRequest_t  _g_barrier_request;

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


HL_Callback_t _cb = {(void (*)(void*,LL_Error_t*))cb_barrier, (void *) &_g_barrier_active };
hl_barrier_t  _xfer =
    {
	HL_XFER_BARRIER,
	&_g_barrier,
	&_g_barrier_request,
	_cb,
	&HL_World_Geometry
    };

HL_Geometry_t *cb_geometry (int comm)
{
    if(comm == 0)
	return &HL_World_Geometry;
    else
	assert(0);
}

void _barrier ()
{
  _g_barrier_active++;
  HL_Xfer (NULL, (hl_xfer_t*)&_xfer);
  while (_g_barrier_active)
      HL_Poll();
}


int main(int argc, char*argv[])
{
  double tf,ti,usec;  
  HL_Collectives_initialize(argc,argv,cb_geometry);
  int r = HL_Rank();
  init__barriers();

  if(!r)
      fprintf(stderr, "Test Barrier 1\n");
  _barrier();
  if(!r)
      fprintf(stderr, "Test Barrier 2\n");
  _barrier();
  _barrier();

  if(!r)
      fprintf(stderr, "Test Barrier Performance\n");
  int niter=10000;
  _barrier();
  ti=timer();
  for(int i=0; i<niter; i++)
      _barrier();
  tf=timer();
  usec = tf - ti;
  
  if(!r)
      fprintf(stderr,"barrier: time=%f usec\n", usec/(double)niter);

  HL_Collectives_finalize();
  return 0;
}
