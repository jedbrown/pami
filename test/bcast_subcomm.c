#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/hl_collectives.h"


#define BUFSIZE 4194304

void cb_barrier (void * clientdata);
void cb_broadcast (void * clientdata);
// Global Geometry Object
HL_Geometry_t           _g_geometry_top;
HL_Geometry_t           _g_geometry_bottom;
HL_Geometry_range_t     _g_range_top;
HL_Geometry_range_t     _g_range_bottom;
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

// Broadcast
HL_CollectiveProtocol_t _g_broadcast;
volatile unsigned       _g_broadcast_active;
HL_CollectiveRequest_t  _g_broadcast_request;
HL_Callback_t _cb_broadcast     = {(void (*)(void*,LL_Error_t*))cb_broadcast,
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

void init__geometry (HL_Geometry_t       *geometry,
		     HL_Geometry_range_t *range,
		     int                  lo, 
		     int                  hi)
{
    range->lo = lo;
    range->hi = hi;
    HL_Geometry_initialize (geometry,               // Geometry Object
                            0,                      // Global id
                            range,                  // List of rank slices
                            1);                     // Count of slices
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

void init__broadcasts ()
{
  HL_Broadcast_Configuration_t broadcast_config;
  broadcast_config.cfg_type    = HL_CFG_BROADCAST;
  broadcast_config.protocol    = HL_DEFAULT_BROADCAST_PROTOCOL;
  broadcast_config.cb_geometry = NULL;
  HL_register(&_g_broadcast,
	      (HL_CollectiveConfiguration_t*)&broadcast_config,
	      0);
  _g_broadcast_active = 0;
}

void _barrier (HL_Geometry_t *geometry)
{
  _g_barrier_active++;
  _xfer_barrier.geometry = geometry;
  HL_Xfer (NULL, (hl_xfer_t*)&_xfer_barrier);
  while (_g_barrier_active)
      HL_Poll();
}


void _broadcast (HL_Geometry_t   * geometry,
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
    HL_Xfer (NULL, (hl_xfer_t*)&_xfer_broadcast);
    while (_g_broadcast_active)
	HL_Poll();
}



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  char buf[BUFSIZE];
  char rbuf[BUFSIZE];  
  HL_Collectives_initialize(argc,argv);
  int rank = HL_Rank();
  int sz   = HL_Size();
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

  init__geometry(&_g_geometry_bottom,&_g_range_bottom,0, half-1);

  if(rank == 0)
      printf("Initializing Bottom Geometry\n");

  init__geometry(&_g_geometry_top,&_g_range_top,half, sz-1);
 
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
  _barrier(&HL_World_Geometry);

  HL_Geometry_t *geometries [] = {&_g_geometry_bottom, &_g_geometry_top};
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
					 0,
					 (double)1e6*(double)dataSent/(double)usec,
					 usec);
				  fflush(stdout);
			      }
		      }
	      }
	  _barrier(&HL_World_Geometry);
      }
  _barrier(&HL_World_Geometry);

  HL_Collectives_finalize();
  return 0;
}
