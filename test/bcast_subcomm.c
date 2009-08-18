#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/hl_collectives.h"
#include <assert.h>

#define BUFSIZE 1048576

void cb_barrier (void * clientdata);
void cb_broadcast (void * clientdata);
XMI_Geometry_t *cb_geometry (int comm);
// Global Geometry Object
XMI_Geometry_t           _g_geometry_top;
XMI_Geometry_t           _g_geometry_bottom;
XMI_Geometry_range_t     _g_range_top;
XMI_Geometry_range_t     _g_range_bottom;
XMI_mapIdToGeometry      _g_geometry_map = cb_geometry;
// Barrier Data

XMI_CollectiveProtocol_t _g_barrier;
volatile unsigned       _g_barrier_active;
XMI_CollectiveRequest_t  _g_barrier_request;
XMI_Callback_t _cb_barrier   = {(void (*)(void*,XMI_Error_t*))cb_barrier,
			       (void *) &_g_barrier_active };
hl_barrier_t  _xfer_barrier =
    {
	XMI_XFER_BARRIER,
	&_g_barrier,
	&_g_barrier_request,
	_cb_barrier,
	&XMI_World_Geometry
    };

// Broadcast
XMI_CollectiveProtocol_t _g_broadcast;
volatile unsigned       _g_broadcast_active;
XMI_CollectiveRequest_t  _g_broadcast_request;
XMI_Callback_t _cb_broadcast     = {(void (*)(void*,XMI_Error_t*))cb_broadcast,
			       (void *) &_g_broadcast_active };
hl_broadcast_t  _xfer_broadcast =
    {
	XMI_XFER_BROADCAST,
	&_g_broadcast,
	&_g_broadcast_request,
	_cb_broadcast,
	&XMI_World_Geometry,
	0,
	NULL,
	NULL,
	0
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
    else if(comm == 1)
	return &_g_geometry_bottom;
    else if(comm == 2)
	return &_g_geometry_top;
    else
	assert(0);

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

void init__geometry (XMI_Geometry_t       *geometry,
		     XMI_Geometry_range_t *range,
		     int                  lo,
		     int                  hi,
		     int                  id)
{
    range->lo = lo;
    range->hi = hi;
    XMI_Geometry_initialize (geometry,               // Geometry Object
                            id,                     // Global id
                            range,                  // List of rank slices
                            1);                     // Count of slices

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

void init__broadcasts ()
{
  XMI_Broadcast_Configuration_t broadcast_config;
  broadcast_config.cfg_type    = XMI_CFG_BROADCAST;
  broadcast_config.protocol    = XMI_DEFAULT_BROADCAST_PROTOCOL;
  XMI_register(&_g_broadcast,
	      (XMI_CollectiveConfiguration_t*)&broadcast_config,
	      0);
  _g_broadcast_active = 0;
}

void _barrier (XMI_Geometry_t *geometry)
{
  _g_barrier_active++;
  _xfer_barrier.geometry = geometry;
  XMI_Xfer (NULL, (hl_xfer_t*)&_xfer_barrier);
  while (_g_barrier_active)
      XMI_Poll();
}


void _broadcast (XMI_Geometry_t   * geometry,
		 int               root,
		 char            * src,
		 char            * dst,
		 unsigned          bytes)
{
    _g_broadcast_active++;
    _xfer_broadcast.geometry = geometry;
    _xfer_broadcast.root     = root;
    _xfer_broadcast.src      = src;
    _xfer_broadcast.dst      = dst;
    _xfer_broadcast.bytes    = bytes;
    XMI_Xfer (NULL, (hl_xfer_t*)&_xfer_broadcast);
    while (_g_broadcast_active)
	XMI_Poll();
}



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  char buf[BUFSIZE];
  char rbuf[BUFSIZE];
  XMI_Collectives_initialize(&argc,&argv,cb_geometry);
  int rank = XMI_Rank();
  int sz   = XMI_Size();
  int half = sz/2;
  int set[2];

  if(rank == 0)
      printf("Initializing Barriers\n");
  init__barriers();

  if(rank == 0)
      printf("Initializing Broadcast\n");
  init__broadcasts();


  if(rank == 0)
      printf("Initializing Top Geometry\n");

  init__geometry(&_g_geometry_bottom,&_g_range_bottom,0, half-1, 1);

  if(rank == 0)
      printf("Initializing Bottom Geometry\n");

  init__geometry(&_g_geometry_top,&_g_range_top,half, sz-1, 2);

  if(rank>=0 && rank<=half-1)
      {
	  set[0]=1;
	  set[1]=0;
      }
  else
      {
	  set[0]=0;
	  set[1]=1;
      }


  /*
     After creating a geometry, you have to barrier on
     a "parent" geometry that contains all the nodes
     in the geometry.
  */
  if(rank == 0)
      fprintf(stderr, "Testing World Geometry\n");
  _barrier(&XMI_World_Geometry);
  if(rank == 0)
      fprintf(stderr, "Done\n");
  _barrier(&XMI_World_Geometry);

  XMI_Geometry_t *geometries [] = {&_g_geometry_bottom, &_g_geometry_top};
  int            roots[]       = {0, half};



  int i,j,k;
  for(k=0; k<2; k++)
      {
	  if (rank == roots[k])
	      {
		  printf("# Broadcast Bandwidth Test -- root = %d\n", roots[k]);
		  printf("# Size(bytes)           cycles    bytes/sec    usec\n");
		  printf("# -----------      -----------    -----------    ---------\n");
	      }
	  if(set[k])
	      {
		  printf("Participant:  %d\n", rank);
		  fflush(stdout);
		  _barrier (geometries[k]);
		  for(i=1; i<=BUFSIZE; i*=2)
		      {
			  long long dataSent = i;
			  int          niter = 100;
			  _barrier (geometries[k]);
			  ti = timer();
			  for (j=0; j<niter; j++)
			      {
				  _broadcast (geometries[k],roots[k], buf,rbuf, i);
			      }
			  tf = timer();
			  _barrier (geometries[k]);
			  usec = (tf - ti)/(double)niter;
			  if (rank == roots[k])
			      {
				  printf("  %11lld %16lld %14.1f %12.2f\n",
					 dataSent,
					 0LL,
					 (double)1e6*(double)dataSent/(double)usec,
					 usec);
				  fflush(stdout);
			      }
		      }
	      }
	  _barrier(&XMI_World_Geometry);
      }
  _barrier(&XMI_World_Geometry);

  XMI_Collectives_finalize();
  return 0;
}
