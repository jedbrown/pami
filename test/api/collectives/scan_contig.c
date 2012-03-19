/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/scan_contig.c
 * \brief Simple scan on world geometry with contiguous datatypes
 */

/* see setup_env() for environment variable overrides               */
#define FULL_TEST  1
#define COUNT      65536
#define NITERLAT   10

#include "../pami_util.h"


int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_task_t          task_id,task_zero=0;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag=0;

  /* Scan variables */
  pami_algorithm_t    *next_algo = NULL;
  pami_metadata_t     *next_md= NULL;
  size_t               scan_num_algorithm[2];
  pami_algorithm_t    *scan_always_works_algo = NULL;
  pami_metadata_t     *scan_always_works_md = NULL;
  pami_algorithm_t    *scan_must_query_algo = NULL;
  pami_metadata_t     *scan_must_query_md = NULL;
  pami_xfer_type_t     scan_xfer = PAMI_XFER_SCAN;
  volatile unsigned    scan_poll_flag=0;

  int                  i, j, nalg = 0, total_alg;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          scan;

  /* Process environment variables and setup globals */
  setup_env();

  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);

  /*  Allocate buffer(s) */
  int err = 0;
  void* sbuf = NULL;
  err = posix_memalign(&sbuf, 128, gMax_byte_count + gBuffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + gBuffer_offset;
  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, gMax_byte_count + gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;

  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     context,        /* Context            */
                     NULL,           /* Clientname=default */
                     &gNum_contexts, /* gNum_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */

  if (rc==1)
    return 1;

  /*  Query the world geometry for barrier algorithms */
  rc |= query_geometry_world(client,
                             context[0],
                             &world_geometry,
                             barrier_xfer,
                             barrier_num_algorithm,
                             &bar_always_works_algo,
                             &bar_always_works_md,
                             &bar_must_query_algo,
                             &bar_must_query_md);

  if (rc==1)
    return 1;

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];

  unsigned iContext = 0;

  for (; iContext < gNum_contexts; ++iContext)
  {

    if (task_id == 0)
      printf("# Context: %u\n", iContext);

    /*  Query the world geometry for scan algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               scan_xfer,
                               scan_num_algorithm,
                               &scan_always_works_algo,
                               &scan_always_works_md,
                               &scan_must_query_algo,
                               &scan_must_query_md);
    if (rc==1)
      return 1;
    total_alg = scan_num_algorithm[0]+scan_num_algorithm[1];
    for (nalg = 0; nalg < total_alg; nalg++)
    {
      metadata_result_t result = {0};
      unsigned query_protocol;
      if(nalg < scan_num_algorithm[0])
      {  
        query_protocol = 0;
        next_algo = &scan_always_works_algo[nalg];
        next_md  = &scan_always_works_md[nalg];
      }
      else
      {  
        query_protocol = 1;
        next_algo = &scan_must_query_algo[nalg-scan_num_algorithm[0]];
        next_md  = &scan_must_query_md[nalg-scan_num_algorithm[0]];
      }
      if (task_id == task_zero) /* root not set yet */
      {
        printf("# Scan Bandwidth Test(size:%zu) -- context = %d, protocol: %s, Metadata: range %zu <-> %zd, mask %#X\n",num_tasks,
               iContext, next_md->name,
               next_md->range_lo,(ssize_t)next_md->range_hi,
               next_md->check_correct.bitmask_correct);
        printf("# Size(bytes)      iterations     bytes/sec      usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(next_md->name, gSelected) == NULL) && gSelector) ||
          ((strstr(next_md->name, gSelected) != NULL) && !gSelector))  continue;

      gProtocolName = next_md->name;

      unsigned checkrequired = next_md->check_correct.values.checkrequired; /*must query every time */
      assert(!checkrequired || next_md->check_fn); /* must have function if checkrequired. */

      scan.cb_done   = cb_done;
      scan.cookie    = (void*)&scan_poll_flag;
      scan.algorithm = *next_algo;
      scan.cmd.xfer_scan.sndbuf    = sbuf;
      scan.cmd.xfer_scan.rcvbuf    = rbuf;
      scan.cmd.xfer_scan.rtype     = PAMI_TYPE_BYTE;
      scan.cmd.xfer_scan.rtypecount= 0;
      scan.cmd.xfer_scan.exclusive = 0;

      int op, dt;

      for (dt=0; dt<dt_count; dt++)
        for (op=0; op<op_count; op++)
        {
          if (gValidTable[op][dt])
          {
            if (task_id == task_zero)
              printf("Running Scan: %s, %s\n",dt_array_str[dt], op_array_str[op]);
            for (i = MAX(1,gMin_byte_count/get_type_size(dt_array[dt])); i <= gMax_byte_count/get_type_size(dt_array[dt]); i *= 2)
            {
              size_t sz=get_type_size(dt_array[dt]);
              size_t  dataSent = i * sz;
              int niter;

              if (dataSent < CUTOFF)
                niter = gNiterlat;
              else
                niter = NITERBW;

              scan.cmd.xfer_scan.stypecount = i;
              scan.cmd.xfer_scan.rtypecount = i;
              scan.cmd.xfer_scan.stype      = dt_array[dt];
              scan.cmd.xfer_scan.rtype      = dt_array[dt];
              scan.cmd.xfer_scan.op         = op_array[op];

              if(query_protocol)
              {  
                size_t sz=get_type_size(dt_array[dt])*i;
                result = check_metadata(*next_md,
                                      scan,
                                      dt_array[dt],
                                      sz, /* metadata uses bytes i, */
                                      scan.cmd.xfer_scan.sndbuf,
                                      dt_array[dt],
                                      sz,
                                      scan.cmd.xfer_scan.rcvbuf);
                if (next_md->check_correct.values.nonlocal)
                {
                  /* \note We currently ignore check_correct.values.nonlocal
                     because these tests should not have nonlocal differences (so far). */
                  result.check.nonlocal = 0;
                }

                if (result.bitmask) continue;
              }

              scan_initialize_sndbuf (sbuf, i, op, dt, task_id);
              memset(rbuf, 0xFF, dataSent);

              /* We aren't testing barrier itself, so use context 0. */
              blocking_coll(context[0], &barrier, &bar_poll_flag);
              ti = timer();
              for (j=0; j<niter; j++)
              {
                if (checkrequired) /* must query every time */
                {
                  result = next_md->check_fn(&scan);
                  if (result.bitmask) continue;
                }

                blocking_coll(context[iContext], &scan, &scan_poll_flag);
              }
              tf = timer();
              /* We aren't testing barrier itself, so use context 0. */
              blocking_coll(context[0], &barrier, &bar_poll_flag);

              int rc_check;
              rc |= rc_check = scan_check_rcvbuf (rbuf, i, op, dt, num_tasks, task_id, 0);

              if (rc_check) fprintf(stderr, "%s FAILED validation\n", gProtocolName);

              usec = (tf - ti)/(double)niter;
              if (task_id == task_zero)
              {
                printf("  %11lld %16d %14.1f %12.2f\n",
                       (long long)dataSent,
                       niter,
                       (double)1e6*(double)dataSent/(double)usec,
                       usec);
                fflush(stdout);
              }
            }
          }
        }
    }
    free(scan_always_works_algo);
    free(scan_always_works_md);
    free(scan_must_query_algo);
    free(scan_must_query_md);

  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);

  sbuf = (char*)sbuf - gBuffer_offset;
  free(sbuf);
  rbuf = (char*)rbuf - gBuffer_offset;
  free(rbuf);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}
