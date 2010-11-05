/**
 * \file test/api/collectives/bcast.c
 * \brief Simple Bcast test
 */

#include "../pami_util.h"

/*define this if you want to validate the data */
#define CHECK_DATA

#define BUFSIZE (1048576*8)
#define NITER 100

char* protocolName;

void initialize_sndbuf (void *buf, int bytes)
{

  unsigned char c = 0x00;
  int i = bytes;
  unsigned char *cbuf = (unsigned char *)  buf;

  for (; i; i--)
    {
      cbuf[i-1] = c++;
    }
}

int check_rcvbuf (void *buf, int bytes)
{
  unsigned char c = 0x00;
  int i = bytes;
  unsigned char *cbuf = (unsigned char *)  buf;

  for (; i; i--)
    {
      if (cbuf[i-1] != c)
        {
          fprintf(stderr, "%s:Check(%d) failed <%p>buf[%d]=%.2u != %.2u \n", protocolName, bytes, buf, i - 1, cbuf[i-1], c);
          return -1;
        }

      c++;
    }

  return 0;
}

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context;
  size_t               num_contexts = 1;
  pami_task_t          task_id;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo;
  pami_metadata_t     *bar_always_works_md;
  pami_algorithm_t    *bar_must_query_algo;
  pami_metadata_t     *bar_must_query_md;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag = 0;

  /* Bcast variables */
  size_t               bcast_num_algorithm[2];
  pami_algorithm_t    *bcast_always_works_algo;
  pami_metadata_t     *bcast_always_works_md;
  pami_algorithm_t    *bcast_must_query_algo;
  pami_metadata_t     *bcast_must_query_md;
  pami_xfer_type_t     bcast_xfer = PAMI_XFER_BROADCAST;
  volatile unsigned    bcast_poll_flag = 0;

  int                  nalg= 0;
  double               ti, tf, usec;
  char                 buf[BUFSIZE];
  pami_xfer_t          barrier;
  pami_xfer_t          broadcast;


  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     &context,       /* Context            */
                     NULL,           /* Clientname=default */
                     &num_contexts,  /* num_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */

  if (rc == 1)
    return 1;

  /*  Query the world geometry for barrier algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            barrier_xfer,
                            barrier_num_algorithm,
                            &bar_always_works_algo,
                            &bar_always_works_md,
                            &bar_must_query_algo,
                            &bar_must_query_md);

  if (rc == 1)
    return 1;

  /*  Query the world geometry for broadcast algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            bcast_xfer,
                            bcast_num_algorithm,
                            &bcast_always_works_algo,
                            &bcast_always_works_md,
                            &bcast_must_query_algo,
                            &bcast_must_query_md);

  if (rc == 1)
    return 1;

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];
  blocking_coll(context, &barrier, &bar_poll_flag);

  for (nalg = 0; nalg < bcast_num_algorithm[0]; nalg++)
    {
      int         root = 0;
      broadcast.cb_done                      = cb_done;
      broadcast.cookie                       = (void*) & bcast_poll_flag;
      broadcast.algorithm                    = bcast_always_works_algo[nalg];
      broadcast.cmd.xfer_broadcast.root      = root;
      broadcast.cmd.xfer_broadcast.buf       = buf;
      broadcast.cmd.xfer_broadcast.type      = PAMI_BYTE;
      broadcast.cmd.xfer_broadcast.typecount = 0;

      protocolName = bcast_always_works_md[nalg].name;
      if (task_id == (size_t)root)
        {
          printf("# Broadcast Bandwidth Test -- root = %d  protocol: %s\n", root, protocolName);
          printf("# Size(bytes)           cycles    bytes/sec    usec\n");
          printf("# -----------      -----------    -----------    ---------\n");
        }
      //      if(strcmp(bcast_always_works_md[nalg].name,"RectangleP2PBroadcast")) continue;
      int i, j;

      for (i = 1; i <= BUFSIZE; i *= 2)
        {
          long long dataSent = i;
          int          niter = NITER;
#ifdef CHECK_DATA

          if (task_id == (size_t)root)
            initialize_sndbuf (buf, i);
          else
            memset(buf, 0xFF, i);

#endif
          blocking_coll(context, &barrier, &bar_poll_flag);
          ti = timer();

          for (j = 0; j < niter; j++)
            {
              broadcast.cmd.xfer_broadcast.typecount = i;
              blocking_coll (context, &broadcast, &bcast_poll_flag);
            }

          blocking_coll(context, &barrier, &bar_poll_flag);
          tf = timer();
#ifdef CHECK_DATA
          check_rcvbuf (buf, i);
#endif
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

  rc = pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(bcast_always_works_algo);
  free(bcast_always_works_md);
  free(bcast_must_query_algo);
  free(bcast_must_query_md);
  return 0;
};
