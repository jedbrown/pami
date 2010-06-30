///
/// \file test/scatterv.c
/// \brief Simple Allgatherv test
///

#include "../pami_util.h"

#define BUFSIZE 524288

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context;
  pami_result_t        result = PAMI_ERROR;
  size_t               num_contexts=1;
  pami_configuration_t configuration;
  pami_task_t          task_id;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;
  int                  algo;
  
  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo;
  pami_metadata_t     *bar_always_works_md;
  pami_algorithm_t    *bar_must_query_algo;
  pami_metadata_t     *bar_must_query_md;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag=0;
  
  /* Scatterv variables */
  size_t               scatterv_num_algorithm[2];
  pami_algorithm_t    *scatterv_always_works_algo;
  pami_metadata_t     *scatterv_always_works_md;
  pami_algorithm_t    *scatterv_must_query_algo;
  pami_metadata_t     *scatterv_must_query_md;
  pami_xfer_type_t     scatterv_xfer = PAMI_XFER_SCATTERV;
  volatile unsigned    scatterv_poll_flag=0;

  int                  nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          scatterv;

  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     &context,       /* Context            */
                     NULL,           /* Clientname=default */
                     &num_contexts,  /* num_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */
  if(rc==1)
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
  if(rc==1)
    return 1;

  /*  Query the world geometry for scatterv algorithms */
  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            scatterv_xfer,
                            scatterv_num_algorithm,
                            &scatterv_always_works_algo,
                            &scatterv_always_works_md,
                            &scatterv_must_query_algo,
                            &scatterv_must_query_md);
  if(rc==1)
    return 1;


  char   *buf       = (char*)malloc(BUFSIZE*num_tasks);
  char   *rbuf      = (char*)malloc(BUFSIZE*num_tasks);
  size_t *lengths   = (size_t*)malloc(num_tasks*sizeof(size_t));
  size_t *displs    = (size_t*)malloc(num_tasks*sizeof(size_t));
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*)&bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];
  blocking_coll(context,&barrier,&bar_poll_flag);

  size_t root = 0;
  if (task_id == root)
      {
        printf("# Scatterv Bandwidth Test -- \n");
        printf("# Size(bytes)           cycles    bytes/sec    usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

  scatterv.cb_done                       = cb_done;
  scatterv.cookie                        = (void*)&scatterv_poll_flag;
  scatterv.algorithm                     = scatterv_always_works_algo[0];
  scatterv.cmd.xfer_scatterv.root        = root;
  scatterv.cmd.xfer_scatterv.sndbuf      = buf;
  scatterv.cmd.xfer_scatterv.stype       = PAMI_BYTE;
  scatterv.cmd.xfer_scatterv.stypecounts = lengths;
  scatterv.cmd.xfer_scatterv.sdispls     = displs;
  scatterv.cmd.xfer_scatterv.rcvbuf      = rbuf;
  scatterv.cmd.xfer_scatterv.rtype       = PAMI_BYTE;
  scatterv.cmd.xfer_scatterv.rtypecount  = 0;

  size_t i,j;
  for(i=1; i<=BUFSIZE; i*=2)
      {
        long long dataSent = i;
        size_t       niter = 100;
        size_t           k = 0;

        for(k=0;k<num_tasks;k++)
            {
              lengths[k] = i;
              displs[k]  = 0;
            }
        blocking_coll(context,&barrier,&bar_poll_flag);
        ti = timer();
        for (j=0; j<niter; j++)
            {
              scatterv.cmd.xfer_scatterv.rtypecount = i;
              blocking_coll(context, &scatterv,&scatterv_poll_flag);
            }
        tf = timer();
        blocking_coll(context,&barrier,&bar_poll_flag);

        usec = (tf - ti)/(double)niter;
        if (task_id == root)
            {
              printf("  %11lld %16lld %14.1f %12.2f\n",
                     dataSent,
                     0LL,
                     (double)1e6*(double)dataSent/(double)usec,
                     usec);
              fflush(stdout);
            }
      }

  rc = pami_shutdown(&client,&context,&num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(scatterv_always_works_algo);
  free(scatterv_always_works_md);
  free(scatterv_must_query_algo);
  free(scatterv_must_query_md);

  return 0;
};
