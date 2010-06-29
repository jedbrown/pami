///
/// \file test/barrier.c
/// \brief Simple Barrier test
///

#include <pami.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define NITER 100
#include <assert.h>


/* Docs17: the done callback */
volatile unsigned       _g_barrier_active = 0;
void cb_barrier (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}
/* Docs18:  the done callback */

static double timer()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

/* Docs15:  The _barrier code to post the barrier */
void _barrier (pami_context_t context, pami_xfer_t *barrier)
{
    static unsigned barrierCount = 0;
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
  barrierCount++;
}
/* Docs16:  The _barrier code to post the barrier */



int main (int argc, char ** argv)
{
  /* Docs01:  Client and Context Create */
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
  
  result = PAMI_Context_createv(client, NULL, 0, &context, 1);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
      return 1;
    }
  /* Docs02:  Client and Context Create Done */
  

  /* Docs03:  Query Task ID */
  pami_configuration_t configuration;
  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
      return 1;
    }
  size_t task_id = configuration.value.intval;
  pami_geometry_t  world_geometry;
  /* Docs04:  Done Query Task ID */

  /* Docs05:  Get a reference to the geometry world object */
  result = PAMI_Geometry_world (client, &world_geometry);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
      return 1;
    }
  /* Docs06:  We now have a geometry world object */

  /* Docs07:  Query the world object for the number of barrier algorithms */
  int algorithm_type = 0;
  pami_algorithm_t *algorithm = NULL;
  int num_algorithm[2] = {0};
  result = PAMI_Geometry_algorithms_num(context,
                                        world_geometry,
                                        PAMI_XFER_BARRIER,
                                        num_algorithm);

  if (result != PAMI_SUCCESS || num_algorithm[0]==0)
    {
      fprintf (stderr,
               "Error. Unable to query barrier algorithm, or no algorithms available result = %d\n",
               result);
      return 1;
    }
  /* Docs08:  Query the world object for the number of barrier algorithms */


  /* Docs09:  Query the world object to generate the algorithm object */
  pami_metadata_t *metas = NULL;
  algorithm = (pami_algorithm_t*)
    malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
  metas = (pami_metadata_t*)
    malloc(sizeof(pami_metadata_t) * num_algorithm[0]);
  result = PAMI_Geometry_algorithms_query(context,
                                          world_geometry,
                                          PAMI_XFER_BARRIER,
                                          algorithm,
                                          metas,
                                          num_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get query algorithm. result = %d\n", result);
      return 1;
    }
  /* Docs10:  Query the world object to generate the algorithm object */


  /* Docs11:  Issue the test barrier */
  pami_xfer_t barrier;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*) & _g_barrier_active;
  barrier.algorithm = algorithm[0];
  if (!task_id)
    fprintf(stderr, "Test Default Barrier(%s)\n", metas[0].name);
  _barrier(context, &barrier);
  if (!task_id)
    fprintf(stderr, "Barrier Done(%s)\n", metas[0].name);
  /* Docs12:  Issue the test barrier */

  int algo;
  for (algo = 0; algo < num_algorithm[algorithm_type]; algo++)
    {
      double ti, tf, usec;
      barrier.algorithm = algorithm[algo];

      if (!task_id)
        {
          fprintf(stderr, "Test Barrier(%s) Correctness (%d of %d algorithms)\n",
                  metas[algo].name, algo + 1, num_algorithm[algorithm_type]);
          ti = timer();
          _barrier(context, &barrier);
          tf = timer();
          usec = tf - ti;

          if (usec < 1800000.0 || usec > 2200000.0)
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

      int niter = NITER;
      _barrier(context, &barrier);

      ti = timer();
      int i;

      for (i = 0; i < niter; i++)
        _barrier(context, &barrier);

      tf = timer();
      usec = tf - ti;

      if (!task_id)
        fprintf(stderr, "Test Barrier(%s) Performance barrier: time=%f usec\n",
                metas[algo].name, usec / (double)niter);
    }


  /* Docs13: Clean up the context */  
  result = PAMI_Context_destroyv(&context, 1);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
      return 1;
    }

  result = PAMI_Client_destroy(&client);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
      return 1;
    }
  /* Docs14: Clean up the context */  
  
  free(metas);
  return 0;
};
