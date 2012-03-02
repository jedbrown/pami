/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/allgather_contig.c
 * \brief Simple Allgather test on world geometry with contiguous datatypes
 */

/* see setup_env() for environment variable overrides               */
#define COUNT     (524288/16)

#include "../pami_util.h"

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_task_t          task_id, task_zero=0;
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

  /* Allgather variables */
  size_t               allgather_num_algorithm[2];
  pami_algorithm_t    *next_algo = NULL;
  pami_metadata_t     *next_md= NULL;
  pami_algorithm_t    *allgather_always_works_algo = NULL;
  pami_metadata_t     *allgather_always_works_md = NULL;
  pami_algorithm_t    *allgather_must_query_algo = NULL;
  pami_metadata_t     *allgather_must_query_md = NULL;
  pami_xfer_type_t     allgather_xfer = PAMI_XFER_ALLGATHER;
  volatile unsigned    allgather_poll_flag = 0;

  int                  nalg= 0, total_alg;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          allgather;

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
  err = posix_memalign(&buf, 128, gMax_byte_count + gBuffer_offset);
  assert(err == 0);
  buf = (char*)buf + gBuffer_offset;

  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, (gMax_byte_count * num_tasks) + gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;


  unsigned iContext = 0;

  for (; iContext < gNum_contexts; ++iContext)
  {

    if (task_id == task_zero)
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

    /*  Query the world geometry for allgather algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               allgather_xfer,
                               allgather_num_algorithm,
                               &allgather_always_works_algo,
                               &allgather_always_works_md,
                               &allgather_must_query_algo,
                               &allgather_must_query_md);

    if (rc == 1)
      return 1;

    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];
    blocking_coll(context[iContext], &barrier, &bar_poll_flag);

    total_alg = allgather_num_algorithm[0]+allgather_num_algorithm[1];
    for (nalg = 0; nalg < total_alg; nalg++)
    {
      metadata_result_t result = {0};
      unsigned query_protocol;
      if(nalg < allgather_num_algorithm[0])
      {
        query_protocol = 0;
        next_algo = &allgather_always_works_algo[nalg];
        next_md  = &allgather_always_works_md[nalg];
      }
      else
      {
        query_protocol = 1;
        next_algo = &allgather_must_query_algo[nalg-allgather_num_algorithm[0]];
        next_md  = &allgather_must_query_md[nalg-allgather_num_algorithm[0]];
      }
      allgather.cb_done    = cb_done;
      allgather.cookie     = (void*) & allgather_poll_flag;
      allgather.algorithm  = *next_algo;
      allgather.cmd.xfer_allgather.sndbuf     = buf;
      allgather.cmd.xfer_allgather.stype      = PAMI_TYPE_BYTE;
      allgather.cmd.xfer_allgather.stypecount = 0;
      allgather.cmd.xfer_allgather.rcvbuf     = rbuf;
      allgather.cmd.xfer_allgather.rtype      = PAMI_TYPE_BYTE;
      allgather.cmd.xfer_allgather.rtypecount = 0;

      gProtocolName = next_md->name;

      if (task_id == task_zero)
      {
        printf("# Allgather Bandwidth Test(size:%zu) -- context = %d, protocol: %s, Metadata: range %zu <-> %zd, mask %#X\n",num_tasks,
               iContext, gProtocolName,
               next_md->range_lo,(ssize_t)next_md->range_hi,
               next_md->check_correct.bitmask_correct);
        printf("# Size(bytes)      iterations     bytes/sec      usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(next_md->name, gSelected) == NULL) && gSelector) ||
          ((strstr(next_md->name, gSelected) != NULL) && !gSelector))  continue;

      int i, j;

      unsigned checkrequired = next_md->check_correct.values.checkrequired; /*must query every time */
      assert(!checkrequired || next_md->check_fn); /* must have function if checkrequired. */

      int dt,op=4/*SUM*/;

      for (dt = 0; dt < dt_count; dt++)
      {
          if ((gFull_test && ((dt != DT_NULL) && (dt != DT_BYTE))) || gValidTable[op][dt])
          {
            if (task_id == task_zero)
              printf("Running Allgather: %s\n", dt_array_str[dt]);

              for (i = MAX(1,gMin_byte_count/get_type_size(dt_array[dt])); i <= gMax_byte_count/get_type_size(dt_array[dt]); i *= 2)
              {
                size_t dataSent = i * get_type_size(dt_array[dt]);
                int          niter;

                if (dataSent < CUTOFF)
                  niter = gNiterlat;
                else
                  niter = NITERBW;

                allgather.cmd.xfer_allgather.stype      = dt_array[dt];
                allgather.cmd.xfer_allgather.rtype      = dt_array[dt];
                allgather.cmd.xfer_allgather.stypecount = i;
                allgather.cmd.xfer_allgather.rtypecount = i;

                gather_initialize_sndbuf_dt (buf, i, task_id, dt);
                memset(rbuf, 0xFF, i);
                if(query_protocol)
                {
                  size_t sz=get_type_size(dt_array[dt])*i;
                  result = check_metadata(*next_md,
                                          allgather,
                                          dt_array[dt],
                                          sz, /* metadata uses bytes i, */
                                          allgather.cmd.xfer_allgather.sndbuf,
                                          dt_array[dt],
                                          sz,
                                          allgather.cmd.xfer_allgather.rcvbuf);
                  if (next_md->check_correct.values.nonlocal)
                  {
                    /* \note We currently ignore check_correct.values.nonlocal
                      because these tests should not have nonlocal differences (so far). */
                    result.check.nonlocal = 0;
                  }

                  if (result.bitmask) continue;
                }

                blocking_coll(context[iContext], &barrier, &bar_poll_flag);
                ti = timer();

                for (j = 0; j < niter; j++)
                {
                  if (checkrequired) /* must query every time */
                  {
                    result = next_md->check_fn(&allgather);
                    if (result.bitmask) continue;
                  }
                  blocking_coll (context[iContext], &allgather, &allgather_poll_flag);
                }

                tf = timer();
                blocking_coll(context[iContext], &barrier, &bar_poll_flag);

                int rc_check;
                rc_check = gather_check_rcvbuf_dt (num_tasks, rbuf, i, dt);

                if (rc_check) fprintf(stderr, "%s FAILED validation on %s\n", gProtocolName, dt_array_str[dt]);

                usec = (tf - ti) / (double)niter;

                if (task_id == task_zero)
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
      }
    }
    free(bar_always_works_algo);
    free(bar_always_works_md);
    free(bar_must_query_algo);
    free(bar_must_query_md);
    free(allgather_always_works_algo);
    free(allgather_always_works_md);
    free(allgather_must_query_algo);
    free(allgather_must_query_md);
  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  buf = (char*)buf - gBuffer_offset;
  free(buf);

  rbuf = (char*)rbuf - gBuffer_offset;
  free(rbuf);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}
