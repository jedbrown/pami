///
/// \file test/p2p/send/send_to_self.c
/// \brief "send to self" point-to-point PAMI_Send() test
///

#include <stdio.h>

#include <pami.h>

static void recv_done (pami_context_t   context,
                       void          * cookie,
                       pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called recv_done function.  active: %zu -> %zu\n", *active, *active-1);
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
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called dispatch function.  cookie = %p, active: %zu, header_addr = %p, pipe_addr = %p\n", cookie, *active, header_addr, pipe_addr);

  recv->local_fn = recv_done;
  recv->cookie   = cookie;
  recv->type     = PAMI_BYTE;
  recv->addr     = NULL;
  recv->offset   = 0;
  fprintf (stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn);

  return;
}

static void send_done_local (pami_context_t   context,
                             void          * cookie,
                             pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_local function.  active: %zu -> %zu\n", *active, *active-1);
  (*active)--;
}

#if 0
static void send_done_remote (pami_context_t   context,
                              void          * cookie,
                              pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_remote function.  active: %zu -> %zu\n", *active, *active-1);
  (*active)--;
}
#endif
int main (int argc, char ** argv)
{
  //volatile size_t send_active = 2;
  volatile size_t send_active = 1;
  volatile size_t recv_active = 1;


  pami_client_t client;
  pami_context_t context;
  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

    fprintf (stderr, "Before Client initialize\n");
  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }
    fprintf (stderr, "After Client initialize\n");

    fprintf (stderr, "before context createv\n");
        { size_t _n = 1; result = PAMI_Context_createv(client, NULL, 0, &context, _n); }
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
    return 1;
  }
    fprintf (stderr, "after context createv\n");

  pami_configuration_t configuration;

  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  fprintf (stderr, "My task id = %zu\n", task_id);

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  //size_t num_tasks = configuration.value.intval;

  size_t dispatch = 0;
  pami_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  pami_send_hint_t options={0};
  fprintf (stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active);
  result = PAMI_Dispatch_set (context,
                             dispatch,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }

  pami_send_t parameters;
  parameters.send.dispatch        = dispatch;
  parameters.send.header.iov_base = (void *)&dispatch; // send *something*
  parameters.send.header.iov_len  = sizeof(size_t);
  parameters.send.data.iov_base   = (void *)&dispatch; // send *something*
  parameters.send.data.iov_len    = sizeof(size_t);
  parameters.events.cookie    = (void *) &send_active;
  parameters.events.local_fn  = send_done_local;
  //parameters.events.remote_fn = send_done_remote;
  parameters.events.remote_fn = NULL;
#if 1
  int iter;
  for (iter=0; iter < 100; iter++)
  {
    fprintf (stderr, "before send ...\n");
    PAMI_Endpoint_create (client, task_id, 0, &parameters.send.dest);
    result = PAMI_Send (context, &parameters);
    fprintf (stderr, "... after send.\n");

    fprintf (stderr, "before send-recv advance loop (send_active = %zu, recv_active = %zu) ...\n", send_active, recv_active);
    while (send_active || recv_active)
    {
      result = PAMI_Context_advance (context, 100);
      if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
        return 1;
      }
    }
    fprintf (stderr, "... after send-recv advance loop\n");
        send_active = recv_active = 1;
  }
#endif
  result = PAMI_Context_destroy (context);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy (client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  return 0;
};
