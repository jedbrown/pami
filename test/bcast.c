///
/// \file tests/bcast.c
/// \brief Simple Barrier test
///

#include "sys/xmi.h"
#include <sys/time.h>
#include <unistd.h>

#define BUFSIZE 524288
volatile unsigned       _g_barrier_active;
volatile unsigned       _g_broadcast_active;

void cb_barrier (void *ctxt, void * clientdata, xmi_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_broadcast (void *ctxt, void * clientdata, xmi_result_t err)
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

void _broadcast (xmi_context_t context, xmi_broadcast_t *broadcast)
{
  _g_broadcast_active++;
  xmi_result_t result;
  result = XMI_Collective(context, (xmi_xfer_t*)broadcast);  
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue broadcast collective. result = %d\n", result);
      exit(1);
    }
  while (_g_broadcast_active)
    result = XMI_Context_advance (context, 1);

}

int main (int argc, char ** argv)
{
  xmi_client_t  client;
  xmi_context_t context;
  xmi_result_t  result = XMI_ERROR;
  
  result = XMI_Client_initialize ("TEST", &client);
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

  xmi_algorithm_t bcastalgorithm[1];
  int             bcastnum_algorithm = 1;
  result = XMI_Geometry_algorithm(context,
				  XMI_XFER_BROADCAST,
				  world_geometry,
				  &bcastalgorithm[0],
				  &bcastnum_algorithm);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query broadcast algorithm. result = %d\n", result);
        return 1;
      }
  
  

  double ti, tf, usec;
  char buf[BUFSIZE];
  char rbuf[BUFSIZE];
  xmi_barrier_t barrier;
  barrier.xfer_type = XMI_XFER_BARRIER;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.geometry  = world_geometry;
  barrier.algorithm = algorithm[0];
  _barrier(context, &barrier);

  
  int root = 0;
  if (task_id == root)
      {
        printf("# Broadcast Bandwidth Test -- root = %d\n", root);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

  xmi_broadcast_t broadcast;
  broadcast.xfer_type = XMI_XFER_BROADCAST;
  broadcast.cb_done   = cb_broadcast;
  broadcast.cookie    = (void*)&_g_broadcast_active;
  broadcast.geometry  = world_geometry;
  broadcast.algorithm = bcastalgorithm[0];
  broadcast.root      = root;
  broadcast.buf       = buf;
  broadcast.type      = XMI_BYTE;
  broadcast.typecount = 0;

  int i,j;
  for(i=1; i<=BUFSIZE; i*=2)
      {
        long long dataSent = i;
        int          niter = 100;
        _barrier(context, &barrier);
        ti = timer();
        for (j=0; j<niter; j++)
            {
              broadcast.typecount = i;
              _broadcast (context, &broadcast);
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

  return 0;
};




