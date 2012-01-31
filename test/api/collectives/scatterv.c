/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/scatterv.c
 * \brief Simple scatterv test
 */

#define COUNT     (524288)
/*
#define OFFSET     0
#define NITERLAT   1
#define NITERBW    MIN(10, niterlat/100+1)
#define CUTOFF     65536
*/

#include "../pami_util.h"

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_task_t          task_id, root_zero=0;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag = 0;

  /* Scatterv variables */
  size_t               scatterv_num_algorithm[2];
  pami_algorithm_t    *scatterv_always_works_algo = NULL;
  pami_metadata_t     *scatterv_always_works_md = NULL;
  pami_algorithm_t    *scatterv_must_query_algo = NULL;
  pami_metadata_t     *scatterv_must_query_md = NULL;
  pami_xfer_type_t     scatterv_xfer = PAMI_XFER_SCATTERV;
  volatile unsigned    scatterv_poll_flag = 0;

  int                  nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          scatterv;

  /* Process environment variables and setup globals */
  setup_env();

  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);


  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     context,        /* Context            */
                     NULL,           /* Clientname=default */
                     &gNum_contexts, /* gNum_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */

  if (rc == 1)
    return 1;

  /*  Allocate buffer(s) */
  int err = 0;
  void* buf = NULL;
  err = posix_memalign(&buf, 128, (gMax_count * num_tasks) + gBuffer_offset);
  assert(err == 0);
  buf = (char*)buf + gBuffer_offset;

  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, (gMax_count * num_tasks) + gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;

  size_t *lengths   = (size_t*)malloc(num_tasks * sizeof(size_t));
  assert(lengths);
  size_t *displs    = (size_t*)malloc(num_tasks * sizeof(size_t));
  assert(displs);


  unsigned iContext = 0;

  for (; iContext < gNum_contexts; ++iContext)
  {

    if (task_id == root_zero)
      printf("# Context: %u\n", iContext);

  /*  Query the world geometry for barrier algorithms */
  rc |= query_geometry_world(client,
                               context[iContext],
                            &world_geometry,
                            barrier_xfer,
                            barrier_num_algorithm,
                            &bar_always_works_algo,
                            &bar_always_works_md,
                            &bar_must_query_algo,
                            &bar_must_query_md);

  if (rc == 1)
    return 1;

  /*  Query the world geometry for scatterv algorithms */
  rc |= query_geometry_world(client,
                            context[iContext],
                            &world_geometry,
                            scatterv_xfer,
                            scatterv_num_algorithm,
                            &scatterv_always_works_algo,
                            &scatterv_always_works_md,
                            &scatterv_must_query_algo,
                            &scatterv_must_query_md);

  if (rc == 1)
    return 1;

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];
  blocking_coll(context[iContext], &barrier, &bar_poll_flag);

    for (nalg = 0; nalg < scatterv_num_algorithm[0]; nalg++)
    {
      pami_endpoint_t    root_ep;
      pami_task_t root_task = 0;
      PAMI_Endpoint_create(client, root_task, 0, &root_ep);
      scatterv.cmd.xfer_scatterv.root        = root_ep;
      scatterv.cb_done                       = cb_done;
      scatterv.cookie                        = (void*) & scatterv_poll_flag;
      scatterv.algorithm                     = scatterv_always_works_algo[nalg];
      scatterv.cmd.xfer_scatterv.sndbuf      = buf;
      scatterv.cmd.xfer_scatterv.stype       = PAMI_TYPE_BYTE;
      scatterv.cmd.xfer_scatterv.stypecounts = lengths;
      scatterv.cmd.xfer_scatterv.sdispls     = displs;
      scatterv.cmd.xfer_scatterv.rcvbuf      = rbuf;
      scatterv.cmd.xfer_scatterv.rtype       = PAMI_TYPE_BYTE;
      scatterv.cmd.xfer_scatterv.rtypecount  = 0;

      gProtocolName = scatterv_always_works_md[nalg].name;

      if (task_id == root_task)
      {
        printf("# Scatterv Bandwidth Test -- context = %d, protocol: %s\n",
               iContext, gProtocolName);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(scatterv_always_works_md[nalg].name,gSelected) == NULL) && gSelector) ||
          ((strstr(scatterv_always_works_md[nalg].name,gSelected) != NULL) && !gSelector))  continue;


      size_t i, j;

      for (i = gMin_count; i <= gMax_count; i *= 2)
      {
        size_t  dataSent = i;
        int          niter;
        size_t           k = 0;

        for (k = 0; k < num_tasks; k++)
        {
          lengths[k] = i;
          displs[k]  = 0;
        }

        if (dataSent < CUTOFF)
          niter = gNiterlat;
        else
          niter = NITERBW;

        blocking_coll(context[iContext], &barrier, &bar_poll_flag);
        ti = timer();

        for (j = 0; j < niter; j++)
        {
          scatterv.cmd.xfer_scatterv.rtypecount = i;
          blocking_coll(context[iContext], &scatterv, &scatterv_poll_flag);
        }

        tf = timer();
        blocking_coll(context[iContext], &barrier, &bar_poll_flag);

        usec = (tf - ti) / (double)niter;

        if (task_id == root_task)
        {
          printf("  %11lld %16d %14.1f %12.2f\n",
                 (long long)dataSent,
                 niter,
                 (double)1e6*(double)dataSent / (double)usec,
                 usec);
          fflush(stdout);
        }
      }
    }

  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(scatterv_always_works_algo);
  free(scatterv_always_works_md);
  free(scatterv_must_query_algo);
  free(scatterv_must_query_md);
  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  buf = (char*)buf - gBuffer_offset;
  free(buf);

  rbuf = (char*)rbuf - gBuffer_offset;
  free(rbuf);

  free(lengths);
  free(displs);

 rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}
