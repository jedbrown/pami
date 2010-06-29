/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/multisend/multisync_global.cc
 * \brief Simple multisync tests.
 */


#include "Global.h"
#include <unistd.h>

#define DBG_FPRINTF(x) //fprintf x


static int           _doneCountdown;
pami_callback_t       _cb_done;

void _done_cb(pami_context_t context, void *cookie, pami_result_t err)
{
  PAMI_assertf(_doneCountdown > 0,"doneCountdown %d\n",_doneCountdown);
  volatile int *doneCountdown = (volatile int*) cookie;
  DBG_FPRINTF((stderr, "%s:%s doneCountdown %d/%d \n",__FILE__,__PRETTY_FUNCTION__, *doneCountdown,_doneCountdown));
  --*doneCountdown;
}

int main(int argc, char ** argv)
{
  pami_client_t client;
  pami_context_t context;
  pami_result_t status = PAMI_ERROR;

  status = PAMI_Client_create("multisync test", &client);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", status);
    return 1;
  }
  DBG_FPRINTF((stderr,"Client %p\n",client));

  status = PAMI_Context_createv(client, NULL, 0, &context, 1);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", status);
    return 1;
  }

  pami_configuration_t configuration;

  configuration.name = PAMI_TASK_ID;
  status = PAMI_Configuration_query(client, &configuration);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  DBG_FPRINTF((stderr, "My task id = %zu\n", task_id));

  configuration.name = PAMI_NUM_TASKS;
  status = PAMI_Configuration_query(client, &configuration);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  if(task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------
  _cb_done.function   = &_done_cb;
  _cb_done.clientdata = &_doneCountdown;

  PAMI::Topology topology_global = __global.topology_global;

  pami_multisync_t multisync;
  memset(&multisync, 0x00, sizeof(multisync));

  multisync.connection_id = 0xB; // arbitrary
  multisync.participants = (pami_topology_t *)&topology_global;

  multisync.client = (size_t) client;	// client ID
  multisync.context = 0;	// context ID
  multisync.roles = -1;

  multisync.cb_done = _cb_done;
// ------------------------------------------------------------------------
// simple multisync root to all
// ------------------------------------------------------------------------

  _doneCountdown = 1;
  //sleep(5); // instead of syncing

  status = PAMI_Multisync(&multisync);

  while(_doneCountdown)
  {
    status = PAMI_Context_advance (context, 10);
  }

// ------------------------------------------------------------------------
  DBG_FPRINTF((stderr, "PAMI_Context_destroyv(&context, 1);\n"));
  status = PAMI_Context_destroyv(&context, 1);
  if(status != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy pami context. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "PAMI_Client_destroy(&client);\n"));
  status = PAMI_Client_destroy(&client);
  if(status != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "return 0;\n"));
  return 0;
}
