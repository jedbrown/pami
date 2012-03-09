/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/alltoallv_int_contig.c
 * \brief Simple Alltoallv_int test on world geometry with contiguous datatypes
 */

/* see setup_env() for environment variable overrides               */
#define COUNT     (4096)

#include "../pami_util.h"


int *sndlens = NULL;
int *sdispls = NULL;
int *rcvlens = NULL;
int *rdispls = NULL;

void initialize_sndbuf(size_t r, void *sbuf, void *rbuf, int dt)
{
  size_t k;

  if (dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *isbuf = (unsigned int *)  sbuf;
    unsigned int *irbuf = (unsigned int *)  rbuf;
    for (k = 0; k < sndlens[r]; k++)
    {
      isbuf[ sdispls[r] + k ] = ((r + k));
      irbuf[ rdispls[r] + k ] = 0xffffffff;
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *dsbuf = (double *)  sbuf;
    double *drbuf = (double *)  rbuf;

    for (k = 0; k < sndlens[r]; k++)
    {
      dsbuf[ sdispls[r] + k ] = ((double)((r + k))) * 1.0;
      drbuf[ sdispls[r] + k ] = 0xffffffffffffffff;
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *fsbuf = (float *)  sbuf;
    float *frbuf = (float *)  rbuf;

    for (k = 0; k < sndlens[r]; k++)
    {
      fsbuf[ sdispls[r] + k ] = ((float)((r + k))) * 1.0;
      frbuf[ sdispls[r] + k ] = 0xffffffffffffffff;
    }
  }
  else
  {
    char *csbuf = (char *)  sbuf;
    char *crbuf = (char *)  rbuf;

    for (k = 0; k < sndlens[r]; k++)
    {
      csbuf[ sdispls[r] + k ] = ((r + k) & 0xff);
      crbuf[ sdispls[r] + k ] = 0xff;
    }
  }
}

int check_rcvbuf(size_t sz, size_t myrank, void *rbuf, int dt)
{
  size_t r, k;

  if (dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *irbuf = (unsigned int *)rbuf;
    for (r = 0; r < sz; r++)
      for (k = 0; k < rcvlens[r]; k++)
      {
        if (irbuf[ rdispls[r] + k ] != (unsigned int)((myrank + k)))
        {
          fprintf(stderr, "%s:Check(%u) failed rbuf[%u+%zu]:%02x instead of %02zx (rank:%zu)\n",
                  gProtocolName, sndlens[r],
                  rdispls[r], k,
                  irbuf[ rdispls[r] + k ],
                  ((myrank + k)),
                  r );
        return 1;
        }
      }
  }
  else if (dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *drbuf = (double *)rbuf;
    for (r = 0; r < sz; r++)
      for (k = 0; k < rcvlens[r]; k++)
      {
        if (drbuf[ rdispls[r] + k ] != (double)(((myrank*1.0) + (k*1.0))))
        {
          fprintf(stderr, "%s:Check(%u) failed rbuf[%u+%zu]:%02f instead of %02zx (rank:%zu)\n",
                  gProtocolName, sndlens[r],
                  rdispls[r], k,
                  drbuf[ rdispls[r] + k ],
                  ((r + k)),
                  r );
        return 1;
        }
      }
  }
  if (dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *frbuf = (float *)rbuf;
    for (r = 0; r < sz; r++)
      for (k = 0; k < rcvlens[r]; k++)
      {
        if (frbuf[ rdispls[r] + k ] != (float)(((myrank*1.0) + (k*1.0))))
        {
          fprintf(stderr, "%s:Check(%u) failed rbuf[%u+%zu]:%02f instead of %02zx (rank:%zu)\n",
                  gProtocolName, sndlens[r],
                  rdispls[r], k,
                  frbuf[ rdispls[r] + k ],
                  ((r + k)),
                  r );
        return 1;
        }
      }
  }

  return 0;
}

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_task_t          task_id;
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

  /* alltoallv_int variables */
  size_t               alltoallv_int_num_algorithm[2];
  pami_algorithm_t    *alltoallv_int_always_works_algo = NULL;
  pami_metadata_t     *alltoallv_int_always_works_md = NULL;
  pami_algorithm_t    *next_algo = NULL;
  pami_metadata_t     *next_md= NULL;
  pami_algorithm_t    *alltoallv_int_must_query_algo = NULL;
  pami_metadata_t     *alltoallv_int_must_query_md = NULL;
  pami_xfer_type_t     alltoallv_int_xfer = PAMI_XFER_ALLTOALLV_INT;
  volatile unsigned    alltoallv_int_poll_flag = 0;

  int                  nalg= 0, total_alg;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          alltoallv_int;

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
  void* sbuf = NULL;
  err = posix_memalign((void*) & sbuf, 128, (gMax_byte_count * num_tasks) + gBuffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + gBuffer_offset;

  void* rbuf = NULL;
  err = posix_memalign((void*) & rbuf, 128, (gMax_byte_count * num_tasks) + gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;

  sndlens = (int*) malloc(num_tasks * sizeof(int));
  assert(sndlens);
  sdispls = (int*) malloc(num_tasks * sizeof(int));
  assert(sdispls);
  rcvlens = (int*) malloc(num_tasks * sizeof(int));
  assert(rcvlens);
  rdispls = (int*) malloc(num_tasks * sizeof(int));
  assert(rdispls);

  unsigned iContext = 0;

  for (; iContext < gNum_contexts; ++iContext)
  {

    if (task_id == 0)
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

    if (rc != PAMI_SUCCESS)
    return 1;

    int o;
    for(o = -1; o <= gOptimize ; o++) /* -1 = default, 0 = de-optimize, 1 = optimize */
    {

      pami_configuration_t configuration[1];
      configuration[0].name = PAMI_GEOMETRY_OPTIMIZE;
      configuration[0].value.intval = o; /* de/optimize */
      if(o == -1) ; /* skip update, use defaults */
      else
        rc |= update_geometry(client,
                              context[iContext],
                              world_geometry,
                              configuration,
                              1);

      if (rc != PAMI_SUCCESS)
      return 1;

    /*  Query the world geometry for alltoallv_int algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               alltoallv_int_xfer,
                               alltoallv_int_num_algorithm,
                               &alltoallv_int_always_works_algo,
                               &alltoallv_int_always_works_md,
                               &alltoallv_int_must_query_algo,
                               &alltoallv_int_must_query_md);

      if (rc != PAMI_SUCCESS)
      return 1;

    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];

    total_alg = alltoallv_int_num_algorithm[0]+alltoallv_int_num_algorithm[1];
    for (nalg = 0; nalg < total_alg; nalg++)
    {
      metadata_result_t result = {0};
      unsigned query_protocol;
      if(nalg < alltoallv_int_num_algorithm[0])
      {  
        query_protocol = 0;
        next_algo = &alltoallv_int_always_works_algo[nalg];
        next_md  = &alltoallv_int_always_works_md[nalg];
      }
      else
      {  
        query_protocol = 1;
        next_algo = &alltoallv_int_must_query_algo[nalg-alltoallv_int_num_algorithm[0]];
        next_md  = &alltoallv_int_must_query_md[nalg-alltoallv_int_num_algorithm[0]];
      }

      gProtocolName = next_md->name;

      alltoallv_int.cb_done    = cb_done;
      alltoallv_int.cookie     = (void*) & alltoallv_int_poll_flag;
      alltoallv_int.algorithm  = *next_algo;
      alltoallv_int.cmd.xfer_alltoallv_int.sndbuf        = sbuf;
      alltoallv_int.cmd.xfer_alltoallv_int.stype         = PAMI_TYPE_BYTE;
      alltoallv_int.cmd.xfer_alltoallv_int.stypecounts   = sndlens;
      alltoallv_int.cmd.xfer_alltoallv_int.sdispls       = sdispls;
      alltoallv_int.cmd.xfer_alltoallv_int.rcvbuf        = rbuf;
      alltoallv_int.cmd.xfer_alltoallv_int.rtype         = PAMI_TYPE_BYTE;
      alltoallv_int.cmd.xfer_alltoallv_int.rtypecounts   = rcvlens;
      alltoallv_int.cmd.xfer_alltoallv_int.rdispls       = rdispls;

      gProtocolName = next_md->name;

      if (task_id == 0)
      {
        printf("# Alltoallv_int Bandwidth Test(size:%zu) -- context = %d, optimize = %d, protocol: %s, Metadata: range %zu <-> %zd, mask %#X\n",num_tasks,
               iContext, o, gProtocolName,
               next_md->range_lo,(ssize_t)next_md->range_hi,
               next_md->check_correct.bitmask_correct);
        printf("# Size(bytes)  iterations    bytes/sec      usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(next_md->name, gSelected) == NULL) && gSelector) ||
          ((strstr(next_md->name, gSelected) != NULL) && !gSelector))  continue;

      int i, j;
      int dt,op=4/*SUM*/;

      unsigned checkrequired = next_md->check_correct.values.checkrequired; /*must query every time */
      assert(!checkrequired || next_md->check_fn); /* must have function if checkrequired. */

      for (dt = 0; dt < dt_count; dt++)
      {
          if (gValidTable[op][dt])
          {
            if (task_id == 0)
              printf("Running Alltoallv: %s\n", dt_array_str[dt]);

            for (i = MAX(1,gMin_byte_count/get_type_size(dt_array[dt])); i <= gMax_byte_count/get_type_size(dt_array[dt]); i *= 2)
            {
              size_t dataSent = i * get_type_size(dt_array[dt]);
              int          niter;

              if (dataSent < CUTOFF)
                niter = gNiterlat;
              else
                niter = NITERBW;

              for (j = 0; j < num_tasks; j++)
              {
                sndlens[j] = rcvlens[j] = i;
                sdispls[j] = rdispls[j] = i * j;

                initialize_sndbuf( j, sbuf, rbuf, dt );

              }
              alltoallv_int.cmd.xfer_alltoallv_int.rtype = dt_array[dt];
              alltoallv_int.cmd.xfer_alltoallv_int.stype = dt_array[dt];

              if(query_protocol)
              {  
                size_t sz=get_type_size(dt_array[dt])*i;
                result = check_metadata(*next_md,
                                        alltoallv_int,
                                        dt_array[dt],
                                        sz, /* metadata uses bytes i, */
                                        alltoallv_int.cmd.xfer_alltoallv_int.sndbuf,
                                        dt_array[dt],
                                        sz,
                                        alltoallv_int.cmd.xfer_alltoallv_int.rcvbuf);
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
                  result = next_md->check_fn(&alltoallv_int);
                  if (result.bitmask) continue;
                }
                blocking_coll(context[iContext], &alltoallv_int, &alltoallv_int_poll_flag);
              }

              tf = timer();
              blocking_coll(context[iContext], &barrier, &bar_poll_flag);

              int rc_check;
              rc |= rc_check = check_rcvbuf(num_tasks, task_id, rbuf, dt);

              if (rc_check) fprintf(stderr, "%s FAILED validation\n", gProtocolName);

              usec = (tf - ti) / (double)niter;

              if (task_id == 0)
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
    free(alltoallv_int_always_works_algo);
    free(alltoallv_int_always_works_md);
    free(alltoallv_int_must_query_algo);
    free(alltoallv_int_must_query_md);
    } /* optimize loop */
  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  sbuf = (char*)sbuf - gBuffer_offset;
  free(sbuf);

  rbuf = (char*)rbuf - gBuffer_offset;
  free(rbuf);

  free(sndlens);
  free(sdispls);
  free(rcvlens);
  free(rdispls);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}
