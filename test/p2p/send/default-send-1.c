///
/// \file test/p2p/send/default-send-1.c
/// \brief Ppoint-to-point XMI_send() test using multiple contexts
///

#include "sys/xmi.h"
#include <stdio.h>
#include <unistd.h>

//#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

xmi_client_t g_client;

static void recv_done (xmi_context_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called recv_done function.  active: %zu -> %zu\n", *active, *active-1));
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
  TRACE((stderr, "Called dispatch function.  cookie = %p, active: %zu\n", cookie, *active));

  recv->local_fn = recv_done;
  recv->cookie   = cookie;
  recv->kind = XMI_AM_KIND_SIMPLE;
  recv->data.simple.addr  = NULL;
  recv->data.simple.bytes = 0;
  TRACE((stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn));

  return;
}

static void send_done_local (xmi_context_t   context,
                             void          * cookie,
                             xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called send_done_local function.  active: %zu -> %zu\n", *active, *active-1));
  (*active)--;
}

static void send_done_remote (xmi_context_t   context,
                              void          * cookie,
                              xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;

  TRACE((stderr, "Called send_done_remote function.  active: %zu -> %zu\n", *active, *active-1));
  (*active)--;
  TRACE((stderr, "... send_done_remote function.  active = %zu\n", *active));
}


unsigned do_test (xmi_context_t context)
{
  volatile size_t send_active = 2;
  volatile size_t recv_active = 1;

  /* Lock the context */
  xmi_result_t result = XMI_Context_lock (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to lock the xmi context. result = %d\n", result);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query(g_client, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  //TRACE((stderr, "My task id = %zu\n", task_id));

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query(g_client, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  //TRACE((stderr, "Number of tasks = %zu\n", num_tasks));
  if (num_tasks != 2)
  {
    fprintf (stderr, "Error. This test requires 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
  }

  size_t dispatch = 0;
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options={0};
  //TRACE((stderr, "Before XMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
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


  xmi_send_t parameters;
  parameters.send.dispatch        = dispatch;
  parameters.send.header.iov_base = NULL;
  parameters.send.header.iov_len  = 0;
  parameters.send.data.iov_base   = NULL;
  parameters.send.data.iov_len    = 0;
  parameters.events.cookie        = (void *) &send_active;
  parameters.events.local_fn      = send_done_local;
  parameters.events.remote_fn     = send_done_remote;

  if (task_id == 0)
  {
    TRACE((stderr, "before send ...\n"));
    parameters.send.task = 1;
    result = XMI_Send (context, &parameters);
    TRACE((stderr, "... after send.\n"));

    TRACE((stderr, "before send-recv advance loop ...\n"));
    while (send_active || recv_active)
    {
      result = XMI_Context_advance (context, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
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
      result = XMI_Context_advance (context, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
        return 1;
      }
    }
    TRACE((stderr, "... after recv advance loop\n"));

    TRACE((stderr, "before send ...\n"));
    parameters.send.task = 0;
    result = XMI_Send (context, &parameters);
    TRACE((stderr, "... after send.\n"));

    TRACE((stderr, "before send advance loop ...\n"));
    while (send_active)
    {
      result = XMI_Context_advance (context, 100);
      if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
        return 1;
      }
    }
    TRACE((stderr, "... after send advance loop\n"));
  }

  /* Unlock the context */
  result = XMI_Context_unlock (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to unlock the xmi context. result = %d\n", result);
    return 1;
  }

  return 0;
};


int main (int argc, char ** argv)
{
  xmi_context_t context[2];
  //xmi_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &g_client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

  int num = 2;
  result = XMI_Context_createv (g_client, NULL, 0, &context[0], &num);
  if (result != XMI_SUCCESS || num != 2)
  {
    fprintf (stderr, "Error. Unable to create the two xmi context. result = %d\n", result);
    return 1;
  }

  /* Test pt-2-pt send on the first context */
  TRACE((stderr, "Before do_test(0)\n"));
  do_test (context[0]);
  TRACE((stderr, " After do_test(0)\n"));

  /* Test pt-2-pt send on the second context */
  TRACE((stderr, "Before do_test(1)\n"));
  do_test (context[1]);
  TRACE((stderr, " After do_test(1)\n"));


  result = XMI_Context_destroy (context[0]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy the first xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_destroy (context[1]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy the second xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Client_finalize (g_client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  fprintf (stderr, "Success.\n");
  return 0;
};
