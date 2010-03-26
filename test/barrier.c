///
/// \file test/barrier.c
/// \brief Simple Barrier test
///

#include "sys/pami.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define TRACE(x) //fprintf x

#include <assert.h>
#define TEST_abort()                       abort()
#define TEST_abortf(fmt...)                { fprintf(stderr, __FILE__ ":%d: \n", __LINE__); fprintf(stderr, fmt); abort(); }
#define TEST_assert(expr)                assert(expr)
#define TEST_assertf(expr, fmt...)       { if (!(expr)) TEST_abortf(fmt); }

volatile unsigned       _g_barrier_active;

void cb_barrier (void *ctxt, void * clientdata, pami_result_t err)
{
  TRACE((stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__));
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
  static unsigned count = 10000;
  TRACE((stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__));
  _g_barrier_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)barrier);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue barrier collective. result = %d\n", result);
      exit(1);
    }
  while (_g_barrier_active && count--)
    result = PAMI_Context_advance (context, 1);
  TEST_assertf(count,"%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
  count = 10000;
}



int main (int argc, char ** argv)
{
  TRACE((stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__));
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  char          cl_string[] = "TEST";
  result = PAMI_Client_initialize (cl_string, &client);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
      return 1;
    }
  TRACE((stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__));

        { size_t _n = 1; result = PAMI_Context_createv(client, NULL, 0, &context, _n); }
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
      return 1;
    }
  TRACE((stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__));


  pami_configuration_t configuration;
  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
      return 1;
    }
  size_t task_id = configuration.value.intval;
  TRACE((stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__));


  pami_geometry_t  world_geometry;

  result = PAMI_Geometry_world (client, &world_geometry);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
      return 1;
    }
  TRACE((stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__));

  int algorithm_type = 0;
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

  TRACE((stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__));
  pami_metadata_t *metas=NULL;
  if (num_algorithm[0])
  {
    algorithm = (pami_algorithm_t*)
                malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
    metas = (pami_metadata_t*)
      malloc(sizeof(pami_metadata_t) * num_algorithm[0]);

    result = PAMI_Geometry_algorithms_info(context,
                                          world_geometry,
                                          PAMI_XFER_BARRIER,
                                          algorithm,
                                          metas,
                                          num_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

  }

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get query algorithm. result = %d\n", result);
      return 1;
    }
  TRACE((stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__));

  pami_xfer_t barrier;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.algorithm = algorithm[0];

  if(!task_id)
    fprintf(stderr, "Test Barrier 1: %s\n", metas[0].name);
  _barrier(context, &barrier);
  if(!task_id)
    fprintf(stderr, "Test Barrier 2, then correctness\n");
  barrier.algorithm = algorithm[0];
  _barrier(context, &barrier);
  _barrier(context, &barrier);

  int algo;
  for(algo=0; algo<num_algorithm[algorithm_type]; algo++)
      {
        double ti, tf, usec;
        barrier.algorithm = algorithm[algo];

        fprintf(stderr, "Test Barrier(%s) Correctness %d of %d algorithms\n",
                metas[algo].name,algo+1, num_algorithm[algorithm_type]);
        if(!task_id)
            {
              ti=timer();
              _barrier(context, &barrier);
              tf=timer();
              usec = tf - ti;

              if(usec < 1800000.0 || usec > 2200000.0)
                fprintf(stderr, "Barrier error: usec=%f want between %f and %f!\n",
                        usec, 1800000.0, 2200000.0);
              else
                fprintf(stderr, "Barrier correct!\n");
            }
        else
            {
              sleep(2);
              _barrier(context, &barrier);
            }

        if(!task_id)
          fprintf(stderr, "Test Barrier(%s) Performance %d of %d algorithms\n",
                  metas[algo].name,algo+1, num_algorithm[algorithm_type]);
        int niter=10000;
        _barrier(context, &barrier);

        ti=timer();
        int i;
        for(i=0; i<niter; i++)
          _barrier(context, &barrier);

        tf=timer();
        usec = tf - ti;

        if(!task_id)
          fprintf(stderr,"barrier: time=%f usec\n", usec/(double)niter);
      }

  result = PAMI_Context_destroy (context);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
      return 1;
    }

  result = PAMI_Client_finalize (client);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
      return 1;
    }

  free(metas);
  return 0;
};
