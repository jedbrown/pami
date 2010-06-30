///
/// \file test/bcast_advisor.c
/// \brief Simple Bcast test with advisor functionality
///

#include "util/compact_attributes.h"
#include <pami.h>
#include "common/CollAdvisor.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define BUFSIZE 524288
volatile unsigned       _g_barrier_active;
volatile unsigned       _g_broadcast_active;

void cb_barrier (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_broadcast (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

static double timer()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

void _barrier (pami_context_t context, pami_xfer_t *barrier)
{
  _g_barrier_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)barrier);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr,
               "Error. Unable to issue barrier collective. result = %d\n",
               result);
      exit(1);
    }

  while (_g_barrier_active)
    result = PAMI_Context_advance (context, 1);

}

void _broadcast (pami_context_t context, pami_xfer_t *broadcast)
{
  _g_broadcast_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)broadcast);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr,
               "Error. Unable to issue broadcast collective. result = %d\n",
               result);
      exit(1);
    }

  while (_g_broadcast_active)
    result = PAMI_Context_advance (context, 1);

}

int main (int argc, char ** argv)
{
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  char cl_string[] = "TEST";
  result = PAMI_Client_create (cl_string, &client);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n",
               result);
      return 1;
    }

  {

    result = PAMI_Context_createv(client, NULL, 0, &context, 1);
  }

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to create pami context. result = %d\n",
               result);
      return 1;
    }


  pami_configuration_t configuration;
  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n",
               configuration.name, result);
      return 1;
    }

  size_t task_id = configuration.value.intval;


  pami_geometry_t  world_geometry;

  result = PAMI_Geometry_world (client, &world_geometry);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get world geometry. result = %d\n",
               result);
      return 1;
    }

  pami_advisor_init();

  pami_advisor_repo_fill(client, context, PAMI_XFER_BROADCAST);


  int algorithm_type = 0;
  pami_algorithm_t *algorithm = NULL;
  size_t num_algorithm[2] = {0};
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

  pami_algorithm_t *bcastalgorithm = NULL;
  pami_metadata_t *metas = NULL;
  size_t bcastnum_algorithm[2] = {0};
  result = PAMI_Geometry_algorithms_num(context,
                                        world_geometry,
                                        PAMI_XFER_BROADCAST,
                                        bcastnum_algorithm);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr,
               "Error. Unable to query bcast algorithm. result = %d\n",
               result);
      return 1;
    }

  if (bcastnum_algorithm[0])
    {
      bcastalgorithm = (pami_algorithm_t*)
                       malloc(sizeof(pami_algorithm_t) * bcastnum_algorithm[0]);
      metas = (pami_metadata_t*)
              malloc(sizeof(pami_metadata_t) * bcastnum_algorithm[0]);

      result = PAMI_Geometry_algorithms_query(context,
                                              world_geometry,
                                              PAMI_XFER_BROADCAST,
                                              bcastalgorithm,
                                              metas,
                                              bcastnum_algorithm[0],
                                              NULL,
                                              NULL,
                                              0);

      if (result != PAMI_SUCCESS)
        {
          fprintf(stderr, "Error. Unable to query broadcast algorithm attributes."
                  "result = %d\n", result);
          return 1;
        }
    }

  double ti, tf, usec;
  char buf[BUFSIZE];
  pami_xfer_t barrier;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*) & _g_barrier_active;
  barrier.algorithm = algorithm[0];
  _barrier(context, &barrier);

  int nalg = 0;

  for (nalg = 0; nalg < bcastnum_algorithm[algorithm_type]; nalg++)
    {
      int root = 0;
      pami_xfer_t broadcast;
      broadcast.cb_done   = cb_broadcast;
      broadcast.cookie    = (void*) & _g_broadcast_active;
      broadcast.algorithm = bcastalgorithm[nalg];
      broadcast.cmd.xfer_broadcast.root      = root;
      broadcast.cmd.xfer_broadcast.buf       = buf;
      broadcast.cmd.xfer_broadcast.type      = PAMI_BYTE;
      broadcast.cmd.xfer_broadcast.typecount = 0;


      if (result != PAMI_SUCCESS)
        {
          fprintf(stderr, "Error. Unable to query broadcast algorithm attributes."
                  "result = %d\n", result);
          return 1;
        }


      if (task_id == (size_t)root)
        {
          printf("# Broadcast Bandwidth Test -- root = %d  protocol: %s\n", root,
                 metas[nalg].name);
          printf("# Size(bytes)           cycles    bytes/sec    usec\n");
          printf("# -----------      -----------    -----------    ---------\n");
        }

      int i, j;

      for (i = 1; i <= BUFSIZE; i *= 2)
        {
          long long dataSent = i;
          int          niter = 100;
          _barrier(context, &barrier);
          ti = timer();

          for (j = 0; j < niter; j++)
            {
              broadcast.cmd.xfer_broadcast.typecount = i;
              _broadcast (context, &broadcast);
            }

          tf = timer();
          _barrier(context, &barrier);

          usec = (tf - ti) / (double)niter;

          if (task_id == (size_t)root)
            {
              printf("  %11lld %16lld %14.1f %12.2f\n",
                     dataSent,
                     0LL,
                     (double)1e6*(double)dataSent / (double)usec,
                     usec);
              fflush(stdout);
            }
        }
    }

  result = PAMI_Context_destroyv(&context, 1);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n",
               result);
      return 1;
    }

  result = PAMI_Client_destroy(&client);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n",
               result);
      return 1;
    }

  free(metas);
  free(algorithm);
  free(bcastalgorithm);
  return 0;
};
