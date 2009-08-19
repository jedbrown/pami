#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/xmi_collectives.h"


#define BUFSIZE 131072

void cb_barrier (void * clientdata);
void cb_scatterv (void * clientdata);

// Barrier Data
XMI_CollectiveProtocol_t _g_barrier;
volatile unsigned       _g_barrier_active;
XMI_CollectiveRequest_t  _g_barrier_request;
XMI_Callback_t _cb_barrier   = {(void (*)(void*,XMI_Error_t*))cb_barrier,
			       (void *) &_g_barrier_active };
XMI_Barrier_t  _xfer_barrier =
    {
	XMI_XFER_BARRIER,
	&_g_barrier,
	&_g_barrier_request,
	_cb_barrier,
	&XMI_World_Geometry
    };

// Scatterv
XMI_CollectiveProtocol_t _g_scatterv;
volatile unsigned       _g_scatterv_active;
XMI_CollectiveRequest_t  _g_scatterv_request;
XMI_Callback_t _cb_scatterv   = {(void (*)(void*,XMI_Error_t*))cb_scatterv,
			       (void *) &_g_scatterv_active };
XMI_Scatterv_t  _xfer_scatterv =
    {
	XMI_XFER_SCATTERV,
	&_g_scatterv,
	&_g_scatterv_request,
	_cb_scatterv,
	&XMI_World_Geometry,
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

XMI_Geometry_t *cb_geometry (int comm)
{
    if(comm == 0)
	return &XMI_World_Geometry;
    else
	assert(0);
}

void cb_barrier (void * clientdata)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_scatterv (void * clientdata)
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

void init__scattervs ()
{
  XMI_Scatterv_Configuration_t scatterv_config;
  scatterv_config.cfg_type    = XMI_CFG_SCATTERV;
  scatterv_config.protocol    = XMI_DEFAULT_SCATTERV_PROTOCOL;
  XMI_register(&_g_scatterv,
	      (XMI_CollectiveConfiguration_t*)&scatterv_config,
	      0);
  _g_scatterv_active = 0;
}

void _barrier ()
{
  _g_barrier_active++;
  XMI_Xfer (NULL, (XMI_Xfer_t*)&_xfer_barrier);
  while (_g_barrier_active)
      XMI_Poll();
}

void _scatterv (int         root,
		char      * src,
		char      * dst,
		size_t    * lengths)
{
    _g_scatterv_active++;
    _xfer_scatterv.root    = root;
    _xfer_scatterv.src     = src;
    _xfer_scatterv.dst     = dst;
    _xfer_scatterv.lengths = lengths;
    XMI_Xfer (NULL, (XMI_Xfer_t*)&_xfer_scatterv);
    while (_g_scatterv_active)
	XMI_Poll();
}



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  XMI_Collectives_initialize(&argc,&argv,cb_geometry);
  init__barriers();
  init__scattervs();
  int     rank    = XMI_Rank();
  int     sz      = XMI_Size();
  size_t *lengths = (size_t*)malloc(sz*sizeof(size_t));
  char   *buf     = (char*)malloc(BUFSIZE*sz);
  char   *rbuf    = (char*)malloc(BUFSIZE*sz);


  int i,j,root = 0;
#if 1
  if (rank == root)
      {
	  printf("# Scatterv Bandwidth Test -- root = %d\n", root);
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
		  _scatterv (root,buf,rbuf, &lengths[0]);
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
//		  fprintf(stderr,"scatterv: time=%f usec\n", usec/(double)niter);
		  fflush(stdout);
	      }
      }
#endif
  XMI_Collectives_finalize();
  return 0;
}
