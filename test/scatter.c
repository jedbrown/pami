///
/// \file test/scatter.c
/// \brief Simple Barrier test
///

#include "sys/xmi.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define BUFSIZE 524288
volatile unsigned       _g_barrier_active;
volatile unsigned       _g_scatter_active;

void cb_barrier (void *ctxt, void * clientdata, xmi_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_scatter (void *ctxt, void * clientdata, xmi_result_t err)
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

void _barrier (xmi_context_t context, xmi_xfer_t *barrier)
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

void _scatter (xmi_context_t context, xmi_xfer_t *scatter)
{
  _g_scatter_active++;
  xmi_result_t result;
  result = XMI_Collective(context, (xmi_xfer_t*)scatter);
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue scatter collective. result = %d\n", result);
      exit(1);
    }
  while (_g_scatter_active)
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

	{ size_t _n = 1; result = XMI_Context_createv(client, NULL, 0, &context, _n); }
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
        return 1;
      }


  xmi_configuration_t configuration;
  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query(client, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t task_id = configuration.value.intval;


  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query(client, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t sz = configuration.value.intval;

  xmi_geometry_t  world_geometry;

  result = XMI_Geometry_world (client, &world_geometry);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
        return 1;
      }

  xmi_algorithm_t *algorithm=NULL;
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
                                          XMI_XFER_BARRIER,
                                          algorithm,
                                          (xmi_metadata_t*)NULL,
                                          num_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

  }

  xmi_algorithm_t *scatteralgorithm=NULL;
  int scatternum_algorithm[2] = {0};
  result = XMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       XMI_XFER_SCATTER,
                                       scatternum_algorithm);

  if (result != XMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query scatter algorithm. result = %d\n",
             result);
    return 1;
  }

  if (scatternum_algorithm[0])
  {
    scatteralgorithm = (xmi_algorithm_t*)
      malloc(sizeof(xmi_algorithm_t) * scatternum_algorithm[0]);

    result = XMI_Geometry_algorithms_info(context,
                                          world_geometry,
                                          XMI_XFER_SCATTER,
                                          scatteralgorithm,
                                          (xmi_metadata_t*)NULL,
                                          scatternum_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);
  }

  double ti, tf, usec;
  char *buf    = (char*)malloc(BUFSIZE*sz);
  char *rbuf   = (char*)malloc(BUFSIZE*sz);
  xmi_xfer_t barrier;
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

  xmi_xfer_t scatter;
  scatter.cb_done    = cb_scatter;
  scatter.cookie     = (void*)&_g_scatter_active;
  scatter.algorithm  = scatteralgorithm[0];
  scatter.cmd.xfer_scatter.root       = root;
  scatter.cmd.xfer_scatter.sndbuf     = buf;
  scatter.cmd.xfer_scatter.stype      = XMI_BYTE;
  scatter.cmd.xfer_scatter.stypecount = 0;
  scatter.cmd.xfer_scatter.rcvbuf     = rbuf;
  scatter.cmd.xfer_scatter.rtype      = XMI_BYTE;
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
  free(scatteralgorithm);
  free(algorithm);
  return 0;
};
