/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/alltoall.c
 * \brief ???
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../interface/xmi_collectives.h"


//#define TRACE(x) printf x;fflush(stdout);
#define TRACE(x)


#define MAX_COMM_SIZE 16
#define MSGSIZE       4096


#define BUFSIZE        (MSGSIZE * MAX_COMM_SIZE)


//#define INIT_BUFS(r)
#define INIT_BUFS(r) init_bufs(r)

//#define CHCK_BUFS
#define CHCK_BUFS    check_bufs()



char sbuf[BUFSIZE];
char rbuf[BUFSIZE];

unsigned sndlens[ MAX_COMM_SIZE ];
unsigned sdispls[ MAX_COMM_SIZE ];
unsigned rcvlens[ MAX_COMM_SIZE ];
unsigned rdispls[ MAX_COMM_SIZE ];



void init_bufs(int r)
{
  for ( unsigned k = 0; k < sndlens[r]; k++ )
    {
      sbuf[ sdispls[r] + k ] = ((r + k) & 0xff);
      rbuf[ rdispls[r] + k ] = 0xff;
    }
}


void check_bufs()
{
  unsigned myrank = XMI_Rank();
  for ( int r = 0; r < XMI_Size(); r++ )
    for ( unsigned k = 0; k < rcvlens[r]; k++ )
      {
	if ( rbuf[ rdispls[r] + k ] != (char)((myrank + k) & 0xff) )
	  {
	    printf("%d: (E) rbuf[%d]:%02x instead of %02x (r:%d)\n",
		   XMI_Rank(),
		   rdispls[r] + k,
		   rbuf[ rdispls[r] + k ],
		   ((r + k) & 0xff),
		   r );
	    exit(1);
	  }
      }
}


XMI_CollectiveProtocol_t _g_alltoall;
volatile unsigned       _g_alltoall_active;
XMI_CollectiveRequest_t  _g_alltoall_request;

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}


// ------ Barrier

void cb_barrier (void * clientdata);
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

void cb_barrier (void * clientdata)
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

void _barrier ()
{
  _g_barrier_active++;
  XMI_Xfer (NULL, (XMI_Xfer_t*)&_xfer_barrier);
  while (_g_barrier_active)
      XMI_Poll();
}

// ------- Alltoall

void cb_alltoall (void * clientdata)
{
  int * active = (int *) clientdata;
  TRACE(("%d: cb_alltoall active:%d(%p)\n",XMI_Rank(),*active,active));
  (*active)--;
}


void init__alltoall ()
{
  XMI_Alltoall_Configuration_t alltoall_config;
  alltoall_config.cfg_type    = XMI_CFG_ALLTOALL;
  alltoall_config.protocol    = XMI_DEFAULT_ALLTOALL_PROTOCOL;
  int rc = XMI_register(&_g_alltoall,
		       (XMI_CollectiveConfiguration_t*)&alltoall_config,
		       0);
  assert ( rc == 0 );
  _g_alltoall_active = 0;
  TRACE(("%d: init alltoall active:%d(%p)\n",XMI_Rank(),_g_alltoall_active,&_g_alltoall_active));
}

XMI_Callback_t _cb = {(void (*)(void*,XMI_Error_t*))cb_alltoall, (void *) &_g_alltoall_active };
XMI_Alltoall_t  _xfer_alltoall =
    {
	XMI_XFER_ALLTOALL,
	&_g_alltoall,
	&_g_alltoall_request,
	_cb,
	&XMI_World_Geometry,
	NULL,// char                     * sndbuf;
        NULL,//unsigned                 * sndlens;
        NULL,//unsigned                 * sdispls;
        NULL,//char                     * rcvbuf;
        NULL,//unsigned                 * rcvlens;
        NULL,//unsigned                 * rdispls;
        NULL,//unsigned                 * sndcounters;
        NULL,//unsigned                 * rcvcounters;
    };

XMI_Geometry_t *cb_geometry (int comm)
{
    if(comm == 0)
	return &XMI_World_Geometry;
    else
	assert(0);
}


void _alltoall (
		char       * sndbuf,
		unsigned   * sndlens,
		unsigned   * sdispls,
		char       * rcvbuf,
		unsigned   * rcvlens,
		unsigned   * rdispls )
{
  _g_alltoall_active++;
  _xfer_alltoall.sndbuf  = sndbuf;
  _xfer_alltoall.sndlens = sndlens;
  _xfer_alltoall.sdispls = sdispls;
  _xfer_alltoall.rcvbuf  = rcvbuf;
  _xfer_alltoall.rcvlens = rcvlens;
  _xfer_alltoall.rdispls = rdispls;

  XMI_Xfer (NULL, (XMI_Xfer_t*)&_xfer_alltoall);
  while (_g_alltoall_active)
      XMI_Poll();
}


int main(int argc, char*argv[])
{
  double tf,ti,usec;

  XMI_Collectives_initialize(&argc,&argv,cb_geometry);
  init__alltoall();

  int rank = XMI_Rank();
  int size = XMI_Size();

  assert ( size < MAX_COMM_SIZE );

  TRACE(("%d: sbuf:%p rbuf:%p\n",rank,sbuf,rbuf));

  int i,j;
#if 1
  if (rank == 0)
      {
	printf("# Alltoall Bandwidth Test(size:%d)\n",size);
	  printf("# Size(bytes)           cycles    bytes/sec      usec\n");
	  printf("# -----------      -----------    -----------    ---------\n");
      }


  for(i=1; i<=MSGSIZE; i*=2)
      {
	  long long dataSent = i;

	  int niter = (i < 1024 ? 100 : 10);

	  for ( j = 0; j < size; j++ )
	    {
	      sndlens[j] = rcvlens[j] = i;
	      sdispls[j] = rdispls[j] = i * j;

	      INIT_BUFS( j );
	    }

	  _barrier ();
	  ti = timer();

	  for (j=0; j<niter; j++)
	      {
		_alltoall ( sbuf, sndlens, sdispls, rbuf, rcvlens, rdispls );
	      }
	  tf = timer();

	  CHCK_BUFS;

	  _barrier ();

	  usec = (tf - ti)/(double)niter;
	  if (rank == 0)
	      {

		  printf("  %11lld %16lld %14.1f %12.2f\n",
			 dataSent,
			 0LL,
			 (double)1e6*(double)dataSent/(double)usec,
			 usec);
		  fflush(stdout);
	      }
      }
#endif
  XMI_Collectives_finalize();
  return 0;
}
