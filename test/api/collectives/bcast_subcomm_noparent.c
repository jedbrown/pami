/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/bcast_subcomm_noparent.c
 * \brief ???
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

#include <pami.h>

#define BUFSIZE 1048576
#include "../pami_util.h"


int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t       context;
  size_t               num_contexts = 1;
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
  pami_xfer_t          barrier;
  volatile unsigned    bar_poll_flag = 0;

  /* Bcast variables */
  size_t               bcast_num_algorithm[2];
  pami_algorithm_t    *bcast_always_works_algo = NULL;
  pami_metadata_t     *bcast_always_works_md = NULL;
  pami_algorithm_t    *bcast_must_query_algo = NULL;
  pami_metadata_t     *bcast_must_query_md = NULL;
  pami_xfer_type_t     bcast_xfer = PAMI_XFER_BROADCAST;
  volatile unsigned    bcast_poll_flag = 0;

  double               ti, tf, usec;
  char                 buf[BUFSIZE];

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


  /*  Create the subgeometry */
  pami_geometry_range_t *range;
  int                    rangecount;
  pami_geometry_t        newgeometry;
  size_t                 newbar_num_algo[2];
  size_t                 newbcast_num_algo[2];
  pami_algorithm_t      *newbar_algo        = NULL;
  pami_metadata_t       *newbar_md          = NULL;
  pami_algorithm_t      *q_newbar_algo      = NULL;
  pami_metadata_t       *q_newbar_md        = NULL;

  pami_algorithm_t      *newbcast_algo      = NULL;
  pami_metadata_t       *newbcast_md        = NULL;
  pami_algorithm_t      *q_newbcast_algo    = NULL;
  pami_metadata_t       *q_newbcast_md      = NULL;
  pami_xfer_t            newbarrier;
  pami_xfer_t            newbcast;

  size_t                 set[2];
  int                    id, root = 0;
  size_t                 half        = num_tasks / 2;
  range     = (pami_geometry_range_t *)malloc(((num_tasks + 1) / 2) * sizeof(pami_geometry_range_t));

  /* \note Test environment variable" TEST_VERBOSE=N     */
  char* sVerbose = getenv("TEST_VERBOSE");

  if(sVerbose) gVerbose=atoi(sVerbose); /* set the global defined in coll_util.h */

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


  char *method = getenv("TEST_SPLIT_METHOD");

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

  /* Delay root tasks, and emulate that he's doing "other" */
  /* message passing.  This will cause the geometry_create */
  /* request from other nodes to be unexpected. */
  if (task_id == root)
    {
      delayTest(1);
      PAMI_Context_advance (context, 1000);
    }

  rc = create_and_query_geometry(client,
                                 context,
                                 context,
                                 PAMI_NULL_GEOMETRY,
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

  rc = query_geometry(client,
                      context,
                      newgeometry,
                      bcast_xfer,
                      newbcast_num_algo,
                      &newbcast_algo,
                      &newbcast_md,
                      &q_newbcast_algo,
                      &q_newbcast_md);

  if (rc == 1)
    return 1;

  /*  Set up world barrier */
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];

  /*  Set up sub geometry barrier */
  newbarrier.cb_done   = cb_done;
  newbarrier.cookie    = (void*) & bar_poll_flag;
  newbarrier.algorithm = newbar_algo[0];

  /*  Set up sub geometry bcast */
  newbcast.cb_done                      = cb_done;
  newbcast.cookie                       = (void*) & bcast_poll_flag;
  newbcast.algorithm                    = newbcast_algo[0];
  newbcast.cmd.xfer_broadcast.root      = root;
  newbcast.cmd.xfer_broadcast.buf       = buf;
  newbcast.cmd.xfer_broadcast.type      = PAMI_TYPE_CONTIGUOUS;
  newbcast.cmd.xfer_broadcast.typecount = 0;


  int             i, j, k;

  for (k = 1; k >= 0; k--)
    {
      if (task_id == root)
        {
          printf("# Broadcast Bandwidth Test -- root = %d  protocol: %s\n", root, newbcast_md[0].name);
          printf("# Size(bytes)           cycles    bytes/sec    usec\n");
          printf("# -----------      -----------    -----------    ---------\n");
        }
      if(((strstr(newbcast_md[0].name,selected) == NULL) && selector) ||
         ((strstr(newbcast_md[0].name,selected) != NULL) && !selector))  continue;

      if (set[k])
        {
          fflush(stdout);
          blocking_coll(context, &newbarrier, &bar_poll_flag);

          for (i = 1; i <= BUFSIZE; i *= 2)
            {
              long long dataSent = i;
              int          niter = 100;
              blocking_coll(context, &newbarrier, &bar_poll_flag);
              ti = timer();

              for (j = 0; j < niter; j++)
                {
                  newbcast.cmd.xfer_broadcast.root      = root;
                  newbcast.cmd.xfer_broadcast.buf       = buf;
                  newbcast.cmd.xfer_broadcast.typecount = i;
                  blocking_coll(context, &newbcast, &bcast_poll_flag);
                }

              tf = timer();
              blocking_coll(context, &newbarrier, &bar_poll_flag);
              usec = (tf - ti) / (double)niter;

              if (task_id == root)
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

      blocking_coll(context, &barrier, &bar_poll_flag);
      blocking_coll(context, &barrier, &bar_poll_flag);
      fflush(stderr);
    }

  blocking_coll(context, &barrier, &bar_poll_flag);

  rc = pami_shutdown(&client, &context, &num_contexts);
  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);
  free(bcast_always_works_algo);
  free(bcast_always_works_md);
  free(bcast_must_query_algo);
  free(bcast_must_query_md);

  free(newbar_algo);
  free(newbar_md);
  free(q_newbar_algo);
  free(q_newbar_md);

  free(newbcast_algo);
  free(newbcast_md);
  free(q_newbcast_algo);
  free(q_newbcast_md);

  return 0;
}
