///
/// \file test/p2p/send/long-header.c
/// \brief "Long header" point-to-point PAMI_send() test
///

#include "sys/pami.h"
#include <stdio.h>

//#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

static void recv_done (pami_context_t   context,
                       void          * cookie,
                       pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called recv_done function.  active: %zu -> %zu\n", *active, *active-1));
  (*active)--;
}

static void test_dispatch (
    pami_context_t        context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_recv_t         * recv)        /**< OUT: receive message structure */
{
  TRACE((stderr, "Called dispatch function.  cookie = %p, active: %zu, header_addr = %p, header_size = %zu\n", cookie,  *((volatile size_t *) cookie), header_addr, header_size));

  recv->local_fn = recv_done;
  recv->cookie   = cookie;
  recv->kind = PAMI_AM_KIND_SIMPLE;
  recv->data.simple.addr  = NULL;
  recv->data.simple.bytes = 0;
  TRACE((stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn));

  return;
}

static void send_done_local (pami_context_t   context,
                             void          * cookie,
                             pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called send_done_local function.  active: %zu -> %zu\n", *active, *active-1));
  (*active)--;
}

static void send_done_remote (pami_context_t   context,
                              void          * cookie,
                              pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called send_done_remote function.  active: %zu -> %zu\n", *active, *active-1));
  (*active)--;
}

int main (int argc, char ** argv)
{
  volatile size_t send_active = 2;
  volatile size_t recv_active = 1;


  pami_client_t client;
  pami_context_t context;
  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_initialize (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

        { size_t _n = 1; result = PAMI_Context_createv(client, NULL, 0, &context, _n); }
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to create pami context. result = %d\n", result);
    return 1;
  }

  pami_configuration_t configuration;

  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  TRACE((stderr, "My task id = %zu\n", task_id));

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  TRACE((stderr, "Number of tasks = %zu\n", num_tasks));
  if (num_tasks != 2)
  {
    fprintf(stderr, "Error. This test requires 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
  }

  pami_send_hint_t options;
  pami_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;

  options.no_long_header = 0;
  TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
  result = PAMI_Dispatch_set (context,
                             0,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }

  options.no_long_header = 1;
  TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
  result = PAMI_Dispatch_set (context,
                             1,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }

  uint8_t header[10240];

  pami_send_t parameters;
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
    parameters.send.dest = PAMI_Client_endpoint (client, 1, 0);
    result = PAMI_Send (context, &parameters);
    if (result != PAMI_SUCCESS)
    {
        fprintf(stderr, "Error. Send using dispatch configured to enable long header support failed. result = %d\n", result);
        return 1;
    }
    TRACE((stderr, "... after send.\n"));

    TRACE((stderr, "before send-recv advance loop ...\n"));
    while (send_active || recv_active)
    {
      result = PAMI_Context_advance (context, 100);
      if (result != PAMI_SUCCESS)
      {
        fprintf(stderr, "Error. Unable to advance pami context. result = %d\n", result);
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
      result = PAMI_Context_advance (context, 100);
      if (result != PAMI_SUCCESS)
      {
        fprintf(stderr, "Error. Unable to advance pami context. result = %d\n", result);
        return 1;
      }
    }
    TRACE((stderr, "... after recv advance loop\n"));

    TRACE((stderr, "before send ...\n"));
    parameters.send.dest = PAMI_Client_endpoint (client, 0, 0);
    result = PAMI_Send (context, &parameters);
    if (result != PAMI_SUCCESS)
    {
        fprintf(stderr, "Error. Send using dispatch configured to enable long header support failed. result = %d\n", result);
        return 1;
    }
    TRACE((stderr, "... after send.\n"));

    TRACE((stderr, "before send advance loop ...\n"));
    while (send_active)
    {
      result = PAMI_Context_advance (context, 100);
      if (result != PAMI_SUCCESS)
      {
        fprintf(stderr, "Error. Unable to advance pami context. result = %d\n", result);
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
    parameters.send.dest = PAMI_Client_endpoint (client, 1, 0);
    parameters.send.dispatch = 1;
    result = PAMI_Send (context, &parameters);
    if (result != PAMI_INVAL)
    {
        fprintf(stderr, "Error. Long header send using dispatch configured to disable long header support did not return an error as expected. result = %d\n", result);
        return 1;
    }
    TRACE((stderr, "... after send.\n"));
  }


  result = PAMI_Context_destroy (context);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_finalize (client);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  return 0;
};
