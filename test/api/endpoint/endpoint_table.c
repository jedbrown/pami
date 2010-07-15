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
  configuration.name = PAMI_NUM_TASKS;
  pami_result_t result =
    PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query PAMI_NUM_TASKS\n");
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
  return PAMI_Send (context, parameters);
};

int main ()
{
  pami_client_t client;
  pami_context_t context[4];

  PAMI_Client_create ("name", &client);

  PAMI_Context_createv (client, NULL, 0, context, 4);

  createEndpointTable (client);

  PAMI_Context_lock (context[0]);

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

  /* Send a message to endpoint "42" */
  send_endpoint (context[0], 42, &parameters);

  while (active) PAMI_Context_advance (context[0], 100);

  PAMI_Context_unlock (context[0]);

  return 0;
};
