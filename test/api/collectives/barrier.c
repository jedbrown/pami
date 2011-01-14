/**
 * \file test/api/collectives/barrier.c
 * \brief Simple Barrier test
 */


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

  int                  nalg;

  /* \note Test environment variable" TEST_PROTOCOL={-}substring.       */
  /* substring is used to select, or de-select (with -) test protocols */
  unsigned selector = 1;
  char* selected = getenv("TEST_PROTOCOL");
  if(!selected) selected = "";
  else if(selected[0]=='-') 
  {
      selector = 0 ;
      ++selected;
  }


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

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & poll_flag;
  barrier.algorithm = always_works_algo[0];

  if(!(((strstr(always_works_md[0].name,selected) == NULL) && selector) ||
       ((strstr(always_works_md[0].name,selected) != NULL) && !selector)))
  {
    if (!task_id)
        fprintf(stderr, "Test Default Barrier(%s)\n", always_works_md[0].name);

    rc = blocking_coll(context, &barrier, &poll_flag);

    if (rc == 1)
        return 1;

    if (!task_id)
        fprintf(stderr, "Barrier Done (%s)\n", always_works_md[0].name);
  }

  for (nalg = 0; nalg < num_algorithm[0]; nalg++)
    {
      double ti, tf, usec;
      barrier.algorithm = always_works_algo[nalg];
      if (!task_id)
        {
          printf("# Barrier Test protocol: %s\n", always_works_md[nalg].name);
          printf("# -------------------------------------------------------------------\n");
        }
      if(((strstr(always_works_md[nalg].name,selected) == NULL) && selector) ||
         ((strstr(always_works_md[nalg].name,selected) != NULL) && !selector))  continue;

      if (!task_id)
        {
          fprintf(stderr, "Test Barrier protocol(%s) Correctness (%d of %zd algorithms)\n",
                  always_works_md[nalg].name, nalg + 1, num_algorithm[0]);
          ti = timer();
          blocking_coll(context, &barrier, &poll_flag);
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
          blocking_coll(context, &barrier, &poll_flag);
        }

      int niter = NITER;
      blocking_coll(context, &barrier, &poll_flag);

      ti = timer();
      int i;

      for (i = 0; i < niter; i++)
          blocking_coll(context, &barrier, &poll_flag);

      tf = timer();
      usec = tf - ti;

      if (!task_id)
      {
          fprintf(stderr, "Test Barrier protocol(%s) Performance: time=%f usec\n",
                  always_works_md[nalg].name, usec / (double)niter);
          delayTest(2);
      }

      blocking_coll(context, &barrier, &poll_flag);
    }

  rc = pami_shutdown(&client, &context, &num_contexts);

  if (rc == 1)
    return 1;

  free(always_works_algo);
  free(always_works_md);
  free(must_query_algo);
  free(must_query_md);
  return 0;
};
