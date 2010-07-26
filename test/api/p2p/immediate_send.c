///
/// \file test/api/p2p/immediate_send.c
/// \brief Simple point-to-point PAMI_Send_immediate() test
///

#include <pami.h>
#include <stdio.h>

//#define TEST_CROSSTALK

//#define USE_SHMEM_OPTION
//#define NO_SHMEM_OPTION


unsigned validate (const void * addr, size_t bytes)
{
  unsigned status = 1;
  uint8_t * byte = (uint8_t *) addr;
  uint8_t i;
  for (i=0; i<bytes; i++)
  {
    if (byte[i] != i)
    {
      fprintf (stderr, "validate(%p,%zu) .. ERROR .. byte[%d] != %d (value is %d)\n", addr, bytes, i, i, byte[i]);
      status = 0;
    }
  }

  return status;
}


static void test_dispatch (
    pami_context_t        context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_endpoint_t origin,
pami_recv_t         * recv)        /**< OUT: receive message structure */
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called dispatch function.  cookie = %p (active: %zu -> %zu), header_size = %zu, pipe_size = %zu, recv = %p\n", cookie, *active, *active-1, header_size, pipe_size, recv);
  (*active)--;
  fprintf (stderr, ">>> [%zu] %s\n", header_size, (char *) header_addr);
  fprintf (stderr, ">>> [%zu] %s\n", pipe_size, (char *) pipe_addr);

  if (validate (header_addr, header_size))
    fprintf (stderr, ">>> header validated.\n");
  else
    fprintf (stderr, ">>> header ERROR !!\n");

  if (validate (pipe_addr, pipe_size))
    fprintf (stderr, ">>> payload validated.\n");
  else
    fprintf (stderr, ">>> payload ERROR !!\n");


  fprintf (stderr, "... dispatch function.\n");

  return;
}


int main (int argc, char ** argv)
{
  volatile size_t recv_active = 1;


  pami_client_t client;
  pami_context_t context[2];

  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client, NULL, 0);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

#ifdef TEST_CROSSTALK
  size_t num = 2;
#else
  size_t num = 1;
#endif
  result = PAMI_Context_createv(client, NULL, 0, context, num);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context(s). result = %d\n", result);
    return 1;
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
  if (num_tasks != 2)
  {
    fprintf (stderr, "Error. This test requires 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
  }

  size_t dispatch = 0;
  pami_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  pami_send_hint_t options={};

#ifdef USE_SHMEM_OPTION
  options.use_shmem = PAMI_HINT3_FORCE_ON;
  fprintf (stderr, "##########################################\n");
  fprintf (stderr, "shared memory optimizations forced ON\n");
  fprintf (stderr, "##########################################\n");
#elif defined(NO_SHMEM_OPTION)
  options.use_shmem = PAMI_HINT3_FORCE_OFF;
  fprintf (stderr, "##########################################\n");
  fprintf (stderr, "shared memory optimizations forced OFF\n");
  fprintf (stderr, "##########################################\n");
#endif

  size_t i = 0;
#ifdef TEST_CROSSTALK
  for (i=0; i<2; i++)
#endif
  {
    fprintf (stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active);
    result = PAMI_Dispatch_set (context[i],
                               dispatch,
                               fn,
                               (void *)&recv_active,
                               options);
    if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }
  }

  char header_string[1024];
  char data_string[1024];

  for (i=0; i<1024; i++)
  {
    header_string[i] = (char)i;
    data_string[i]   = (char)i;
  }

  size_t h, hsize = 0;
  size_t header_bytes[16];
  header_bytes[hsize++] = 0;
  header_bytes[hsize++] = 16;
  header_bytes[hsize++] = 32;

  size_t p, psize = 0;
  size_t data_bytes[16];
  data_bytes[psize++] = 0;
  data_bytes[psize++] = 16;
  data_bytes[psize++] = 32;

  pami_send_immediate_t parameters;
  parameters.dispatch        = dispatch;
  parameters.header.iov_base = header_string;
  parameters.data.iov_base   = data_string;

  if (task_id == 0)
  {
#ifdef TEST_CROSSTALK
    fprintf (stdout, "PAMI_Send_immediate() functional test [crosstalk]\n");
    fprintf (stdout, "\n");
    PAMI_Endpoint_create (client, 1, 1, &parameters.dest);
#else
    fprintf (stdout, "PAMI_Send_immediate() functional test\n");
    fprintf (stdout, "\n");
    PAMI_Endpoint_create (client, 1, 0, &parameters.dest);
#endif

    for (h=0; h<hsize; h++)
    {
      parameters.header.iov_len = header_bytes[h];
      for (p=0; p<psize; p++)
      {
        parameters.data.iov_len = data_bytes[p];

        fprintf (stderr, "before send immediate, {%zu, %zu}, endpoint destination: 0x%08x ...\n", h, p, parameters.dest);
        result = PAMI_Send_immediate (context[0], &parameters);
        fprintf (stderr, "... after send immediate.\n");

        fprintf (stderr, "before advance loop ...\n");
        while (recv_active != 0)
        {
          result = PAMI_Context_advance (context[0], 100);
          if (result != PAMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
            return 1;
          }
        }
        recv_active = 1;
        fprintf (stderr, "... after advance loop\n");
      }
    }
  }
  else
  {
#ifdef TEST_CROSSTALK
    size_t contextid = 1;
#else
    size_t contextid = 0;
#endif

    PAMI_Endpoint_create (client, 0, 0, &parameters.dest);
    for (h=0; h<hsize; h++)
    {
      parameters.header.iov_len = header_bytes[h];
      for (p=0; p<psize; p++)
      {
        parameters.data.iov_len = data_bytes[p];
        fprintf (stderr, "before recv advance loop, context id = %zu ...\n", contextid);
        while (recv_active != 0)
        {
          result = PAMI_Context_advance (context[contextid], 100);
          if (result != PAMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
            return 1;
          }
        }
        recv_active = 1;
        fprintf (stderr, "... after recv advance loop\n");

        fprintf (stderr, "before send, context id = %zu ...\n", contextid);
        result = PAMI_Send_immediate (context[contextid], &parameters);
        fprintf (stderr, "... after send.\n");

        fprintf (stderr, "before send advance loop, context id = %zu ...\n", contextid);
        result = PAMI_Context_advance (context[contextid], 100);
        if (result != PAMI_SUCCESS)
        {
          fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
          return 1;
        }
        fprintf (stderr, "... after send advance loop\n");
      }
    }
  }



  result = PAMI_Context_destroyv(context, num);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy(&client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  fprintf (stdout, "Success (%d)\n", task_id);

  return 0;
};
