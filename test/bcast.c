///
/// \file test/bcast.c
/// \brief Simple Bcast test
///

#include "util/compact_attributes.h"
#include <pami.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

//define this if you want to validate the data
#define CHECK_DATA

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

#ifdef ENABLE_MAMBO_WORKAROUNDS
#define BUFSIZE 131072 // any more is too long on mambo
#define NITER 10
#else
#define BUFSIZE 524288
#define NITER 100
#endif

size_t task_id;
volatile unsigned       _g_barrier_active;
volatile unsigned       _g_broadcast_active;

void cb_barrier (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  TRACE_ERR((stderr, "%s\n", __PRETTY_FUNCTION__));
  (*active)--;
}

void cb_broadcast (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  TRACE_ERR((stderr, "%s\n", __PRETTY_FUNCTION__));
  (*active)--;
}

static double timer()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

void _barrier (pami_context_t context, pami_xfer_t *barrier)
{
  static unsigned barrierCount = 0;
  TRACE_ERR((stderr,"%s %u\n", __PRETTY_FUNCTION__,barrierCount));
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
  TRACE_ERR((stderr, "%s exit %u\n", __PRETTY_FUNCTION__,barrierCount));
  barrierCount++;

}

void _broadcast (pami_context_t context, pami_xfer_t *broadcast)
{
  static unsigned broadcastCount = 0;
  TRACE_ERR((stderr, "%s %u\n", __PRETTY_FUNCTION__,broadcastCount));
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
  TRACE_ERR((stderr, "%s exit %u\n", __PRETTY_FUNCTION__,broadcastCount));
  broadcastCount++;
}
#ifdef CHECK_DATA
void initialize_sndbuf (void *buf, int bytes) {

  char c = 0x00;
  int i = bytes;
  unsigned char *cbuf = (unsigned char *)  buf;
  for (; i; i--) {
    cbuf[i-1] = c++;
  }
}

int check_rcvbuf (void *buf, int bytes) {

  char c = 0x00;
  int i = bytes;
  unsigned char *cbuf = (unsigned char *)  buf;
  for (; i; i--) {
    if(cbuf[i-1] != c)
    {
      fprintf(stderr, "Check failed %.2u != %.2u \n",cbuf[i-1],c);
      return -1;
    }
    c++;
  }
  TRACE_ERR((stderr,"Check Passes\n"));

  return 0;
}
#endif

int main (int argc, char ** argv)
{
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  TRACE_ERR((stderr, "%s\n", __PRETTY_FUNCTION__));
  char cl_string[] = "TEST";
  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }
  TRACE_ERR((stderr, "%s after client initialize\n", __PRETTY_FUNCTION__));

        {  result = PAMI_Context_createv(client, NULL, 0, &context, 1); }
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
    return 1;
  }
  TRACE_ERR((stderr, "%s after context create\n", __PRETTY_FUNCTION__));


  pami_configuration_t configuration;
  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  task_id = configuration.value.intval;


  pami_geometry_t  world_geometry;

  result = PAMI_Geometry_world (client, &world_geometry);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
    return 1;
  }

  int algorithm_type = 0;
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
  TRACE_ERR((stderr, "%s task_id %zu, barrier num_algorithm %u/%u\n", __PRETTY_FUNCTION__, task_id, num_algorithm[0],num_algorithm[1]));

  if (num_algorithm[0])
  {
    algorithm = (pami_algorithm_t*)
                malloc(sizeof(pami_algorithm_t) * num_algorithm[0]);
    TRACE_ERR((stderr, "%s PAMI_Geometry_algorithms_query\n", __PRETTY_FUNCTION__));
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
  else 
  {
      fprintf(stderr,"ERROR. We need a working barrier to continue\n");
      return 1;
  }

  pami_algorithm_t *bcastalgorithm=NULL;
  pami_metadata_t *metas=NULL;
  int bcastnum_algorithm[2] = {0};
  TRACE_ERR((stderr, "%s PAMI_Geometry_algorithms_num\n", __PRETTY_FUNCTION__));
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

    TRACE_ERR((stderr, "%s PAMI_Geometry_algorithms_query bcastnum_algorithm[0]=%u, metas=%p\n", __PRETTY_FUNCTION__,bcastnum_algorithm[0],metas));
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
    int i=0;
    for(;i<bcastnum_algorithm[0];++i)
    {
      TRACE_ERR((stderr, "%s PAMI_Geometry_query metas[%d]=%s\n", __PRETTY_FUNCTION__,i,metas[i].name));
    }
  }
  else
  {
    fprintf (stderr, "NOOP. No broadcasts defined\n");
    return 0;
  }

  double ti, tf, usec;
  char buf[BUFSIZE];
  pami_xfer_t barrier;
  barrier.cb_done   = cb_barrier;
  barrier.cookie    = (void*)&_g_barrier_active;
  barrier.algorithm = algorithm[0];
  TRACE_ERR((stderr, "%s barrier next\n", __PRETTY_FUNCTION__));
  _barrier(context, &barrier);

  int nalg = 0;
  for(nalg=0; nalg<bcastnum_algorithm[algorithm_type]; nalg++)
  {
    int root = 0;
    pami_xfer_t broadcast;
    broadcast.cb_done   = cb_broadcast;
    broadcast.cookie    = (void*)&_g_broadcast_active;
    broadcast.algorithm = bcastalgorithm[nalg];
    broadcast.cmd.xfer_broadcast.root      = root;
    broadcast.cmd.xfer_broadcast.buf       = buf;
    broadcast.cmd.xfer_broadcast.type      = PAMI_BYTE;
    broadcast.cmd.xfer_broadcast.typecount = 0;

    if (task_id == (size_t)root)
    {
      printf("# Broadcast Bandwidth Test -- root = %d  protocol: %s\n", root,
             metas[nalg].name);
      printf("# Size(bytes)           cycles    bytes/sec    usec\n");
      printf("# -----------      -----------    -----------    ---------\n");
    }
    else TRACE_ERR((stderr, "start test protocol: %s \n", metas[nalg].name));

    int i,j;
#ifdef ENABLE_MAMBO_WORKAROUNDS  // doesn't support chars on MU
    for(i=4; i<=BUFSIZE; i*=2)
#else
    for(i=1; i<=BUFSIZE; i*=2)
#endif
    {
      long long dataSent = i;
      int          niter = NITER;
#ifdef CHECK_DATA
      if (task_id == (size_t)root)
        initialize_sndbuf (buf, i);
      else
        memset(buf, 0xFF, i);
#endif
      _barrier(context, &barrier);
      ti = timer();
      for (j=0; j<niter; j++)
      {
        broadcast.cmd.xfer_broadcast.typecount = i;
        _broadcast (context, &broadcast);
	//	_barrier(context, &barrier);
      }
      //Asyncbroadcast will complete at differnet times on different nodes.
      _barrier(context, &barrier);
      tf = timer();
#ifdef CHECK_DATA
        check_rcvbuf (buf, i);
#endif

      usec = (tf - ti)/(double)niter;
      if (task_id == (size_t)root)
      {
        printf("  %11lld %16lld %14.1f %12.2f\n",
               dataSent,
               0LL,
               (double)1e6*(double)dataSent/(double)usec,
               usec);
        fflush(stdout);
      }
      else TRACE_ERR((stderr, "end test protocol: %s, bytes %d \n", metas[nalg].name, i));
    }
  }
  result = PAMI_Context_destroyv(&context, 1);
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
  free(metas);
  TRACE_ERR((stderr, "%s DONE\n", __PRETTY_FUNCTION__));
  return 0;
};
