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
#include "sys/xmi.h"


//#define TRACE(x) printf x;fflush(stdout);
#define TRACE(x)


#define MAX_COMM_SIZE 16
#define MSGSIZE       4096
#define BUFSIZE       (MSGSIZE * MAX_COMM_SIZE)


//#define INIT_BUFS(r)
#define INIT_BUFS(r) init_bufs(r)

//#define CHCK_BUFS
#define CHCK_BUFS(s,r)    check_bufs(s,r)

volatile unsigned       _g_barrier_active;
volatile unsigned       _g_alltoallv_active;

char sbuf[BUFSIZE];
char rbuf[BUFSIZE];
size_t sndlens[ MAX_COMM_SIZE ];
size_t sdispls[ MAX_COMM_SIZE ];
size_t rcvlens[ MAX_COMM_SIZE ];
size_t rdispls[ MAX_COMM_SIZE ];

void init_bufs(size_t r)
{
  for ( size_t k = 0; k < sndlens[r]; k++ )
    {
      sbuf[ sdispls[r] + k ] = ((r + k) & 0xff);
      rbuf[ rdispls[r] + k ] = 0xff;
    }
}


void check_bufs(size_t sz, size_t myrank)
{
  for ( size_t r = 0; r < sz; r++ )
    for ( size_t k = 0; k < rcvlens[r]; k++ )
      {
	if ( rbuf[ rdispls[r] + k ] != (char)((myrank + k) & 0xff) )
	  {
	    printf("%zd: (E) rbuf[%zd]:%02x instead of %02zx (r:%zd)\n",
                   myrank,
		   rdispls[r] + k,
		   rbuf[ rdispls[r] + k ],
		   ((r + k) & 0xff),
		   r );
	    exit(1);
	  }
      }
}

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}


void cb_barrier (xmi_context_t ctxt, void * clientdata, xmi_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_alltoallv (xmi_context_t ctxt, void * clientdata, xmi_result_t res)
{
  int * active = (int *) clientdata;
  TRACE(("%d: cb_alltoallv active:%d(%p)\n",XMI_Rank(),*active,active));
  (*active)--;
}


void _barrier (xmi_context_t context, xmi_barrier_t *barrier)
{
  _g_barrier_active++;
  xmi_result_t result;
  result = XMI_Collective(context, (xmi_xfer_t*)barrier);
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue barrier collective. result = %d\n", result);
      exit(1);
    }
  while (_g_barrier_active)
    result = XMI_Context_advance (context, 1);

}


void _alltoallv (xmi_context_t    context,
                 xmi_alltoallv_t *xfer,
                 char            *sndbuf,
                 size_t          *sndlens,
                 size_t          *sdispls,
                 char            *rcvbuf,
                 size_t          *rcvlens,
                 size_t          *rdispls )
{
  xmi_result_t result;
  _g_alltoallv_active++;
  xfer->sndbuf        = sndbuf;
  xfer->stype         = XMI_BYTE;
  xfer->stypecounts   = sndlens;
  xfer->sdispls       = sdispls;
  xfer->rcvbuf        = rcvbuf;
  xfer->rtype         = XMI_BYTE;
  xfer->rtypecounts   = rcvlens;
  xfer->rdispls       = rdispls;
  result = XMI_Collective (NULL, (xmi_xfer_t*)xfer);
  while (_g_alltoallv_active)
    result = XMI_Context_advance (context, 1);
}


int main(int argc, char*argv[])
{
  xmi_client_t  client;
  xmi_context_t context;
  xmi_result_t  result = XMI_ERROR;
  char          cl_string[] = "TEST";
  double ti, tf, usec;
  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
        return 1;
      }

  result = XMI_Context_create (client, NULL, 0, &context);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
        return 1;
      }


  xmi_configuration_t configuration;
  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query (context, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t task_id = configuration.value.intval;

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query (context, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t sz = configuration.value.intval;


  xmi_geometry_t  world_geometry;

  result = XMI_Geometry_world (context, &world_geometry);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
        return 1;
      }

  xmi_algorithm_t algorithm[1];
  int             num_algorithm = 1;
  result = XMI_Geometry_algorithm(context,
				  XMI_XFER_BARRIER,
				  world_geometry,
				  &algorithm[0],
				  &num_algorithm);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query barrier algorithm. result = %d\n", result);
        return 1;
      }

  xmi_algorithm_t alltoallvalgorithm[1];
  int             alltoallvnum_algorithm = 1;
  result = XMI_Geometry_algorithm(context,
				  XMI_XFER_ALLTOALLV,
				  world_geometry,
				  &alltoallvalgorithm[0],
				  &alltoallvnum_algorithm);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query alltoallv algorithm. result = %d\n", result);
        return 1;
      }

  assert ( sz < MAX_COMM_SIZE );

  xmi_barrier_t barrier;
  barrier.xfer_type = XMI_XFER_BARRIER;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.geometry  = world_geometry;
  barrier.algorithm = algorithm[0];

  xmi_alltoallv_t alltoallv;
  alltoallv.xfer_type  = XMI_XFER_ALLTOALLV;
  alltoallv.cb_done    = cb_alltoallv;
  alltoallv.cookie     = (void*)&_g_alltoallv_active;
  alltoallv.geometry   = world_geometry;
  alltoallv.algorithm  = alltoallvalgorithm[0];



  
  size_t i,j;
  if (task_id == 0)
      {
	printf("# Alltoallv Bandwidth Test(size:%zd) %p\n",sz, cb_alltoallv);
	  printf("# Size(bytes)           cycles    bytes/sec      usec\n");
	  printf("# -----------      -----------    -----------    ---------\n");
      }


  for(i=1; i<=MSGSIZE; i*=2)
      {
	  long long dataSent = i;
	  size_t niter = (i < 1024 ? 100 : 10);
	  for ( j = 0; j < sz; j++ )
	    {
	      sndlens[j] = rcvlens[j] = i;
	      sdispls[j] = rdispls[j] = i * j;
	      INIT_BUFS( j );
	    }

	  _barrier (context, &barrier);
	  ti = timer();

	  for (j=0; j<niter; j++)
	      {
		_alltoallv ( context,
                             &alltoallv,
                             sbuf,
                             sndlens,
                             sdispls,
                             rbuf,
                             rcvlens,
                             rdispls );
	      }
	  tf = timer();

	  CHCK_BUFS(sz, task_id);

	  _barrier (context, &barrier);

	  usec = (tf - ti)/(double)niter;
	  if (task_id == 0)
	      {

		  printf("  %11lld %16lld %14.1f %12.2f\n",
			 dataSent,
			 0LL,
			 (double)1e6*(double)dataSent/(double)usec,
			 usec);
		  fflush(stdout);
	      }
      }

  result = XMI_Context_destroy (context);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to destroy xmi context. result = %d\n", result);
        return 1;
      }
  
  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
        return 1;
      }
  return 0;
}
