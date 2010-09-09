/**
 * \file test/api/endpoint/endpoint_table.c
 * \brief ???
 */

#include <pami.h>
#include <stdio.h>

pami_endpoint_t * _endpoint;

static void createEndpointTable (pami_client_t client)
{
  pami_configuration_t configuration;
  configuration.name = PAMI_CLIENT_NUM_TASKS;
  pami_result_t result =
    PAMI_Client_query(client, &configuration,1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query PAMI_CLIENT_NUM_TASKS\n");
    abort();
  }

  size_t global_tasks = configuration.value.intval;

  _endpoint =
    (pami_endpoint_t *) malloc (sizeof(pami_endpoint_t) *
                                global_tasks * 4);

  size_t i, n, size = global_tasks * 4;
  pami_endpoint_t * ptr = _endpoint;
  n = size;
  for (i=0; i<global_tasks; i++)
  {
    PAMI_Endpoint_createv (client, i, ptr, &n);
    ptr += n;
    size -= n;
    n = size;
  }
};

static void test_dispatch (
    pami_context_t       context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_endpoint_t      origin,
    pami_recv_t        * recv)         /**< OUT: receive message structure */
{
  fprintf (stderr, "recv'd message from endpoint 0x%08x.\n", origin);
  volatile size_t * expect = (volatile size_t *) cookie;
  (*expect)--;
  return;
}

static void decrement (pami_context_t   context,
                       void           * cookie,
                       pami_result_t    result)
{
  volatile size_t * value = (volatile size_t *) cookie;
  (*value)--;
};


static pami_result_t send_endpoint (pami_context_t   context,
                                    size_t           target,
                                    pami_send_t    * parameters)
{
  parameters->send.dest = _endpoint[target];
  fprintf (stderr, "send message to endpoint 0x%08x.\n", parameters->send.dest);
  return PAMI_Send (context, parameters);
};

int main ()
{
  pami_client_t client;
  pami_context_t context[4];
  pami_result_t result;

  PAMI_Client_create ("name", &client, NULL, 0);

  /* Create four contexts - every task creates the same number */
  PAMI_Context_createv (client, NULL, 0, context, 4);

  createEndpointTable (client);


  pami_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  pami_send_hint_t options = {0};
  volatile size_t expect = 0;

  size_t i;
  for (i=0; i<4; i++)
  {
    PAMI_Context_lock (context[i]);
    result = PAMI_Dispatch_set (context[i], 0, fn, (void *)&expect, options);
    if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }
  }

  pami_configuration_t configuration;

  configuration.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query(client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  pami_task_t task_id = configuration.value.intval;
  fprintf (stderr, "My task id = %d\n", task_id);

  configuration.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query(client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  fprintf (stderr, "Number of tasks = %zu\n", num_tasks);
  if (num_tasks < 2)
  {
    fprintf (stderr, "Error. This test requires at least 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
  }

  if (task_id == 1) expect += num_tasks;

  uint8_t header[16];
  uint8_t data[1024];
  volatile size_t active = 1;

  pami_send_t parameters;
  parameters.send.dispatch        = 0;
  parameters.send.header.iov_base = header;
  parameters.send.header.iov_len  = 16;
  parameters.send.data.iov_base   = data;
  parameters.send.data.iov_len    = 1024;
  parameters.events.cookie        = (void *) &active;
  parameters.events.local_fn      = decrement;

  /* Send a message to endpoint "6" */
  send_endpoint (context[0], 6, &parameters);

  fprintf (stdout, "before advance, active = %zu, expect = %zu\n", active, expect);
  while ((active + expect) > 0) PAMI_Context_advancev (context, 4, 100);

  for (i=0; i<4; i++) PAMI_Context_unlock (context[i]);

  result = PAMI_Context_destroyv (context, 4);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy (&client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  fprintf (stdout, "Success (%d)\n", task_id);

  return 0;
};
