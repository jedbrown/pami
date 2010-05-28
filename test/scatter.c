///
/// \file test/scatter.c
/// \brief Simple Barrier test
///

#include <pami.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define BUFSIZE 524288
volatile unsigned       _g_barrier_active;
volatile unsigned       _g_scatter_active;

void cb_barrier (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_scatter (void *ctxt, void * clientdata, pami_result_t err)
{
    int * active = (int *) clientdata;
    (*active)--;
}

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
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

void _scatter (pami_context_t context, pami_xfer_t *scatter)
{
  _g_scatter_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)scatter);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue scatter collective. result = %d\n", result);
      exit(1);
    }
  while (_g_scatter_active)
    result = PAMI_Context_advance (context, 1);

}

int main (int argc, char ** argv)
{
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  char          cl_string[] = "TEST";
  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
        return 1;
      }

        {  result = PAMI_Context_createv(client, NULL, 0, &context, 1); }
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

  pami_algorithm_t *scatteralgorithm=NULL;
  int scatternum_algorithm[2] = {0};
  result = PAMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       PAMI_XFER_SCATTER,
                                       scatternum_algorithm);

  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query scatter algorithm. result = %d\n",
             result);
    return 1;
  }

  if (scatternum_algorithm[0])
  {
    scatteralgorithm = (pami_algorithm_t*)
      malloc(sizeof(pami_algorithm_t) * scatternum_algorithm[0]);

    result = PAMI_Geometry_query(context,
                                          world_geometry,
                                          PAMI_XFER_SCATTER,
                                          scatteralgorithm,
                                          (pami_metadata_t*)NULL,
                                          scatternum_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);
  }

  double ti, tf, usec;
  char *buf    = (char*)malloc(BUFSIZE*sz);
  char *rbuf   = (char*)malloc(BUFSIZE*sz);
  pami_xfer_t barrier;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.algorithm = algorithm[0];
  _barrier(context, &barrier);


  size_t root = 0;
  if (task_id == root)
      {
        printf("# Scatter Bandwidth Test -- \n");
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

  pami_xfer_t scatter;
  scatter.cb_done    = cb_scatter;
  scatter.cookie     = (void*)&_g_scatter_active;
  scatter.algorithm  = scatteralgorithm[0];
  scatter.cmd.xfer_scatter.root       = root;
  scatter.cmd.xfer_scatter.sndbuf     = buf;
  scatter.cmd.xfer_scatter.stype      = PAMI_BYTE;
  scatter.cmd.xfer_scatter.stypecount = 0;
  scatter.cmd.xfer_scatter.rcvbuf     = rbuf;
  scatter.cmd.xfer_scatter.rtype      = PAMI_BYTE;
  scatter.cmd.xfer_scatter.rtypecount = 0;

  int i,j;
  for(i=1; i<=BUFSIZE; i*=2)
      {
        long long dataSent = i;
        int          niter = 100;
        _barrier(context, &barrier);
        ti = timer();
        for (j=0; j<niter; j++)
            {
              scatter.cmd.xfer_scatter.stypecount = i;
              scatter.cmd.xfer_scatter.rtypecount = i;
              _scatter (context, &scatter);
            }
        tf = timer();
        _barrier(context, &barrier);

        usec = (tf - ti)/(double)niter;
        if (task_id == root)
            {
              printf("  %11lld %16lld %14.1f %12.2f\n",
                     dataSent,
                     0LL,
                     (double)1e6*(double)dataSent/(double)usec,
                     usec);
              fflush(stdout);
            }
      }

  result = PAMI_Context_destroyv(&context, 1);
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
  free(scatteralgorithm);
  free(algorithm);
  return 0;
};
