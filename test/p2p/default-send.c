///
/// \file test/p2p/default-send.c
/// \brief Simple point-topoint XMI_send() test
///

#include "sys/xmi.h"
#include <stdio.h>
static void recv_done (xmi_context_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called recv_done function.  active: %zd -> %zd\n", *active, *active-1);
  (*active)--;
}

static void test_dispatch (
    xmi_context_t        context,      /**< IN: XMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    size_t               task,         /**< IN: source task */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called dispatch function.  cookie = %p, active: %zd\n", cookie, *active);
  //(*active)--;
  //fprintf (stderr, "... dispatch function.  active = %zd\n", *active);

  recv->local_fn = recv_done;
  recv->cookie   = cookie;
  recv->kind = XMI_AM_KIND_SIMPLE;
  recv->data.simple.addr  = NULL;
  recv->data.simple.bytes = 0;
  fprintf (stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn);

  return;
}

static void send_done_local (xmi_context_t   context,
                             void          * cookie,
                             xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_local function.  active: %zd -> %zd\n", *active, *active-1);
  (*active)--;
}

static void send_done_remote (xmi_context_t   context,
                              void          * cookie,
                              xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_remote function.  active: %zd -> %zd\n", *active, *active-1);
  (*active)--;
  fprintf (stderr, "... send_done_remote function.  active = %zd\n", *active);
}

int main (int argc, char ** argv)
{
  volatile size_t send_active = 2;
  volatile size_t recv_active = 1;


  xmi_client_t client;
  xmi_context_t context;
  //xmi_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_create (client, NULL, 0, &context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query (context, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  fprintf (stderr, "My task id = %zd\n", task_id);

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query (context, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  fprintf (stderr, "Number of tasks = %zd\n", num_tasks);
  if (num_tasks != 2)
  {
    fprintf (stderr, "Error. This test requires 2 tasks. Number of tasks in this job: %zd\n", num_tasks);
    return 1;
  }

  size_t dispatch = 0;
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options;
  fprintf (stderr, "Before XMI_Dispatch_set() .. &recv_active = %p, recv_active = %zd\n", &recv_active, recv_active);
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

  xmi_send_simple_t parameters;
  parameters.send.dispatch = dispatch;
  parameters.send.cookie   = (void *) &send_active;
  parameters.send.header.addr = NULL;
  parameters.send.header.bytes = 0;
  parameters.simple.addr  = NULL;
  parameters.simple.bytes = 0;
  parameters.simple.local_fn  = send_done_local;
  parameters.simple.remote_fn = send_done_remote;

  if (task_id == 0)
  {
    fprintf (stderr, "before send ...\n");
    parameters.send.task = 1;
    result = XMI_Send (context, &parameters);
    fprintf (stderr, "... after send.\n");

    fprintf (stderr, "before send-recv advance loop ...\n");
    while (send_active || recv_active)
    {
      result = XMI_Context_advance (context, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
        return 1;
      }
    }
    fprintf (stderr, "... after send-recv advance loop\n");
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
    parameters.send.task = 0;
    result = XMI_Send (context, &parameters);
    fprintf (stderr, "... after send.\n");

    fprintf (stderr, "before send advance loop ...\n");
    while (send_active)
    {
      result = XMI_Context_advance (context, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
        return 1;
      }
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

  return 0;
};
