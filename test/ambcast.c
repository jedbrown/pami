#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/hl_collectives.h"


#define BUFSIZE 524288
#define NITER   1000
void cb_barrier (void * clientdata);
void cb_broadcast (void * clientdata);
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
volatile unsigned       _g_total_broadcasts;
XMI_CollectiveRequest_t  _g_broadcast_request;
char                   *_g_recv_buffer;

XMI_Callback_t _cb_broadcast   = {(void (*)(void*,XMI_Error_t*))cb_broadcast,
			       (void *) &_g_broadcast_active };
hl_ambroadcast_t  _xfer_broadcast =
    {
	XMI_XFER_AMBROADCAST,
	&_g_broadcast,
	&_g_broadcast_request,
	_cb_broadcast,
	&XMI_World_Geometry,
	NULL,
	NULL,
	0
    };

void cb_ambcast_done (void * clientdata, XMI_Error_t*err)
{
  _g_total_broadcasts++;
  free(clientdata);
}

typedef struct ambcast_unexpected_t
{
  XMI_CollectiveRequest_t  req;
  void                   *buf;
};


void * cb_bcast_recv(unsigned           root,
		     unsigned           comm,
		     const unsigned     sndlen,
		     unsigned         * rcvlen,
		     char            ** rcvbuf,
		     XMI_Callback_t    * const cb_done)
{
  *rcvlen                        = sndlen;
  *rcvbuf                        = _g_recv_buffer;
  cb_done->function              = cb_ambcast_done;
  ambcast_unexpected_t  *reqdata = (ambcast_unexpected_t*)malloc(sizeof(ambcast_unexpected_t)+sndlen);
  reqdata->buf                   = &reqdata[1];
  cb_done->clientdata            = reqdata;
  return (void*)&reqdata->req;
}



XMI_Geometry_t *cb_geometry (int comm)
{
    if(comm == 0)
	return &XMI_World_Geometry;
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
  XMI_AMBroadcast_Configuration_t broadcast_config;
  broadcast_config.cfg_type    = XMI_CFG_AMBROADCAST;
  broadcast_config.cb_recv     = cb_bcast_recv;
  broadcast_config.protocol    = XMI_DEFAULT_AMBROADCAST_PROTOCOL;
  XMI_register(&_g_broadcast,
	      (XMI_CollectiveConfiguration_t*)&broadcast_config,
	      0);
  _g_broadcast_active = 0;
}

void _barrier ()
{
  _g_barrier_active++;
  XMI_Xfer (NULL, (hl_xfer_t*)&_xfer_barrier);
  while (_g_barrier_active)
      XMI_Poll();
}

void _broadcast (char            * src,
		 unsigned          bytes)
{
    _g_broadcast_active++;
    _xfer_broadcast.src   = src;
    _xfer_broadcast.bytes = bytes;
    XMI_Xfer (NULL, (hl_xfer_t*)&_xfer_broadcast);
}



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  char buf[BUFSIZE];
  char rbuf[BUFSIZE];
  _g_recv_buffer = rbuf;

  XMI_Collectives_initialize(&argc,&argv,cb_geometry);
  init__barriers();
  init__broadcasts();
  unsigned     rank = XMI_Rank();
  unsigned i,j,root = 0;
#if 1
  if (rank == root)
    {
      printf("# Broadcast Bandwidth Test -- root = %d\n", root);
      printf("# Size(bytes)           cycles    bytes/sec    usec\n");
      printf("# -----------      -----------    -----------    ---------\n");
    }
  _barrier ();
  for(i=1; i<=BUFSIZE; i*=2)
    {
      long long dataSent = i;
      unsigned     niter = NITER;

      if(rank==root)
	{

	  ti = timer();
	  for (j=0; j<niter; j++)
	    {
	      _broadcast (buf, i);
	    }
	  while (_g_broadcast_active)
	    XMI_Poll();

	  _barrier();
	  tf = timer();
	  usec = (tf - ti)/(double)niter;
	  printf("  %11lld %16lld %14.1f %12.2f\n",
		 dataSent,
		 0LL,
		 (double)1e6*(double)dataSent/(double)usec,
		 usec);
	  fflush(stdout);
	}
      else
	{
	  while(_g_total_broadcasts < niter)
	    XMI_Poll();
	  _g_total_broadcasts = 0;
	  _barrier();

	}
    }
  


#endif
  XMI_Collectives_finalize();
  return 0;
}
