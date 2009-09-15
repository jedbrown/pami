#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

#include "sys/xmi.h"

#define BUFSIZE 1048576

// Geometry Objects
volatile unsigned        _g_barrier_active;
volatile unsigned        _g_broadcast_active;

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

void cb_barrier (void *context, void * clientdata, xmi_result_t res)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_broadcast (void *context, void * clientdata, xmi_result_t res)
{
    int * active = (int *) clientdata;
    (*active)--;
}

void _barrier (xmi_context_t  context,
               xmi_barrier_t *barrier)
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

void _broadcast (xmi_context_t    context,
                 xmi_broadcast_t *broadcast)
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



int main(int argc, char*argv[])
{
  xmi_client_t  client;
  xmi_context_t context;
  xmi_result_t  result = XMI_ERROR;
  double        tf,ti,usec;
  char          buf[BUFSIZE];
  char          rbuf[BUFSIZE];

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
  size_t rank = configuration.value.intval;

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query (context, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t          sz    = configuration.value.intval;
  size_t          half  = sz/2;
  size_t          set[2];
  xmi_geometry_t  world_geometry;
  result = XMI_Geometry_world (context, &world_geometry);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
        return 1;
      }
  int             num_algorithm = 1;
  xmi_algorithm_t world_algorithm[1];
  result = XMI_Geometry_algorithm(context,
				  XMI_XFER_BARRIER,
				  world_geometry,
				  &world_algorithm[0],
				  &num_algorithm);

  
  

  if(rank == 0)
    printf("Initializing Bottom Geometry\n");


  xmi_barrier_t world_barrier;
  world_barrier.xfer_type = XMI_XFER_BARRIER;
  world_barrier.cb_done   = cb_barrier;
  world_barrier.cookie    = (void*)&_g_barrier_active;
  world_barrier.geometry  = world_geometry;
  world_barrier.algorithm = world_algorithm[0];
  _barrier(context, &world_barrier);

  
  xmi_geometry_t           bottom_geometry;
  xmi_geometry_range_t     bottom_range;
  xmi_algorithm_t          bottom_algorithm[1];
  xmi_barrier_t            bottom_barrier;
  xmi_broadcast_t          bottom_broadcast;
  
  xmi_geometry_t           top_geometry;
  xmi_geometry_range_t     top_range;
  xmi_algorithm_t          top_algorithm[1];
  xmi_barrier_t            top_barrier;
  xmi_broadcast_t          top_broadcast;
  if(rank>=0 && rank<=half-1)
      {
        bottom_range.lo =0;
        bottom_range.hi =half-1;
        result = XMI_Geometry_initialize (context,
                                          &bottom_geometry,
                                          1,
                                          &bottom_range,
                                          1);

        result = XMI_Geometry_algorithm(context,
                                        XMI_XFER_BARRIER,
                                        bottom_geometry,
                                        &bottom_algorithm[0],
                                        &num_algorithm);
        _barrier (context, &world_barrier);
        bottom_barrier.xfer_type = XMI_XFER_BARRIER;
        bottom_barrier.cb_done   = cb_barrier;
        bottom_barrier.cookie    = (void*)&_g_barrier_active;
        bottom_barrier.geometry  = bottom_geometry;
        bottom_barrier.algorithm = bottom_algorithm[0];


        result = XMI_Geometry_algorithm(context,
                                        XMI_XFER_BROADCAST,
                                        bottom_geometry,
                                        &bottom_algorithm[0],
                                        &num_algorithm);
        bottom_broadcast.xfer_type = XMI_XFER_BROADCAST;
        bottom_broadcast.cb_done   = cb_broadcast;
        bottom_broadcast.cookie    = (void*)&_g_broadcast_active;
        bottom_broadcast.geometry  = bottom_geometry;
        bottom_broadcast.algorithm = bottom_algorithm[0];
        bottom_broadcast.root      = 0;
        bottom_broadcast.buf       = NULL;
        bottom_broadcast.type      = XMI_BYTE;
        bottom_broadcast.typecount = 0;


        set[0]=1;
        set[1]=0;
      }
  else
      {
        top_range.lo =half;
        top_range.hi =sz-1;
        result = XMI_Geometry_initialize (context,
                                          &top_geometry,
                                          1,
                                          &top_range,
                                          2);
        _barrier (context, &world_barrier);
        result = XMI_Geometry_algorithm(context,
                                        XMI_XFER_BARRIER,
                                        top_geometry,
                                        &top_algorithm[0],
                                        &num_algorithm);

        top_barrier.xfer_type = XMI_XFER_BARRIER;
        top_barrier.cb_done   = cb_barrier;
        top_barrier.cookie    = (void*)&_g_barrier_active;
        top_barrier.geometry  = top_geometry;
        top_barrier.algorithm = top_algorithm[0];

        result = XMI_Geometry_algorithm(context,
                                        XMI_XFER_BROADCAST,
                                        top_geometry,
                                        &top_algorithm[0],
                                        &num_algorithm);
        top_broadcast.xfer_type = XMI_XFER_BROADCAST;
        top_broadcast.cb_done   = cb_broadcast;
        top_broadcast.cookie    = (void*)&_g_broadcast_active;
        top_broadcast.geometry  = top_geometry;
        top_broadcast.algorithm = top_algorithm[0];
        top_broadcast.root      = 0;
        top_broadcast.buf       = NULL;
        top_broadcast.type      = XMI_BYTE;
        top_broadcast.typecount = 0;

         
        set[0]=0;
        set[1]=1;
      }
 
  
  xmi_barrier_t   *barriers   [] = {&bottom_barrier, &top_barrier};
  xmi_broadcast_t *broadcasts [] = {&bottom_broadcast, &top_broadcast};
  size_t           roots[]        = {0, half};
  int             i,j,k;
  for(k=0; k<2; k++)
      {
        if (rank == roots[k])
            {
              printf("# Broadcast Bandwidth Test -- root = %d\n", (int)roots[k]);
              printf("# Size(bytes)           cycles    bytes/sec    usec\n");
              printf("# -----------      -----------    -----------    ---------\n");
            }
        if(set[k])
            {
              printf("Participant:  %d\n", (int)rank);
              fflush(stdout);
              _barrier (context, barriers[k]);
              for(i=1; i<=BUFSIZE; i*=2)
                  {
                    long long dataSent = i;
                    int          niter = 100;
                    _barrier (context, barriers[k]);
                    ti = timer();
                    for (j=0; j<niter; j++)
                        {
                          broadcasts[k]->root      = roots[k];
                          broadcasts[k]->buf       = buf;
                          broadcasts[k]->typecount = i;
                          _broadcast(context, broadcasts[k]);
                        }
                    tf = timer();
                    _barrier (context, barriers[k]);
                    usec = (tf - ti)/(double)niter;
                    if (rank == roots[k])
                        {
                          printf("  %11lld %16lld %14.1f %12.2f\n",
                                 dataSent,
                                 0LL,
                                 (double)1e6*(double)dataSent/(double)usec,
                                 usec);
                          fflush(stdout);
                        }
                  }
            }
        _barrier (context, &world_barrier);
      }
  _barrier (context, &world_barrier);

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
}
