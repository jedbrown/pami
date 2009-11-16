///
/// \file test/p2p/send/long-header.c
/// \brief "Long header" point-to-point XMI_send() test
///

#include "sys/xmi.h"
#include <stdio.h>

//#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

static void recv_done (xmi_client_t client, size_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called recv_done function.  active: %zd -> %zd\n", *active, *active-1));
  (*active)--;
}

static void test_dispatch (
    xmi_context_t        context,      /**< IN: XMI context */
    size_t               contextid,
    void               * cookie,       /**< IN: dispatch cookie */
    xmi_task_t           task,         /**< IN: source task */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called dispatch function.  cookie = %p, active: %zd, header_addr = %p, header_size = %zd\n", cookie, *active, header_addr, header_size));

  recv->local_fn = recv_done;
  recv->cookie   = cookie;
  recv->kind = XMI_AM_KIND_SIMPLE;
  recv->data.simple.addr  = NULL;
  recv->data.simple.bytes = 0;
  TRACE((stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn));

  return;
}

static void send_done_local (xmi_client_t client, size_t   context,
                             void          * cookie,
                             xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called send_done_local function.  active: %zd -> %zd\n", *active, *active-1));
  (*active)--;
}

static void send_done_remote (xmi_client_t client, size_t   context,
                              void          * cookie,
                              xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called send_done_remote function.  active: %zd -> %zd\n", *active, *active-1));
  (*active)--;
}

int main (int argc, char ** argv)
{
  volatile size_t send_active = 2;
  volatile size_t recv_active = 1;


  xmi_client_t client;
  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_create(client, NULL, 0, 1);
  if (result != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to create xmi context. result = %d\n", result);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query (client, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  TRACE((stderr, "My task id = %zd\n", task_id));

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query (client, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  TRACE((stderr, "Number of tasks = %zd\n", num_tasks));
  if (num_tasks != 2)
  {
    fprintf(stderr, "Error. This test requires 2 tasks. Number of tasks in this job: %zd\n", num_tasks);
    return 1;
  }

  xmi_send_hint_t options;
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;

  options.no_long_header = 0;
  TRACE((stderr, "Before XMI_Dispatch_set() .. &recv_active = %p, recv_active = %zd\n", &recv_active, recv_active));
  result = XMI_Dispatch_set (client, 0,
                             0,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable register xmi dispatch. result = %d\n", result);
    return 1;
  }

  options.no_long_header = 1;
  TRACE((stderr, "Before XMI_Dispatch_set() .. &recv_active = %p, recv_active = %zd\n", &recv_active, recv_active));
  result = XMI_Dispatch_set (client, 0,
                             1,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable register xmi dispatch. result = %d\n", result);
    return 1;
  }

  uint8_t header[10240];

  xmi_send_t parameters;
  parameters.send.dispatch        = 0;
  parameters.send.header.iov_base = (void *) header;
  parameters.send.header.iov_len  = 10240;
  parameters.send.data.iov_base   = NULL;
  parameters.send.data.iov_len    = 0;
  parameters.events.cookie        = (void *) &send_active;
  parameters.events.local_fn      = send_done_local;
  parameters.events.remote_fn     = send_done_remote;

  if (task_id == 0)
  {
    TRACE((stderr, "before send ...\n"));
    parameters.send.task = 1;
    result = XMI_Send (client, 0, &parameters);
    if (result != XMI_SUCCESS)
    {
        fprintf(stderr, "Error. Send using dispatch configured to enable long header support failed. result = %d\n", result);
        return 1;
    }
    TRACE((stderr, "... after send.\n"));

    TRACE((stderr, "before send-recv advance loop ...\n"));
    while (send_active || recv_active)
    {
      result = XMI_Context_advance (client, 0, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf(stderr, "Error. Unable to advance xmi context. result = %d\n", result);
        return 1;
      }
    }
    TRACE((stderr, "... after send-recv advance loop\n"));
  }
  else
  {
    TRACE((stderr, "before recv advance loop ...\n"));
    while (recv_active != 0)
    {
      result = XMI_Context_advance (client, 0, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf(stderr, "Error. Unable to advance xmi context. result = %d\n", result);
        return 1;
      }
    }
    TRACE((stderr, "... after recv advance loop\n"));

    TRACE((stderr, "before send ...\n"));
    parameters.send.task = 0;
    result = XMI_Send (client, 0, &parameters);
    if (result != XMI_SUCCESS)
    {
        fprintf(stderr, "Error. Send using dispatch configured to enable long header support failed. result = %d\n", result);
        return 1;
    }
    TRACE((stderr, "... after send.\n"));

    TRACE((stderr, "before send advance loop ...\n"));
    while (send_active)
    {
      result = XMI_Context_advance (client, 0, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf(stderr, "Error. Unable to advance xmi context. result = %d\n", result);
        return 1;
      }
    }
    TRACE((stderr, "... after send advance loop\n"));
  }

  /* ********************
   * Test error path .. dispatch configured without long header support used
   * to send a long header message.
   * *******************/
  if (task_id == 0)
  {
    TRACE((stderr, "before send ...\n"));
    parameters.send.task = 1;
    parameters.send.dispatch = 1;
    result = XMI_Send (client, 0, &parameters);
    if (result != XMI_INVAL)
    {
        fprintf(stderr, "Error. Long header send using dispatch configured to disable long header support did not return an error as expected. result = %d\n", result);
        return 1;
    }
    TRACE((stderr, "... after send.\n"));
  }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  return 0;
};
