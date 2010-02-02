/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/multisend/multicombine.cc
 * \brief Simple multicombine tests.
 */


#include "test/multisend/Buffer.h"

#include "Global.h"
#include <unistd.h>

#ifndef TEST_BUF_SIZE
  #define TEST_BUF_SIZE	32
#endif // TEST_BUF_SIZE

static XMI::Test::Buffer<TEST_BUF_SIZE> _buffer;

static int           _doneCountdown;
xmi_callback_t       _cb_done;

void _done_cb(xmi_context_t context, void *cookie, xmi_result_t err)
{
  XMI_assertf(_doneCountdown > 0,"doneCountdown %d\n",_doneCountdown);
  volatile int *doneCountdown = (volatile int*) cookie;
  DBG_FPRINTF((stderr, "%s:%s doneCountdown %d/%d \n",__FILE__,__PRETTY_FUNCTION__, *doneCountdown,_doneCountdown));
  --*doneCountdown;
}

int main(int argc, char ** argv)
{
  unsigned x;
  xmi_client_t client;
  xmi_context_t context;
  xmi_result_t status = XMI_ERROR;

  status = XMI_Client_initialize("multicombine test", &client);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
    return 1;
  }
  DBG_FPRINTF((stderr,"Client %p\n",client));
  size_t n = 1;
  status = XMI_Context_createv(client, NULL, 0, &context, n);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  status = XMI_Configuration_query(client, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  DBG_FPRINTF((stderr, "My task id = %zu\n", task_id));

  configuration.name = XMI_NUM_TASKS;
  status = XMI_Configuration_query(client, &configuration);
  if(status != XMI_SUCCESS)
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

  XMI::Topology topology_global = __global.topology_global;
  
  xmi_multicombine_t multicombine;
  memset(&multicombine, 0x00, sizeof(multicombine));

  multicombine.connection_id = 0xB0BC; // arbitrary
  multicombine.data_participants = (xmi_topology_t *)&topology_global;
  multicombine.results_participants = (xmi_topology_t *)&topology_global;
  multicombine.count = TEST_BUF_SIZE/sizeof(unsigned);
  multicombine.data = (xmi_pipeworkqueue_t*) _buffer.srcPwq();
  multicombine.dtype = XMI_UNSIGNED_INT;
  multicombine.optor = XMI_MIN;
  multicombine.results = (xmi_pipeworkqueue_t*) _buffer.dstPwq();
  multicombine.client = client;	// client ID
  multicombine.context = 0;	// context ID
  multicombine.roles = -1;

  multicombine.cb_done = _cb_done;
// ------------------------------------------------------------------------
// simple multicombine 
// ------------------------------------------------------------------------
  _buffer.reset(true); // isRoot = true

  
  _doneCountdown = 1;
  //sleep(5); // instead of combineing

  status = XMI_Multicombine(&multicombine);

  while(_doneCountdown)
  {
    status = XMI_Context_advance (context, 10);
  }

  size_t
  bytesConsumed = 0,
  bytesProduced = 0;

  _buffer.validate(bytesConsumed,
                   bytesProduced,
                   true,   // isRoot = true
                   true);  // isDest = true
  if((bytesConsumed != TEST_BUF_SIZE) ||
     (bytesProduced != TEST_BUF_SIZE))
  {
    fprintf(stderr, "FAIL bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
  }
  else
    fprintf(stderr, "PASS bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);

// ------------------------------------------------------------------------
// simple multicombine root to all with one contributing '0' to the MIN allreduce
// ------------------------------------------------------------------------
  if(task_id == topology_global.index2Rank(0))
  {
    _buffer.resetMIN0(true); // isRoot = true so set to 0's
  }
  else
  {
    _buffer.resetMIN0(); 
  }

  
  _doneCountdown = 1;
  
  status = XMI_Multicombine(&multicombine);

  while(_doneCountdown)
  {
    status = XMI_Context_advance (context, 10);
  }

  size_t
  bytesConsumed = 0,
  bytesProduced = 0;

  _buffer.validateMIN0(bytesConsumed,
                   bytesProduced,
                   true,   // isRoot = true
                   true);  // isDest = true
  if((bytesConsumed != TEST_BUF_SIZE) ||
     (bytesProduced != TEST_BUF_SIZE))
  {
    fprintf(stderr, "FAIL bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
  }
  else
    fprintf(stderr, "PASS bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);

// ------------------------------------------------------------------------
  DBG_FPRINTF((stderr, "XMI_Context_destroy(context);\n"));
  status = XMI_Context_destroy(context);
  if(status != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy xmi context. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "XMI_Client_finalize(client);\n"));
  status = XMI_Client_finalize(client);
  if(status != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "return 0;\n"));
  return 0;
}
