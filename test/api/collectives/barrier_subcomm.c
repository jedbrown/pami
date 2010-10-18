/*/ */
/*/ \file test/api/collectives/barrier_subcomm.c */
/*/ \brief Simple Barrier test */
/*/ */


#include "../pami_util.h"

#define NITER 100
#include <assert.h>

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context;
  size_t               num_contexts = 1;
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
  volatile unsigned    poll_flag = 0;

  int                  algo;

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

  rc = query_geometry_world(client,
                            context,
                            &world_geometry,
                            barrier_xfer,
                            num_algorithm,
                            &always_works_algo,
                            &always_works_md,
                            &must_query_algo,
                            &must_query_md);

  if (rc == 1)
    return 1;

  /*  Create the subgeometry */
  pami_geometry_range_t *range;
  int                    rangecount;
  pami_geometry_t        newgeometry;
  size_t                 newbar_num_algo[2];
  pami_algorithm_t      *newbar_algo        = NULL;
  pami_metadata_t       *newbar_md          = NULL;
  pami_algorithm_t      *q_newbar_algo      = NULL;
  pami_metadata_t       *q_newbar_md        = NULL;

  pami_xfer_t            newbarrier;

  size_t                 set[2];
  int                    id, root = 0;
  size_t                 half        = num_tasks / 2;
  range     = (pami_geometry_range_t *)malloc(((num_tasks + 1) / 2) * sizeof(pami_geometry_range_t));

  char *method = getenv("BARRIER_TEST_SPLIT_METHOD");

  if (!(method && !strcmp(method, "1")))
  {
    if (task_id >= 0 && task_id <= half - 1)
    {
      range[0].lo = 0;
      range[0].hi = half - 1;
      set[0]   = 1;
      set[1]   = 0;
      id       = 1;
      root     = 0;
    }
    else
    {
      range[0].lo = half;
      range[0].hi = num_tasks - 1;
      set[0]   = 0;
      set[1]   = 1;
      id       = 2;
      root     = half;
    }

    rangecount = 1;
  }
  else
  {
    int i = 0;
    int iter = 0;;

    if ((task_id % 2) == 0)
    {
      for (i = 0; i < num_tasks; i++)
      {
        if ((i % 2) == 0)
        {
          range[iter].lo = i;
          range[iter].hi = i;
          iter++;
        }
      }

      set[0]   = 1;
      set[1]   = 0;
      id       = 2;
      root     = 0;
      rangecount = iter;
    }
    else
    {
      for (i = 0; i < num_tasks; i++)
      {
        if ((i % 2) != 0)
        {
          range[iter].lo = i;
          range[iter].hi = i;
          iter++;
        }
      }

      set[0]   = 0;
      set[1]   = 1;
      id       = 2;
      root     = 1;
      rangecount = iter;
    }

  }

  rc = create_and_query_geometry(client,
                                 context,
                                 world_geometry,
                                 &newgeometry,
                                 range,
                                 rangecount,
                                 id,
                                 barrier_xfer,
                                 newbar_num_algo,
                                 &newbar_algo,
                                 &newbar_md,
                                 &q_newbar_algo,
                                 &q_newbar_md);

  if (rc == 1)
    return 1;


  /*  Set up world barrier */
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & poll_flag;
  barrier.algorithm = always_works_algo[0];

  /*  Set up sub geometry barrier */
  newbarrier.cb_done   = cb_done;
  newbarrier.cookie    = (void*) & poll_flag;
  newbarrier.algorithm = newbar_algo[0];

  rc = blocking_coll(context, &barrier, &poll_flag);

  if (rc == 1)
    return 1;

  int  k;

  for (k = 1; k >= 0; k--)
  {
    if (set[k])
    {

      for (algo = 0; algo < newbar_num_algo[0]; algo++)
      {
        double ti, tf, usec;
        newbarrier.algorithm = newbar_algo[algo];

        if (task_id == root)
        {
          fprintf(stderr, "Test set(%u):  Barrier protocol(%s) Correctness (%d of %zd algorithms)\n",k,
                  newbar_md[algo].name, algo + 1, newbar_num_algo[0]);
          ti = timer();
          blocking_coll(context, &newbarrier, &poll_flag);
          tf = timer();
          usec = tf - ti;

          if (usec < 1800000.0 || usec > 2200000.0)
            fprintf(stderr, "Barrier error: usec=%f want between %f and %f!\n",
                    usec, 1800000.0, 2200000.0);
          else
            fprintf(stderr, "Barrier correct!\n");
        }
        else
        {
          delayTest(2);
          blocking_coll(context, &newbarrier, &poll_flag);
        }

        int niter = NITER;
        blocking_coll(context, &newbarrier, &poll_flag);

        ti = timer();
        int i;

        for (i = 0; i < niter; i++)
          blocking_coll(context, &newbarrier, &poll_flag);

        tf = timer();
        usec = tf - ti;

        if (task_id == root)
          fprintf(stderr, "Test set(%u): Barrier protocol(%s) Performance: time=%f usec\n",k,
                  newbar_md[algo].name, usec / (double)niter);
      }
    }
  }

  blocking_coll(context, &barrier, &poll_flag);

  rc = pami_shutdown(&client, &context, &num_contexts);

  if (rc == 1)
    return 1;

  free(always_works_algo);
  free(always_works_md);
  free(must_query_algo);
  free(must_query_md);
  return 0;
};
