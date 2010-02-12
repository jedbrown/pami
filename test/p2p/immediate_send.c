///
/// \file test/p2p/immediate_send.c
/// \brief Simple point-to-point XMI_Send_immediate() test
///

#include "sys/xmi.h"
#include <stdio.h>

static void test_dispatch (
    xmi_context_t        context,      /**< IN: XMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called dispatch function.  cookie = %p (active: %zu -> %zu), header_size = %zu, pipe_size = %zu\n", cookie, *active, *active-1, header_size, pipe_size);
  (*active)--;
  fprintf (stderr, ">>> [%zu] %s\n", header_size, (char *) header_addr);
  fprintf (stderr, ">>> [%zu] %s\n", pipe_size, (char *) pipe_addr);
  fprintf (stderr, "... dispatch function.\n");

  return;
}


int main (int argc, char ** argv)
{
  volatile size_t recv_active = 1;


  xmi_client_t client;
  xmi_context_t context;

  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

	{ size_t _n = 1; result = XMI_Context_createv(client, NULL, 0, &context, _n); }
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query(client, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  fprintf (stderr, "My task id = %zu\n", task_id);

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query(client, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  fprintf (stderr, "Number of tasks = %zu\n", num_tasks);
  if (num_tasks != 2)
  {
    fprintf (stderr, "Error. This test requires 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
  }

  size_t dispatch = 0;
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options={0};
  fprintf (stderr, "Before XMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active);
  result = XMI_Dispatch_set (context,
                             dispatch,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register xmi dispatch. result = %d\n", result);
    return 1;
  }

  char header_string[1024];
  char data_string[1024];
  size_t header_bytes = 1 + snprintf (header_string, 1024, "Header: This message is from task: %zu", task_id);
  size_t data_bytes   = 1 + snprintf (data_string, 1024, "Data: Hello!");

  xmi_send_immediate_t parameters;
  parameters.dispatch        = dispatch;
  parameters.header.iov_base = header_string;
  parameters.header.iov_len  = header_bytes;
  parameters.data.iov_base   = data_string;
  parameters.data.iov_len    = data_bytes;

  if (task_id == 0)
  {
    fprintf (stderr, "before send immediate ...\n");
    parameters.dest = XMI_Client_endpoint (client, 1, 0);
    result = XMI_Send_immediate (context, &parameters);
    fprintf (stderr, "... after send immediate.\n");

    fprintf (stderr, "before advance loop ...\n");
    while (recv_active != 0)
    {
      result = XMI_Context_advance (context, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
        return 1;
      }
    }
    fprintf (stderr, "... after advance loop\n");
  }
  else
  {
    fprintf (stderr, "before recv advance loop ...\n");
    while (recv_active != 0)
    {
      result = XMI_Context_advance (context, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
        return 1;
      }
    }
    fprintf (stderr, "... after recv advance loop\n");

    fprintf (stderr, "before send ...\n");
    parameters.dest = XMI_Client_endpoint (client, 0, 0);
    result = XMI_Send_immediate (context, &parameters);
    fprintf (stderr, "... after send.\n");

    fprintf (stderr, "before send advance loop ...\n");
    result = XMI_Context_advance (context, 100);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
      return 1;
    }
    fprintf (stderr, "... after send advance loop\n");
  }



  result = XMI_Context_destroy (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  fprintf (stderr, "\nTest Completed Successfully.\n");

  return 0;
};
