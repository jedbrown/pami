/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/bcast_subcomm2.c
 * \brief ???
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

#include "sys/pami.h"

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
void cb_geom_init (void *context, void * clientdata, pami_result_t res)
{
  int * active = (int *) clientdata;
  (*active)--;
}
void cb_barrier (void *context, void * clientdata, pami_result_t res)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_broadcast (void *context, void * clientdata, pami_result_t res)
{
    int * active = (int *) clientdata;
    (*active)--;
}

void _barrier (pami_context_t  context,
               pami_xfer_t *barrier)
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

void _broadcast (pami_context_t    context,
                 pami_xfer_t *broadcast)
{
  _g_broadcast_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)broadcast);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to issue broadcast collective. result = %d\n", result);
        exit(1);
      }
  while (_g_broadcast_active)
    result = PAMI_Context_advance (context, 1);
}



int main(int argc, char*argv[])
{
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  double        tf,ti,usec;
  char          buf[BUFSIZE];
  char          cl_string[] = "TEST";
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
  size_t rank = configuration.value.intval;

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t          sz    = configuration.value.intval;
  size_t          set[2];
  pami_geometry_t  world_geometry;
  result = PAMI_Geometry_world (client, &world_geometry);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
        return 1;
      }
  int algorithm_type = 0;
  pami_algorithm_t *world_algorithm=NULL;
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
    world_algorithm = (pami_algorithm_t*)
      malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
    result = PAMI_Geometry_query(context,
                                          world_geometry,
                                          PAMI_XFER_BARRIER,
                                          world_algorithm,
                                          (pami_metadata_t*)NULL,
                                          num_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

  }


  pami_xfer_t world_barrier;
  world_barrier.cb_done   = cb_barrier;
  world_barrier.cookie    = (void*)&_g_barrier_active;
  world_barrier.algorithm = world_algorithm[0];
  _barrier(context, &world_barrier);


  pami_geometry_t           bottom_geometry;
  pami_geometry_range_t    *bottom_range;
  pami_algorithm_t         *bottom_bar_algorithm=NULL;
  pami_algorithm_t         *bottom_bcast_algorithm=NULL;
  pami_xfer_t            bottom_barrier;
  pami_xfer_t          bottom_broadcast;

  pami_geometry_t           top_geometry;
  pami_geometry_range_t    *top_range;
  pami_algorithm_t         *top_bar_algorithm=NULL;
  pami_algorithm_t         *top_bcast_algorithm=NULL;
  pami_xfer_t            top_barrier;
  pami_xfer_t          top_broadcast;
  int                   geom_init = 1;
  if(((rank%2)==0))
      {
        fprintf(stderr, "%d:  Creating Bottom Geometry\n", (int)rank);
        bottom_range = (pami_geometry_range_t *)malloc(((sz+1)/2)*sizeof(pami_geometry_range_t));
        int iter=0;
        size_t i;
        for(i=0; i<sz; i++)
            {
              if((i%2)==0)
                  {
                    bottom_range[iter].lo =i;
                    bottom_range[iter].hi =i;
                    iter++;
                  }
            }
        result = PAMI_Geometry_create_taskrange (client,
                                          &bottom_geometry,
                                            world_geometry,
                                          1,
                                          bottom_range,
                                            1,
                                            context,
                                            cb_geom_init,
                                            &geom_init);
        while (geom_init == 1)
          result = PAMI_Context_advance (context, 1);

        result = PAMI_Geometry_algorithms_num(context,
                                             bottom_geometry,
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
          bottom_bar_algorithm = (pami_algorithm_t*)
            malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
          result = PAMI_Geometry_query(context,
                                                bottom_geometry,
                                                PAMI_XFER_BARRIER,
                                                bottom_bar_algorithm,
                                                (pami_metadata_t*)NULL,
                                                num_algorithm[0],
                                                NULL,
                                                NULL,
                                                0);

        }

        bottom_barrier.cb_done   = cb_barrier;
        bottom_barrier.cookie    = (void*)&_g_barrier_active;
        bottom_barrier.algorithm = bottom_bar_algorithm[0];

        result = PAMI_Geometry_algorithms_num(context,
                                             bottom_geometry,
                                             PAMI_XFER_BROADCAST,
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
          bottom_bcast_algorithm = (pami_algorithm_t*)
            malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);

          result = PAMI_Geometry_query(context,
                                                bottom_geometry,
                                                PAMI_XFER_BROADCAST,
                                                bottom_bcast_algorithm,
                                                (pami_metadata_t*)NULL,
                                                num_algorithm[0],
                                                NULL,
                                                NULL,
                                                0);

        }

        bottom_broadcast.cb_done   = cb_broadcast;
        bottom_broadcast.cookie    = (void*)&_g_broadcast_active;
        bottom_broadcast.algorithm = bottom_bcast_algorithm[0];
        bottom_broadcast.cmd.xfer_broadcast.root      = 0;
        bottom_broadcast.cmd.xfer_broadcast.buf       = NULL;
        bottom_broadcast.cmd.xfer_broadcast.type      = PAMI_BYTE;
        bottom_broadcast.cmd.xfer_broadcast.typecount = 0;


        set[0]=1;
        set[1]=0;
      }
  else
      {
        fprintf(stderr, "%d:  Creating Top Geometry\n", (int)rank);
        int iter=0;
        size_t i;
        top_range  = (pami_geometry_range_t *)malloc(((sz+1)/2)*sizeof(pami_geometry_range_t));
        for(i=0; i<sz; i++)
            {
              if((i%2)!=0)
                  {
                    top_range[iter].lo =i;
                    top_range[iter].hi =i;
                    iter++;
                  }
            }

        result = PAMI_Geometry_create_taskrange (client,
                                          &top_geometry,
                                            world_geometry,
                                          2,
                                          top_range,
                                            1,
                                            context,
                                            cb_geom_init,
                                            &geom_init);

        while (geom_init == 1)
          result = PAMI_Context_advance (context, 1);

        result = PAMI_Geometry_algorithms_num(context,
                                             top_geometry,
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
          top_bar_algorithm = (pami_algorithm_t*)
            malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
          result = PAMI_Geometry_query(context,
                                                top_geometry,
                                                PAMI_XFER_BARRIER,
                                                top_bar_algorithm,
                                                (pami_metadata_t*)NULL,
                                                num_algorithm[0],
                                                NULL,
                                                NULL,
                                                0);

        }


        top_barrier.cb_done   = cb_barrier;
        top_barrier.cookie    = (void*)&_g_barrier_active;
        top_barrier.algorithm = top_bar_algorithm[0];

        result = PAMI_Geometry_algorithms_num(context,
                                             top_geometry,
                                             PAMI_XFER_BROADCAST,
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
          top_bcast_algorithm = (pami_algorithm_t*)
            malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);

          result = PAMI_Geometry_query(context,
                                                top_geometry,
                                                PAMI_XFER_BROADCAST,
                                                top_bcast_algorithm,
                                                (pami_metadata_t*)NULL,
                                                num_algorithm[0],
                                                NULL,
                                                NULL,
                                                0);

        }
        else assert(0);
        top_broadcast.cb_done   = cb_broadcast;
        top_broadcast.cookie    = (void*)&_g_broadcast_active;
        top_broadcast.algorithm = top_bcast_algorithm[0];
        top_broadcast.cmd.xfer_broadcast.root      = 0;
        top_broadcast.cmd.xfer_broadcast.buf       = NULL;
        top_broadcast.cmd.xfer_broadcast.type      = PAMI_BYTE;
        top_broadcast.cmd.xfer_broadcast.typecount = 0;


        set[0]=0;
        set[1]=1;
      }


  pami_xfer_t   *barriers   [] = {&bottom_barrier, &top_barrier};
  pami_xfer_t *broadcasts [] = {&bottom_broadcast, &top_broadcast};
  size_t           roots[]= {0, 1};
  int             i,j,k;
  for(k=0; k<=1; k++)
      {
        _barrier (context, &world_barrier);
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
              int alg = 0;
              // todo:  fix so that we use the right algorithm array for each comm
              // the code as is should work for simple splits assuming some
              // network symmetry
//              for(alg=0; alg<num_algorithm[algorithm_type]; alg++)
                  {
                    if (rank == roots[k])
                      fprintf(stderr, "Trying algorithm %d of %d\n", alg+1, num_algorithm[algorithm_type]);
                    _barrier (context, barriers[k]);
                    for(i=1; i<=BUFSIZE; i*=2)
                        {
                          long long dataSent = i;
                          int          niter = 100;
                          _barrier (context, barriers[k]);
                          ti = timer();
                          for (j=0; j<niter; j++)
                              {
                                broadcasts[k]->cmd.xfer_broadcast.root      = roots[k];
                                broadcasts[k]->cmd.xfer_broadcast.buf       = buf;
                                broadcasts[k]->cmd.xfer_broadcast.typecount = i;
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
            }
        _barrier (context, &world_barrier);
      }
  _barrier (context, &world_barrier);

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

  free(world_algorithm);
  //  free(top_algorithm);
  //  free(bottom_algorithm);
  return 0;
}
