///
/// \file tests/bcast.c
/// \brief Simple Barrier test
///

#include "sys/xmi.h"
#include <sys/time.h>
#include <unistd.h>

#define BUFSIZE 524288
volatile unsigned       _g_barrier_active;
volatile unsigned       _g_scatterv_active;

void cb_barrier (void *ctxt, void * clientdata, xmi_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_scatterv (void *ctxt, void * clientdata, xmi_result_t err)
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

void _scatterv (xmi_context_t context, xmi_scatterv_t *scatterv)
{
  _g_scatterv_active++;
  xmi_result_t result;
  result = XMI_Collective(context, (xmi_xfer_t*)scatterv);  
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue scatterv collective. result = %d\n", result);
      exit(1);
    }
  while (_g_scatterv_active)
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

  xmi_algorithm_t scattervalgorithm[1];
  int             scattervnum_algorithm = 1;
  result = XMI_Geometry_algorithm(context,
				  XMI_XFER_SCATTERV,
				  world_geometry,
				  &scattervalgorithm[0],
				  &scattervnum_algorithm);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query scatterv algorithm. result = %d\n", result);
        return 1;
      }
  
  

  double ti, tf, usec;
  char *buf    = (char*)malloc(BUFSIZE*sz);
  char *rbuf   = (char*)malloc(BUFSIZE*sz);
  size_t *lengths = (size_t*)malloc(sz*sizeof(size_t));
  size_t *displs = (size_t*)malloc(sz*sizeof(size_t));
  xmi_barrier_t barrier;
  barrier.xfer_type = XMI_XFER_BARRIER;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.geometry  = world_geometry;
  barrier.algorithm = algorithm[0];
  _barrier(context, &barrier);

  
  size_t root = 0;
  if (task_id == root)
      {
        printf("# Scatterv Bandwidth Test -- \n");
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

  xmi_scatterv_t scatterv;
  scatterv.xfer_type = XMI_XFER_SCATTERV;
  scatterv.cb_done   = cb_scatterv;
  scatterv.cookie    = (void*)&_g_scatterv_active;
  scatterv.geometry  = world_geometry;
  scatterv.algorithm = scattervalgorithm[0];
  scatterv.root      = root;
  scatterv.sndbuf    = buf;
  scatterv.stype      = XMI_BYTE;
  scatterv.stypecounts = lengths;
  scatterv.sdispls     = displs;
  scatterv.rcvbuf     = rbuf;
  scatterv.rtype      = XMI_BYTE;
  scatterv.rtypecount = 0;

  int i,j;
  for(i=1; i<=BUFSIZE; i*=2)
      {
        long long dataSent = i;
        int          niter = 100;
        int              k = 0;
        
        for(k=0;k<sz;k++)
            {
              lengths[k] = i;
              displs[k]  = 0;
            }
        _barrier(context, &barrier);
        ti = timer();
        for (j=0; j<niter; j++)
            {
              scatterv.rtypecount = i;
              _scatterv (context, &scatterv);
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




