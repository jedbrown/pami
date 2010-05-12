/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#include <pami.h>


#define BUFSIZE 262144
#define NITER   100

volatile unsigned       _g_barrier_active;
volatile unsigned       _g_broadcast_active;
volatile unsigned       _g_total_broadcasts;
char                   *_g_recv_buffer;

void cb_ambcast_done (void *context, void * clientdata, pami_result_t err)
{
  _g_total_broadcasts++;
  free(clientdata);
}


void cb_bcast_recv  (pami_context_t         context,
                     size_t                root,
                     pami_geometry_t        geometry,
                     const size_t          sndlen,
                     void                * user_header,
                     const size_t          headerlen,
                     void               ** rcvbuf,
                     pami_type_t          * rtype,
                     size_t              * rtypecount,
                     pami_event_function  * const cb_info,
                     void                ** cookie)
{

  *rcvbuf                        = malloc(sndlen);
  *rtype                         = PAMI_BYTE;
  *rtypecount                    = sndlen;
  *cb_info                       = cb_ambcast_done;
  *cookie                        = (void*)*rcvbuf;
}

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
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

void _barrier (pami_context_t context, pami_xfer_t *barrier)
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


void _broadcast (pami_context_t context, pami_xfer_t *broadcast)
{
  _g_broadcast_active++;
  pami_result_t result;
  result = PAMI_Collective(context, (pami_xfer_t*)broadcast);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue broadcast collective. result = %d\n", result);
      exit(1);
    }
}



int main(int argc, char*argv[])
{
  double tf,ti,usec;
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  char buf[BUFSIZE];
  char rbuf[BUFSIZE];
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
  size_t task_id = configuration.value.intval;


  pami_geometry_t  world_geometry;
  result = PAMI_Geometry_world (client, &world_geometry);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
        return 1;
      }

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

  if (num_algorithm[0])
  {
    algorithm = (pami_algorithm_t*)
                malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
    result = PAMI_Geometry_query(context,
                                          world_geometry,
                                          PAMI_XFER_BARRIER,
                                          algorithm,
                                          (pami_metadata_t*)NULL,
                                          num_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

  }

  unsigned     rank = task_id;
  unsigned i,j,root = 0;
  _g_recv_buffer = rbuf;

  pami_xfer_t barrier;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.algorithm = algorithm[0];
  _barrier(context, &barrier);

  pami_algorithm_t *bcastalgorithm=NULL;
  pami_metadata_t *metas=NULL;
  int bcastnum_algorithm[2] = {0};
  result = PAMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       PAMI_XFER_AMBROADCAST,
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
    result = PAMI_Geometry_query(context,
                                          world_geometry,
                                          PAMI_XFER_AMBROADCAST,
                                          bcastalgorithm,
                                          metas,
                                          bcastnum_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);
  }
  pami_xfer_t broadcast;
  broadcast.cb_done   = cb_broadcast;
  broadcast.cookie    = (void*)&_g_broadcast_active;
  broadcast.algorithm = bcastalgorithm[0];
  broadcast.cmd.xfer_ambroadcast.user_header  = NULL;
  broadcast.cmd.xfer_ambroadcast.headerlen    = 0;
  broadcast.cmd.xfer_ambroadcast.sndbuf       = buf;
  broadcast.cmd.xfer_ambroadcast.stype        = PAMI_BYTE;
  broadcast.cmd.xfer_ambroadcast.stypecount   = 0;

  int nalg = 0;
  for(nalg=0; nalg<bcastnum_algorithm[0]; nalg++)
  {
    if (rank == root)
      {
        printf("# Broadcast Bandwidth Test -- root = %d, %s\n", root, metas[nalg].name);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

    pami_collective_hint_t h={0};
    pami_dispatch_callback_fn fn;
    fn.ambroadcast = cb_bcast_recv;
    PAMI_AMCollective_dispatch_set(context,
				   bcastalgorithm[nalg],
				   0,
				   fn,
				   NULL,
				   h);
    broadcast.algorithm = bcastalgorithm[nalg];

    _barrier (context, &barrier);
    for(i=1; i<=BUFSIZE; i*=2)
      {
        long long dataSent = i;
        unsigned     niter = NITER;
        if(rank==root)
            {
              ti = timer();
              for (j=0; j<niter; j++)
                  {
                    broadcast.cmd.xfer_ambroadcast.stypecount = i;
                    _broadcast (context,&broadcast);
                  }
              while (_g_broadcast_active)
                result = PAMI_Context_advance (context, 1);
              _barrier(context, &barrier);
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
                result = PAMI_Context_advance (context, 1);

              _g_total_broadcasts = 0;
              _barrier(context, &barrier);

            }
      }
  }

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

  free(algorithm);
  free(bcastalgorithm);
  return 0;
}
