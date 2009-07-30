#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/hl_collectives.h"


#define BUFSIZE 524288

void cb_barrier (void * clientdata);
void cb_broadcast (void * clientdata);
// Barrier Data
CM_CollectiveProtocol_t _g_barrier;
volatile unsigned       _g_barrier_active;
CM_CollectiveRequest_t  _g_barrier_request;
CM_Callback_t _cb_barrier   = {(void (*)(void*,CM_Error_t*))cb_barrier,
			       (void *) &_g_barrier_active };
hl_barrier_t  _xfer_barrier =
    {
	HL_XFER_BARRIER,
	&_g_barrier,
	&_g_barrier_request,
	_cb_barrier,
	&HL_World_Geometry
    };

// Broadcast
CM_CollectiveProtocol_t _g_broadcast;
volatile unsigned       _g_broadcast_active;
CM_CollectiveRequest_t  _g_broadcast_request;
CM_Callback_t _cb_broadcast   = {(void (*)(void*,CM_Error_t*))cb_broadcast,
			       (void *) &_g_broadcast_active };
hl_broadcast_t  _xfer_broadcast =
    {
	HL_XFER_BROADCAST,
	&_g_broadcast,
	&_g_broadcast_request,
	_cb_broadcast,
	&HL_World_Geometry,
	0,
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

void cb_broadcast (void * clientdata)
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

void init__broadcasts ()
{
  HL_Broadcast_Configuration_t broadcast_config;
  broadcast_config.cfg_type    = HL_CFG_BROADCAST;
  broadcast_config.protocol    = HL_DEFAULT_BROADCAST_PROTOCOL;
  HL_register(&_g_broadcast,
	      (HL_CollectiveConfiguration_t*)&broadcast_config,
	      0);
  _g_broadcast_active = 0;
}

void _barrier ()
{
  _g_barrier_active++;
  HL_Xfer (NULL, (hl_xfer_t*)&_xfer_barrier);
  while (_g_barrier_active)
      HL_Poll();
}

void _broadcast (int               root,
		 char            * src,
		 char            * dst,
		 unsigned          bytes)
{
    _g_broadcast_active++;
    _xfer_broadcast.root  = root;
    _xfer_broadcast.src   = src;
    _xfer_broadcast.dst   = dst;
    _xfer_broadcast.bytes = bytes;
    HL_Xfer (NULL, (hl_xfer_t*)&_xfer_broadcast);
    while (_g_broadcast_active)
	HL_Poll();
}



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  char buf[BUFSIZE];
  char rbuf[BUFSIZE];

  HL_Collectives_initialize(&argc,&argv,cb_geometry);
  init__barriers();
  init__broadcasts();
  int rank = HL_Rank();
  int i,j,root = 0;
#if 1
  if (rank == root)
      {
	  printf("# Broadcast Bandwidth Test -- root = %d\n", root);
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
		  _broadcast (root, buf,rbuf, i);
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
//		  fprintf(stderr,"broadcast: time=%f usec\n", usec/(double)niter);
		  fflush(stdout);
	      }
      }
#endif
  HL_Collectives_finalize();
  return 0;
}
