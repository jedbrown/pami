//
/// \file test/allgatherv.c
///

#include <pami.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define BUFSIZE 524288
volatile unsigned       _g_barrier_active;
volatile unsigned       _g_allgatherv_active;

void cb_barrier (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_allgatherv (void *ctxt, void * clientdata, pami_result_t err)
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

void _allgatherv (pami_context_t context, pami_xfer_t *allgatherv)
{
  _g_allgatherv_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)allgatherv);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue allgatherv collective. result = %d\n", result);
      exit(1);
    }
  while (_g_allgatherv_active)
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
    result = PAMI_Geometry_algorithms_query(context,
                                          world_geometry,
                                          PAMI_XFER_BARRIER,
                                          algorithm,
                                          (pami_metadata_t*)NULL,
                                          num_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

  }

  pami_algorithm_t *allgathervalgorithm=NULL;
  int allgathervnum_algorithm[2] = {0};
  result = PAMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       PAMI_XFER_ALLGATHERV,
                                       allgathervnum_algorithm);

  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query allgatherv algorithm. result = %d\n",
             result);
    return 1;
  }

  if (allgathervnum_algorithm[0])
  {
    allgathervalgorithm = (pami_algorithm_t*)
      malloc(sizeof(pami_algorithm_t) * allgathervnum_algorithm[0]);

    result = PAMI_Geometry_algorithms_query(context,
                                          world_geometry,
                                          PAMI_XFER_ALLGATHERV,
                                          allgathervalgorithm,
                                          (pami_metadata_t*)NULL,
                                          allgathervnum_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);
  }


  double ti, tf, usec;
  char   *buf     = (char*)malloc(BUFSIZE*sz);
  char   *rbuf    = (char*)malloc(BUFSIZE*sz);
  size_t *lengths = (size_t*)malloc(sz*sizeof(size_t));
  size_t *displs  = (size_t*)malloc(sz*sizeof(size_t));
  pami_xfer_t barrier;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.algorithm = algorithm[0];
  _barrier(context, &barrier);


  if (task_id == 0)
      {
        printf("# Allgatherv Bandwidth Test -- root\n");
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

  pami_xfer_t allgatherv;
  allgatherv.cb_done    = cb_allgatherv;
  allgatherv.cookie     = (void*)&_g_allgatherv_active;
  allgatherv.algorithm  = allgathervalgorithm[0];
  allgatherv.cmd.xfer_allgatherv.sndbuf     = buf;
  allgatherv.cmd.xfer_allgatherv.stype      = PAMI_BYTE;
  allgatherv.cmd.xfer_allgatherv.stypecount = 0;
  allgatherv.cmd.xfer_allgatherv.rcvbuf     = rbuf;
  allgatherv.cmd.xfer_allgatherv.rtype      = PAMI_BYTE;
  allgatherv.cmd.xfer_allgatherv.rtypecounts= lengths;
  allgatherv.cmd.xfer_allgatherv.rdispls    = displs;


  unsigned i,j,k;
  for(i=1; i<=BUFSIZE; i*=2)
      {
        long long dataSent = i;
        unsigned  niter    = 100;
        for(k=0;k<sz;k++)lengths[k] = i;
        for(k=0;k<sz;k++)displs[k]  = 0;
        allgatherv.cmd.xfer_allgatherv.stypecount       = i;

        _barrier(context, &barrier);
        ti = timer();
        for (j=0; j<niter; j++)
            {
              _allgatherv (context, &allgatherv);
            }
        tf = timer();
        _barrier(context, &barrier);

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
  free(algorithm);
  free(allgathervalgorithm);
  return 0;
};
