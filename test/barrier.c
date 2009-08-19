#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/xmi_collectives.h"

XMI_CollectiveProtocol_t _g_barrier;
volatile unsigned       _g_barrier_active;
XMI_CollectiveRequest_t  _g_barrier_request;

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

void cb_barrier (void * clientdata, XMI_Error_t *err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void init__barriers ()
{
  XMI_Barrier_Configuration_t barrier_config;
  barrier_config.cfg_type    = XMI_CFG_BARRIER;
  barrier_config.protocol    = XMI_DEFAULT_BARRIER_PROTOCOL;
  XMI_register(&_g_barrier,
	      (XMI_CollectiveConfiguration_t*)&barrier_config,
	      0);
  _g_barrier_active = 0;
}


XMI_Callback_t _cb = { cb_barrier, (void *)&_g_barrier_active };
XMI_Barrier_t  _xfer =
    {
	XMI_XFER_BARRIER,
	&_g_barrier,
	&_g_barrier_request,
	_cb,
	&XMI_World_Geometry
    };

XMI_Geometry_t *cb_geometry (int comm)
{
    if(comm == 0)
	return &XMI_World_Geometry;
    else
	assert(0);
}

void _barrier ()
{
  _g_barrier_active++;
  XMI_Xfer (NULL, (XMI_Xfer_t*)&_xfer);
  while (_g_barrier_active)
      XMI_Poll();
}


int main(int argc, char*argv[])
{
  double tf,ti,usec;  
  XMI_Collectives_initialize(&argc,&argv,cb_geometry);
  int r = XMI_Rank();
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

  XMI_Collectives_finalize();
  return 0;
}
