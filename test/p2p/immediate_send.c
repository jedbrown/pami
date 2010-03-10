///
/// \file test/p2p/immediate_send.c
/// \brief Simple point-to-point XMI_Send_immediate() test
///

#include "sys/xmi.h"
#include <stdio.h>

#define TEST_CROSSTALK

unsigned validate (void * addr, size_t bytes)
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
    xmi_context_t        context,      /**< IN: XMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
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


  xmi_client_t client;
  xmi_context_t context[2];

  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

#ifdef TEST_CROSSTALK
  result = XMI_Context_createv(client, NULL, 0, context, 2);
#else
  result = XMI_Context_createv(client, NULL, 0, context, 1);
#endif
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context(s). result = %d\n", result);
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
  xmi_task_t task_id = configuration.value.intval;
  fprintf (stderr, "My task id = %d\n", task_id);

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query(client, &configuration);
  if (result != XMI_SUCCESS)
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
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options={0};

  size_t i = 0;
#ifdef TEST_CROSSTALK
  for (i=0; i<2; i++)
#endif
  {
    fprintf (stderr, "Before XMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active);
    result = XMI_Dispatch_set (context[i],
                               dispatch,
                               fn,
                               (void *)&recv_active,
                               options);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable register xmi dispatch. result = %d\n", result);
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

  xmi_send_immediate_t parameters;
  parameters.dispatch        = dispatch;
  parameters.header.iov_base = header_string;
  parameters.data.iov_base   = data_string;

  if (task_id == 0)
  {
#ifdef TEST_CROSSTALK
    fprintf (stdout, "XMI_Send_immediate() functional test [crosstalk]\n");
    fprintf (stdout, "\n");
    parameters.dest = XMI_Client_endpoint (client, 1, 1);
#else
    fprintf (stdout, "XMI_Send_immediate() functional test\n");
    fprintf (stdout, "\n");
    parameters.dest = XMI_Client_endpoint (client, 1, 0);
#endif

    for (h=0; h<hsize; h++)
    {
      parameters.header.iov_len = header_bytes[h];
      for (p=0; p<psize; p++)
      {
        parameters.data.iov_len = data_bytes[p];

        fprintf (stderr, "before send immediate, {%zu, %zu}, endpoint destination: 0x%08x ...\n", h, p, parameters.dest);
        result = XMI_Send_immediate (context[0], &parameters);
        fprintf (stderr, "... after send immediate.\n");

        fprintf (stderr, "before advance loop ...\n");
        while (recv_active != 0)
        {
          result = XMI_Context_advance (context[0], 100);
          if (result != XMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
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

    parameters.dest = XMI_Client_endpoint (client, 0, 0);
    for (h=0; h<hsize; h++)
    {
      parameters.header.iov_len = header_bytes[h];
      for (p=0; p<psize; p++)
      {
        parameters.data.iov_len = data_bytes[p];
        fprintf (stderr, "before recv advance loop, context id = %zu ...\n", contextid);
        while (recv_active != 0)
        {
          result = XMI_Context_advance (context[contextid], 100);
          if (result != XMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
            return 1;
          }
        }
        recv_active = 1;
        fprintf (stderr, "... after recv advance loop\n");

        fprintf (stderr, "before send, context id = %zu ...\n", contextid);
        result = XMI_Send_immediate (context[contextid], &parameters);
        fprintf (stderr, "... after send.\n");

        fprintf (stderr, "before send advance loop, context id = %zu ...\n", contextid);
        result = XMI_Context_advance (context[contextid], 100);
        if (result != XMI_SUCCESS)
        {
          fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
          return 1;
        }
        fprintf (stderr, "... after send advance loop\n");
      }
    }
  }



  result = XMI_Context_destroy (context[0]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy xmi context. result = %d\n", result);
    return 1;
  }
#ifdef TEST_CROSSTALK
  result = XMI_Context_destroy (context[1]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy xmi context. result = %d\n", result);
    return 1;
  }
#endif

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  fprintf (stdout, "Success (%d)\n", task_id);

  return 0;
};
