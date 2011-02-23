/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/api/collectives/barrier_subcomm.c
 * \brief Simple Barrier test on sub-geometries
 */


#include "../../../../api/pami_util.h"

#define NITERLAT   1   
#include <assert.h>

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context[PAMI_MAX_PROC_PER_NODE]; /* arbitrary max */
  size_t               num_contexts = 1;
  pami_task_t          task_id;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               num_algorithm[2];
  pami_algorithm_t    *always_works_algo = NULL;
  pami_metadata_t     *always_works_md = NULL;
  pami_algorithm_t    *must_query_algo = NULL;
  pami_metadata_t     *must_query_md = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  pami_xfer_t          barrier;
  volatile unsigned    poll_flag = 0;

  int                  nalg;

  /* \note Test environment variable" TEST_PROTOCOL={-}substring.       */
  /* substring is used to select, or de-select (with -) test protocols */
  unsigned selector = 1;
  char* selected = getenv("TEST_PROTOCOL");

  if (!selected) selected = "";
  else if (selected[0] == '-')
  {
    selector = 0 ;
    ++selected;
  }


  /* \note Test environment variable" TEST_PARENTLESS=0 or 1, defaults to 0.
     0 - world_geometry is the parent
     1 - parentless                                                      */
  char* sParentless = getenv("TEST_PARENTLESS");
  unsigned parentless = 0; /*Not parentless*/

  if (sParentless) parentless = atoi(sParentless);

  /* \note Test environment variable" TEST_NUM_CONTEXTS=N, defaults to 1.*/
  char* snum_contexts = getenv("TEST_NUM_CONTEXTS");

  if (snum_contexts) num_contexts = atoi(snum_contexts);

  assert(num_contexts > 0);
  assert(num_contexts <= PAMI_MAX_PROC_PER_NODE);

  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     context,        /* Context            */
                     NULL,           /* Clientname=default */
                     &num_contexts,  /* num_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */

  if (rc == 1)
    return 1;
  if (num_tasks < 4)
  {
    fprintf(stderr,"No barrier subcomms on 1 node\n");
    return 0;
  }

  unsigned iContext = 0;

  for (; iContext < num_contexts; ++iContext)
  {

    if (task_id == 0)
      printf("# Context: %u\n", iContext);

    rc |= query_geometry_world(client,
                              context[iContext],
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
    int                    id, root = 0, timeDelay = 0;
    size_t                 half        = num_tasks / 2;
    range     = (pami_geometry_range_t *)malloc(((num_tasks + 1) / 2) * sizeof(pami_geometry_range_t));

    char *method = getenv("TEST_SPLIT_METHOD");

    // Default or TEST_SPLIT_METHOD=0 : divide in half
    if ((!method || !strcmp(method, "0")))
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
    // TEST_SPLIT_METHOD=-1 : alternate ranks
    else if ((method && !strcmp(method, "-1")))
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
    // TEST_SPLIT_METHOD=N : Split the first "N" processes into a communicator
    else
    {
      half = atoi(method);
      if ((half <= 1) || (half >= (num_tasks-1)))
      {
        fprintf(stderr,"No barrier subcomms on 1 node\n");
        return 0;
      }
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

    if(root)
    {
      timeDelay = 2; // Need to stagger barriers between subcomm's
    }

    /* Delay root tasks, and emulate that he's doing "other"
       message passing.  This will cause the geometry_create
       request from other nodes to be unexpected when doing
       parentless geometries and won't affect parented.      */
    if (task_id == root)
    {
      delayTest(1);
      unsigned ii = 0;

      for (; ii < num_contexts; ++ii)
        PAMI_Context_advance (context[ii], 1000);
    }

    rc |= create_and_query_geometry(client,
                                   context[iContext],
                                   parentless ? PAMI_NULL_GEOMETRY : world_geometry,
                                   &newgeometry,
                                   range,
                                   rangecount,
                                   id + iContext, // Unique id for each context
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

    rc |= blocking_coll(context[iContext], &barrier, &poll_flag);

    if (rc == 1)
      return 1;

    int  k;

    for (k = 1; k >= 0; k--)
    {
      if (set[k])
      {

        for (nalg = 0; nalg < newbar_num_algo[0]; nalg++)
        {
          double ti, tf, usec;
          newbarrier.algorithm = newbar_algo[nalg];

          if (task_id == root)
          {
            printf("# Barrier Test -- context = %d, root = %d, protocol: %s\n",
                   iContext, root, newbar_md[nalg].name);
            printf("# -------------------------------------------------------------------\n");
          }

          if (((strstr(newbar_md[nalg].name, selected) == NULL) && selector) ||
              ((strstr(newbar_md[nalg].name, selected) != NULL) && !selector))  continue;

          if (task_id == root)
          {
            delayTest(timeDelay*nalg);
            fprintf(stderr, "Test set(%u):  Barrier protocol(%s) Correctness (%d of %zd algorithms)\n", k,
                    newbar_md[nalg].name, nalg + 1, newbar_num_algo[0]);
            ti = timer();
            blocking_coll(context[iContext], &newbarrier, &poll_flag);
            tf = timer();
            usec = tf - ti;

            if (usec < 1800000.0 || usec > 2200000.0)
            {  
              rc = 1;  
              fprintf(stderr, "%s FAIL: usec=%f want between %f and %f!\n",newbar_md[nalg].name,
                      usec, 1800000.0, 2200000.0);
            }
            else
              fprintf(stderr, "Barrier correct!\n");
          }
          else
          {
            delayTest(2+timeDelay*nalg);
            blocking_coll(context[iContext], &newbarrier, &poll_flag);
          }

          int niter = NITERLAT;
          blocking_coll(context[iContext], &newbarrier, &poll_flag);

          ti = timer();
          int i;

          for (i = 0; i < niter; i++)
            blocking_coll(context[iContext], &newbarrier, &poll_flag);

          tf = timer();
          usec = tf - ti;

          if (task_id == root)
            fprintf(stderr, "Test set(%u): Barrier protocol(%s) Performance: time=%f usec\n", k,
                    newbar_md[nalg].name, usec / (double)niter);
        }
      }
    }

    blocking_coll(context[iContext], &barrier, &poll_flag);

    free(always_works_algo);
    free(always_works_md);
    free(must_query_algo);
    free(must_query_md);

  } /*for(unsigned iContext = 0; iContext < num_contexts; ++iContexts)*/

  rc |= pami_shutdown(&client, context, &num_contexts);

  return rc;
};
