/**
 * \file sys/examples/endpoint_table.c
 * \brief ???
 */
pami_endpoint_t * _endpoint;

pami_result_t send_id (pami_context_t context, size_t target, pami_send_t * parameters)
{
  parameters->dest = _endpoint[target];
  return PAMI_Send (context, parameters);
}

int main ()
{
  pami_client_t client;
  pami_context_t context[4];
  size_t num_global_tasks;
  size_t num_global_endpoints;

  PAMI_Client_initialize ("name", &client, &num_global_tasks);

  PAMI_Context_createv (client, context, 4, num_global_endpoints);

  _endpoint = (pami_endpoint_t *) malloc (sizeof(pami_endpoint_t) * num_global_endpoints);

  size_t i, n;
  pami_endpoint_t * ptr = _endpoint;
  for (i=0; i<num_global_tasks; i++)
  {
    ptr += PAMI_Client_endpointv (client, i, ptr);
  }

  PAMI_Context_lock (context[0]);

  uint8_t header[16];
  uint8_t data[1024];

  volatile size_t active = 1;
  pami_send_t parameters;
  parameters.send.dispatch = 0;
  parameters.send.header.iov_base = header;
  parameters.send.header.iov_len  = 16;
  parameters.send.data.iov_base   = data;
  parameters.send.data.iov_len    = 1024;
  parameters.events.cookie   = (void *) &active;
  parameters.events.local_fn = decrement;

  /* Send a message to endpoint "42" */
  send_id (context[0], 42, parameters);

  while (active) PAMI_Context_advance (context[0]);

  PAMI_Context_unlock (context[0]);

  return 0;
}
