/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/ambcast.c
 * \brief ???
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "sys/xmi.h"


#define BUFSIZE 524288
#define NITER   1000

volatile unsigned       _g_barrier_active;
volatile unsigned       _g_broadcast_active;
volatile unsigned       _g_total_broadcasts;
char                   *_g_recv_buffer;

void cb_ambcast_done (xmi_client_t client, size_t context, void * clientdata, xmi_result_t err)
{
  _g_total_broadcasts++;
  free(clientdata);
}


void cb_bcast_recv  (xmi_client_t client, size_t context,
                     unsigned              root,
                     xmi_geometry_t        geometry,
                     const size_t          sndlen,
                     void                * user_header,
                     const size_t          headerlen,
                     void               ** rcvbuf,
                     xmi_type_t          * rtype,
                     size_t              * rtypecount,
                     xmi_event_function  * const cb_info,
                     void                ** cookie)
{
  *rcvbuf                        = malloc(sndlen);
  *rtype                         = XMI_BYTE;
  *rtypecount                    = sndlen;
  *cb_info                       = cb_ambcast_done;
  *cookie                        = (void*)rcvbuf;
}

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

void cb_barrier (xmi_client_t client, size_t context, void * clientdata, xmi_result_t res)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_broadcast (xmi_client_t client, size_t context, void * clientdata, xmi_result_t res)
{
    int * active = (int *) clientdata;
    (*active)--;
}

void _barrier (xmi_client_t client, size_t context, xmi_barrier_t *barrier)
{
  _g_barrier_active++;
  xmi_result_t result;
  result = XMI_Collective(client, context, (xmi_xfer_t*)barrier);
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue barrier collective. result = %d\n", result);
      exit(1);
    }
  while (_g_barrier_active)
    result = XMI_Context_advance (client, context, 1);

}


void _broadcast (xmi_client_t client, size_t context, xmi_ambroadcast_t *broadcast)
{
  _g_broadcast_active++;
  xmi_result_t result;
  result = XMI_Collective(client, context, (xmi_xfer_t*)broadcast);
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue broadcast collective. result = %d\n", result);
      exit(1);
    }
}



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  xmi_client_t  client;
  xmi_result_t  result = XMI_ERROR;
  char buf[BUFSIZE];
  char rbuf[BUFSIZE];
  char          cl_string[] = "TEST";
  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
        return 1;
      }

  result = XMI_Context_create(client, NULL, 0, 1);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
        return 1;
      }

  xmi_configuration_t configuration;
  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query (client, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t task_id = configuration.value.intval;


  xmi_geometry_t  world_geometry;
  result = XMI_Geometry_world (client, 0, &world_geometry);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
        return 1;
      }

  int algorithm_type = 0;
  xmi_algorithm_t *algorithm;
  int num_algorithm[2] = {0};
  result = XMI_Geometry_algorithms_num(client, 0,
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
    result = XMI_Geometry_algorithms_info(client, 0,
                                          world_geometry,
                                          XMI_XFER_BARRIER,
                                          algorithm,
                                          (xmi_metadata_t*)NULL,
                                          algorithm_type,
                                          num_algorithm[0]);

  }

  unsigned     rank = task_id;
  unsigned i,j,root = 0;
  _g_recv_buffer = rbuf;


  if (rank == root)
      {
        printf("# Broadcast Bandwidth Test -- root = %d\n", root);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

  xmi_barrier_t barrier;
  barrier.xfer_type = XMI_XFER_BARRIER;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.geometry  = world_geometry;
  barrier.algorithm = algorithm[0];
  _barrier(client, 0, &barrier);

  xmi_algorithm_t *bcastalgorithm;
  int bcastnum_algorithm[2] = {0};
  result = XMI_Geometry_algorithms_num(client, 0,
                                       world_geometry,
                                       XMI_XFER_BROADCAST,
                                       bcastnum_algorithm);

  if (result != XMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query bcast algorithm. result = %d\n",
             result);
    return 1;
  }
  
  if (bcastnum_algorithm[0])
  {
    bcastalgorithm = (xmi_algorithm_t*)
      malloc(sizeof(xmi_algorithm_t) * bcastnum_algorithm[0]);
    
    result = XMI_Geometry_algorithms_info(client, 0,
                                          world_geometry,
                                          XMI_XFER_BROADCAST,
                                          bcastalgorithm,
                                          (xmi_metadata_t*)NULL,
                                          algorithm_type = 0,
                                          bcastnum_algorithm[0]);
  }
  xmi_ambroadcast_t broadcast;
  broadcast.xfer_type = XMI_XFER_AMBROADCAST;
  broadcast.cb_done   = cb_broadcast;
  broadcast.cookie    = (void*)&_g_broadcast_active;
  broadcast.geometry  = world_geometry;
  broadcast.algorithm = bcastalgorithm[0];
  broadcast.user_header  = NULL;
  broadcast.headerlen    = 0;
  broadcast.sndbuf       = buf;
  broadcast.stype        = XMI_BYTE;
  broadcast.stypecount   = 0;


    _barrier (client, 0, &barrier);
  for(i=1; i<=BUFSIZE; i*=2)
      {
        long long dataSent = i;
        unsigned     niter = NITER;
        if(rank==root)
            {
              ti = timer();
              for (j=0; j<niter; j++)
                  {
                    broadcast.stypecount = 0;
                    _broadcast (client, 0,&broadcast);
                  }
              while (_g_broadcast_active)
                result = XMI_Context_advance (client, 0, 1);

              _barrier(client, 0, &barrier);
              tf = timer();
              usec = (tf - ti)/(double)niter;
              printf("  %11lld %16lld %14.1f %12.2f\n",
                     dataSent,
                     0LL,
                     (double)1e6*(double)dataSent/(double)usec,
                     usec);
              fflush(stdout);
            }
        else
            {
              while(_g_total_broadcasts < niter)
                result = XMI_Context_advance (client, 0, 1);
              _g_total_broadcasts = 0;
              _barrier(client, 0, &barrier);

            }
      }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
        return 1;
      }

  free(algorithm);
  free(bcastalgorithm);
  return 0;
}
