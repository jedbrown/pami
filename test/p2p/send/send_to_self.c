///
/// \file test/p2p/send/send_to_self.c
/// \brief "send to self" point-to-point XMI_Send() test
///

#include <stdio.h>

#include "sys/xmi.h"

static void recv_done (xmi_context_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called recv_done function.  active: %zu -> %zu\n", *active, *active-1);
  (*active)--;
}

static void test_dispatch (
    xmi_context_t        context,      /**< IN: XMI context */
    size_t               contextid,    /**< IN: context identifier */
    void               * cookie,       /**< IN: dispatch cookie */
    xmi_task_t           task,         /**< IN: source task */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called dispatch function.  cookie = %p, active: %zu, header_addr = %p, pipe_addr = %p\n", cookie, *active, header_addr, pipe_addr);

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
  fprintf (stderr, "Called send_done_local function.  active: %zu -> %zu\n", *active, *active-1);
  (*active)--;
}

static void send_done_remote (xmi_context_t   context,
                              void          * cookie,
                              xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_remote function.  active: %zu -> %zu\n", *active, *active-1);
  (*active)--;
}

int main (int argc, char ** argv)
{
  volatile size_t send_active = 2;
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

	{ int _n = 1; result = XMI_Context_createv(client, NULL, 0, &context, &_n); }
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

  size_t dispatch = 0;
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options={0};
  fprintf (stderr, "Before XMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active);
  result = XMI_Dispatch_set (context,
                             dispatch,
                             (xmi_dispatch_callback_fn) fn,
                             (void *)&recv_active,
                             options);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register xmi dispatch. result = %d\n", result);
    return 1;
  }

  xmi_send_t parameters;
  parameters.send.dispatch        = dispatch;
  parameters.send.header.iov_base = (void *)&dispatch; // send *something*
  parameters.send.header.iov_len  = sizeof(size_t);
  parameters.send.data.iov_base   = (void *)&dispatch; // send *something*
  parameters.send.data.iov_len    = sizeof(size_t);
  parameters.events.cookie    = (void *) &send_active;
  parameters.events.local_fn  = send_done_local;
  parameters.events.remote_fn = send_done_remote;

  {
    fprintf (stderr, "before send ...\n");
    parameters.send.task = task_id;
    result = XMI_Send (context, &parameters);
    fprintf (stderr, "... after send.\n");

    fprintf (stderr, "before send-recv advance loop (send_active = %zu, recv_active = %zu) ...\n", send_active, recv_active);
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
