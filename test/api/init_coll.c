///
/// \file test/api/init_coll.c
/// \brief Simple Collective Initialization Test
///


#include "pami_util.h"

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context;
  size_t               num_contexts=1;
  pami_task_t          task_id;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               num_algorithm[2];
  pami_algorithm_t    *always_works_algo;
  pami_metadata_t     *always_works_md;
  pami_algorithm_t    *must_query_algo;
  pami_metadata_t     *must_query_md;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  pami_xfer_t          barrier;
  volatile unsigned    poll_flag=0;
  double               tf, ti;
  
  ti=timer();
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

  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            barrier_xfer,
                            num_algorithm,
                            &always_works_algo,
                            &always_works_md,
                            &must_query_algo,
                            &must_query_md);
  if(rc==1)
    return 1;
 
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & poll_flag;
  barrier.algorithm = always_works_algo[0];
  rc = blocking_coll(context, &barrier, &poll_flag);
  if(rc==1)
    return 1;

  
  fprintf(stderr, "Hello PAMI Collective World:  task_id=%d, size=%zd\n",
          task_id,
          num_tasks);

  
  rc = pami_shutdown(&client,&context,&num_contexts); 
  if(rc==1)
    return 1;
  tf=timer();
  free(always_works_algo);
  free(always_works_md);
  free(must_query_algo);
  free(must_query_md);
  return 0;
};
