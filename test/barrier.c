///
/// \file test/barrier.c
/// \brief Simple Barrier test
///

#include "sys/xmi.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

volatile unsigned       _g_barrier_active;

void cb_barrier (void *ctxt, void * clientdata, xmi_result_t err)
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



int main (int argc, char ** argv)
{
  xmi_client_t  client;
  xmi_context_t context;
  xmi_result_t  result = XMI_ERROR;
  char          cl_string[] = "TEST";
  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
      return 1;
    }

	{ int _n = 1; result = XMI_Context_createv(client, NULL, 0, &context, &_n); }
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


  xmi_geometry_t  world_geometry;

  result = XMI_Geometry_world (context, &world_geometry);
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
      return 1;
    }
  
  int algorithm_type = 0;
  xmi_algorithm_t *algorithm;
  int num_algorithm[2] = {0};
  result = XMI_Geometry_algorithms_num(context,
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
    result = XMI_Geometry_algorithms_info(context,
                                          world_geometry,
                                          XMI_XFER_BROADCAST,
                                          algorithm,
                                          (xmi_metadata_t*)NULL,
                                          algorithm_type,
                                          num_algorithm[0]);

  }
  
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get query algorithm. result = %d\n", result);
      return 1;
    }


  xmi_barrier_t barrier;
  barrier.xfer_type = XMI_XFER_BARRIER;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.geometry  = world_geometry;
  barrier.algorithm = algorithm[0];

  if(!task_id)
    fprintf(stderr, "Test Barrier 1\n");
  _barrier(context, &barrier);
  if(!task_id)
    fprintf(stderr, "Test Barrier 2, then correctness\n");
  _barrier(context, &barrier);
  _barrier(context, &barrier);

  int algo;
  for(algo=0; algo<num_algorithm[algorithm_type]; algo++)
      {
        double ti, tf, usec;
        barrier.algorithm = algorithm[algo];
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
          fprintf(stderr, "Test Barrier Performance %d of %d algorithms\n",
                  algo+1, num_algorithm[algorithm_type]);
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
};
