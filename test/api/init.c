///
/// \file test/api/init.c
/// \brief Simple Initialization Test
///

#include "pami_util.h"

int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context;
  pami_result_t        result = PAMI_ERROR;
  size_t               num_contexts=1;
  pami_task_t          task_id=0;
  size_t               num_tasks=0;
  pami_geometry_t      world_geometry;

  int rc = pami_init(&client,        /* Client             */
                     &context,       /* Context            */
                     NULL,           /* Clientname=default */
                     &num_contexts,  /* num_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */


  fprintf(stderr, "Hello PAMI World:  task_id=%d, size=%zd\n",
          task_id,
          num_tasks);
  
  rc = pami_shutdown(&client,&context,&num_contexts); 
  if(rc==1)
    return 1;
  
  return 0;
};
