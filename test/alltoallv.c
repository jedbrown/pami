/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/alltoallv.c
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
  size_t k;
  for ( k = 0; k < sndlens[r]; k++ )
    {
      sbuf[ sdispls[r] + k ] = ((r + k) & 0xff);
      rbuf[ rdispls[r] + k ] = 0xff;
    }
}


void check_bufs(size_t sz, size_t myrank)
{
  size_t r, k;
  for ( r = 0; r < sz; r++ )
    for ( k = 0; k < rcvlens[r]; k++ )
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


void cb_barrier (xmi_client_t client, size_t ctxt, void * clientdata, xmi_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_alltoallv (xmi_client_t client, size_t ctxt, void * clientdata, xmi_result_t res)
{
  int * active = (int *) clientdata;
  TRACE(("%d: cb_alltoallv active:%d(%p)\n",XMI_Rank(),*active,active));
  (*active)--;
}


void _barrier (xmi_client_t client, size_t context, xmi_barrier_t *barrier)
{
  _g_barrier_active++;
  xmi_result_t result;
  result = XMI_Collective(client, context, (xmi_xfer_t*)barrier);
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue barrier collective. result = %d\n", result);
      exit(1);
    }
  while (_g_barrier_active)
    result = XMI_Context_advance (client, context, 1);

}


void _alltoallv (xmi_client_t client, size_t    context,
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
  result = XMI_Collective (client, context, (xmi_xfer_t*)xfer);
  while (_g_alltoallv_active)
    result = XMI_Context_advance (client, context, 1);
}


int main(int argc, char*argv[])
{
  xmi_client_t  client;
  xmi_result_t  result = XMI_ERROR;
  char          cl_string[] = "TEST";
  double ti, tf, usec;
  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
        return 1;
      }

  result = XMI_Context_create(client, NULL, 0, 1);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
        return 1;
      }


  xmi_configuration_t configuration;
  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query (client, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t task_id = configuration.value.intval;

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query (client, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t sz = configuration.value.intval;


  xmi_geometry_t  world_geometry;

  result = XMI_Geometry_world (client, 0, &world_geometry);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
        return 1;
      }

  int algorithm_type = 0;
  xmi_algorithm_t *algorithm;
  int num_algorithm[2] = {0};
  result = XMI_Geometry_algorithms_num(client, 0,
                                       world_geometry,
                                       XMI_XFER_BARRIER,
                                       num_algorithm);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query barrier algorithm. result = %d\n",
             result);
    return 1;
  }

  if (num_algorithm[0])
  {
    algorithm = (xmi_algorithm_t*)
                malloc(sizeof(xmi_algorithm_t) * num_algorithm[0]);
    result = XMI_Geometry_algorithms_info(client, 0,
                                          world_geometry,
                                          XMI_XFER_BARRIER,
                                          algorithm,
                                          (xmi_metadata_t*)NULL,
                                          algorithm_type,
                                          num_algorithm[0]);

  }


  xmi_algorithm_t *alltoallvalgorithm;
  int             alltoallvnum_algorithm[2];
  result = XMI_Geometry_algorithms_num(client, 0,
                                       world_geometry,
                                       XMI_XFER_ALLTOALLV,
                                       alltoallvnum_algorithm);

  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to query alltoallv algorithm. result = %d\n", result);
    return 1;
  }

  if (num_algorithm[0])
  {
    algorithm = (xmi_algorithm_t*)
                malloc(sizeof(xmi_algorithm_t) * num_algorithm[0]);
    result = XMI_Geometry_algorithms_info(client, 0,
                                          world_geometry,
                                          XMI_XFER_ALLTOALLV,
                                          alltoallvalgorithm,
                                          (xmi_metadata_t*)NULL,
                                          algorithm_type,
                                          alltoallvnum_algorithm[0]);

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

	  _barrier (client, 0, &barrier);
	  ti = timer();

	  for (j=0; j<niter; j++)
	      {
		_alltoallv ( client, 0,
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

	  _barrier (client, 0, &barrier);

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

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
        return 1;
      }

  free(algorithm);
  free(alltoallvalgorithm);
  return 0;
}
