/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/scan.c
 * \brief Simple scan on world geometry
 */

/* see setup_env() for environment variable overrides
#define OFFSET     0
 */
#define FULL_TEST  1
#define COUNT      65536
#define NITERLAT   10

#include "../pami_util.h"


void initialize_sndbuf (void *buf, int count, int op, int dt, int task_id)
{

  int i;
  /* if (op == PAMI_SUM && dt == PAMI_UNSIGNED_INT) { */
  if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *ibuf = (unsigned int *)  buf;
    for (i = 0; i < count; i++)
    {
      ibuf[i] = i;
    }
  }
  else
  {
    size_t sz=get_type_size(dt_array[dt]);
    memset(buf,  task_id,  count * sz);
  }
}

int check_rcvbuf (void *buf, int count, int op, int dt, int num_tasks, int task_id)
{

  int i, err = 0;
  /*  if (op == PAMI_SUM && dt == PAMI_UNSIGNED_INT) { */
  if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *rbuf = (unsigned int *)  buf;
    for (i = 0; i < count; i++)
    {
      if (rbuf[i] != i * (task_id+1))
      {
        fprintf(stderr,"Check(%d) failed rbuf[%d] %u != %u\n",count,i,rbuf[i],i*(task_id+1));
        err = -1;
        return err;
      }
    }
  }

  return err;
}


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
  size_t               scan_num_algorithm[2];
  pami_algorithm_t    *scan_always_works_algo = NULL;
  pami_metadata_t     *scan_always_works_md = NULL;
  pami_algorithm_t    *scan_must_query_algo = NULL;
  pami_metadata_t     *scan_must_query_md = NULL;
  pami_xfer_type_t     scan_xfer = PAMI_XFER_SCAN;
  volatile unsigned    scan_poll_flag=0;

  int                  i, j, nalg = 0;
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
  err = posix_memalign(&sbuf, 128, gMax_count + gBuffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + gBuffer_offset;
  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, gMax_count + gBuffer_offset);
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

    for (nalg = 0; nalg < scan_num_algorithm[0]; nalg++)
    {
      if (task_id == task_zero) /* root not set yet */
      {
        printf("# Scan Bandwidth Test(size:%zu) -- context = %d, task_zero = %d protocol: %s\n",num_tasks,
               iContext, task_zero, scan_always_works_md[nalg].name);
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(scan_always_works_md[nalg].name, gSelected) == NULL) && gSelector) ||
          ((strstr(scan_always_works_md[nalg].name, gSelected) != NULL) && !gSelector))  continue;

      gProtocolName = scan_always_works_md[nalg].name;

      scan.cb_done   = cb_done;
      scan.cookie    = (void*)&scan_poll_flag;
      scan.algorithm = scan_always_works_algo[nalg];
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
            for (i = gMin_count; i <= gMax_count/get_type_size(dt_array[dt]); i *= 2)
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

              initialize_sndbuf (sbuf, i, op, dt, task_id);
              memset(rbuf, 0xFF, dataSent);

              /* We aren't testing barrier itself, so use context 0. */
              blocking_coll(context[0], &barrier, &bar_poll_flag);
              ti = timer();
              for (j=0; j<niter; j++)
              {
                blocking_coll(context[iContext], &scan, &scan_poll_flag);
              }
              tf = timer();
              /* We aren't testing barrier itself, so use context 0. */
              blocking_coll(context[0], &barrier, &bar_poll_flag);

              int rc_check;
              rc |= rc_check = check_rcvbuf (rbuf, i, op, dt, num_tasks, task_id);

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
