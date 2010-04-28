/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#include <pami.h>


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
            printf("%zu: (E) rbuf[%zu]:%02x instead of %02zx (r:%zu)\n",
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


void cb_barrier (pami_context_t ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_alltoallv (pami_context_t ctxt, void * clientdata, pami_result_t res)
{
  int * active = (int *) clientdata;
  TRACE(("%d: cb_alltoallv active:%d(%p)\n",PAMI_Rank(),*active,active));
  (*active)--;
}


void _barrier (pami_context_t context, pami_xfer_t *barrier)
{
  _g_barrier_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)barrier);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue barrier collective. result = %d\n", result);
      exit(1);
    }
  while (_g_barrier_active)
    result = PAMI_Context_advance (context, 1);

}


void _alltoallv (pami_context_t    context,
                 pami_xfer_t *xfer,
                 char            *sndbuf,
                 size_t          *sndlens,
                 size_t          *sdispls,
                 char            *rcvbuf,
                 size_t          *rcvlens,
                 size_t          *rdispls )
{
  pami_result_t result;
  _g_alltoallv_active++;
  xfer->cmd.xfer_alltoallv.sndbuf        = sndbuf;
  xfer->cmd.xfer_alltoallv.stype         = PAMI_BYTE;
  xfer->cmd.xfer_alltoallv.stypecounts   = sndlens;
  xfer->cmd.xfer_alltoallv.sdispls       = sdispls;
  xfer->cmd.xfer_alltoallv.rcvbuf        = rcvbuf;
  xfer->cmd.xfer_alltoallv.rtype         = PAMI_BYTE;
  xfer->cmd.xfer_alltoallv.rtypecounts   = rcvlens;
  xfer->cmd.xfer_alltoallv.rdispls       = rdispls;
  result = PAMI_Collective (NULL, (pami_xfer_t*)xfer);
  while (_g_alltoallv_active)
    result = PAMI_Context_advance (context, 1);
}


int main(int argc, char*argv[])
{
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  char          cl_string[] = "TEST";
  double ti, tf, usec;
  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
        return 1;
      }

        { size_t _n = 1; result = PAMI_Context_createv(client, NULL, 0, &context, _n); }
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
        return 1;
      }


  pami_configuration_t configuration;
  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t task_id = configuration.value.intval;

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t sz = configuration.value.intval;


  pami_geometry_t  world_geometry;

  result = PAMI_Geometry_world (client, &world_geometry);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
        return 1;
      }

  pami_algorithm_t *algorithm=NULL;
  int num_algorithm[2] = {0};
  result = PAMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       PAMI_XFER_BARRIER,
                                       num_algorithm);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query barrier algorithm. result = %d\n",
             result);
    return 1;
  }

  if (num_algorithm[0])
  {
    algorithm = (pami_algorithm_t*)
                malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
    result = PAMI_Geometry_query(context,
                                          world_geometry,
                                          PAMI_XFER_BARRIER,
                                          algorithm,
                                          (pami_metadata_t*)NULL,
                                          num_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

  }


  pami_algorithm_t *alltoallvalgorithm=NULL;
  int             alltoallvnum_algorithm[2];
  result = PAMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       PAMI_XFER_ALLTOALLV,
                                       alltoallvnum_algorithm);

  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to query alltoallv algorithm. result = %d\n", result);
    return 1;
  }

  if (num_algorithm[0])
  {
    alltoallvalgorithm = (pami_algorithm_t*)
      malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
    result = PAMI_Geometry_query(context,
                                          world_geometry,
                                          PAMI_XFER_ALLTOALLV,
                                          alltoallvalgorithm,
                                          (pami_metadata_t*)NULL,
                                          alltoallvnum_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

  }

  assert ( sz < MAX_COMM_SIZE );

  pami_xfer_t barrier;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.algorithm = algorithm[0];

  pami_xfer_t alltoallv;
  alltoallv.cb_done    = cb_alltoallv;
  alltoallv.cookie     = (void*)&_g_alltoallv_active;
  alltoallv.algorithm  = alltoallvalgorithm[0];




  size_t i,j;
  if (task_id == 0)
      {
        printf("# Alltoallv Bandwidth Test(size:%zu) %p\n",sz, cb_alltoallv);
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

  result = PAMI_Context_destroy (context);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
        return 1;
      }

  result = PAMI_Client_destroy (client);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
        return 1;
      }

  free(algorithm);
  free(alltoallvalgorithm);
  return 0;
}
