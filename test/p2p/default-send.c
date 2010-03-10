///
/// \file test/p2p/default-send.c
/// \brief Simple point-topoint XMI_send() test
///

#include "sys/xmi.h"
#include <stdio.h>

#define TEST_REMOTE_CALLBACK
#define TEST_CROSSTALK

uint8_t __recv_buffer[2048];
size_t __recv_size;
size_t __data_errors;

unsigned validate (void * addr, size_t bytes)
{
  unsigned status = 1;
  uint8_t * byte = (uint8_t *) addr;
  size_t i;
  for (i=0; i<bytes; i++)
  {
    if (byte[i] != (uint8_t)i)
    {
      fprintf (stderr, "validate(%p,%zu) .. ERROR .. byte[%zu] != %d (&byte[%zu] = %p, value is %d)\n", addr, bytes, i, (uint8_t)i, i, &byte[i], byte[i]);
      status = 0;
    }
  }

  return status;
}


static void recv_done (xmi_context_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called recv_done function.  active(%p): %zu -> %zu, __recv_size = %zu\n", active, *active, *active-1, __recv_size);

  if (!validate(__recv_buffer, __recv_size))
  {
    __data_errors++;
    fprintf (stderr, "validate data ERROR!\n");
  }

  (*active)--;
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
  fprintf (stderr, "Called dispatch function.  cookie = %p, active: %zu, header_size = %zu, pipe_size = %zu\n", cookie, *active, header_size, pipe_size);
  //(*active)--;
  //fprintf (stderr, "... dispatch function.  active = %zu\n", *active);

  if (!validate(header_addr, header_size))
    fprintf (stderr, "validate header ERROR!\n");

  if (pipe_size == 0)
  {
    (*active)--;
  }
  else
  {
    __recv_size = pipe_size;

    recv->local_fn = recv_done;
    recv->cookie   = cookie;
    recv->kind = XMI_AM_KIND_SIMPLE;
    recv->data.simple.addr  = __recv_buffer;
    recv->data.simple.bytes = pipe_size;
    //fprintf (stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn);
  }

  return;
}

static void send_done_local (xmi_context_t   context,
                             void          * cookie,
                             xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_local function.  active(%p): %zu -> %zu\n", active, *active, *active-1);
  (*active)--;
}

static void send_done_remote (xmi_context_t   context,
                              void          * cookie,
                              xmi_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_remote function.  active(%p): %zu -> %zu\n", active, *active, *active-1);
  (*active)--;
  //fprintf (stderr, "... send_done_remote function.  active = %zu\n", *active);
}

int main (int argc, char ** argv)
{
  volatile size_t send_active = 1;
#ifdef TEST_REMOTE_CALLBACK
  send_active++;
#endif
  volatile size_t recv_active = 1;

  __data_errors = 0;

  xmi_client_t client;
  xmi_context_t context[2];
  //xmi_configuration_t * configuration = NULL;
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

  uint8_t header[1024];
  uint8_t data[1024];
  for (i=0; i<1024; i++)
  {
    header[i] = (uint8_t)i;
    data[i]   = (uint8_t)i;
  }

  size_t h, hsize = 0;
  size_t header_bytes[16];
  header_bytes[hsize++] = 0;
  header_bytes[hsize++] = 16;
  header_bytes[hsize++] = 32;

  size_t p, psize = 0;
  size_t data_bytes[16];
  //data_bytes[psize++] = 0;
  //data_bytes[psize++] = 16;
  //data_bytes[psize++] = 32;
  //data_bytes[psize++] = 64;
  data_bytes[psize++] = 128;
  data_bytes[psize++] = 256;
  data_bytes[psize++] = 512;
  data_bytes[psize++] = 1024;

  xmi_send_t parameters;
  parameters.send.dispatch        = dispatch;
  parameters.send.header.iov_base = header;
  parameters.send.data.iov_base   = data;
  parameters.events.cookie        = (void *) &send_active;
  parameters.events.local_fn      = send_done_local;
#ifdef TEST_REMOTE_CALLBACK
  parameters.events.remote_fn     = send_done_remote;
#else
  parameters.events.remote_fn     = NULL;
#endif
  if (task_id == 0)
  {
#ifdef TEST_CROSSTALK
    fprintf (stdout, "XMI_Send() functional test [crosstalk]\n");
    fprintf (stdout, "\n");
    parameters.send.dest = XMI_Client_endpoint (client, 1, 1);
#else
    fprintf (stdout, "XMI_Send() functional test [no crosstalk]\n");
    fprintf (stdout, "\n");
    parameters.send.dest = XMI_Client_endpoint (client, 1, 0);
#endif

    for (h=0; h<hsize; h++)
    {
      parameters.send.header.iov_len = header_bytes[h];
      for (p=0; p<psize; p++)
      {
        parameters.send.data.iov_len = data_bytes[p];

        fprintf (stderr, "################################### %zu %zu\n", header_bytes[h], data_bytes[p]);
        fprintf (stderr, "before send ...\n");
        result = XMI_Send (context[0], &parameters);
        fprintf (stderr, "... after send.\n");

        fprintf (stderr, "before send-recv advance loop ... &send_active = %p, &recv_active = %p\n", &send_active, &recv_active);
        while (send_active || recv_active)
        {
          result = XMI_Context_advance (context[0], 100);
          if (result != XMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
            return 1;
          }
        }
        send_active = 1;
#ifdef TEST_REMOTE_CALLBACK
        send_active++;
#endif
        recv_active = 1;
        fprintf (stderr, "... after send-recv advance loop\n");
      }
    }
  }
  else
  {
#ifdef TEST_REMOTE_CALLBACK
    size_t contextid = 1;
#else
    size_t contextid = 0;
#endif
    parameters.send.dest = XMI_Client_endpoint (client, 0, 0);

    for (h=0; h<hsize; h++)
    {
      parameters.send.header.iov_len = header_bytes[h];
      for (p=0; p<psize; p++)
      {
        parameters.send.data.iov_len = data_bytes[p];

        fprintf (stderr, "################################### %zu %zu\n", header_bytes[h], data_bytes[p]);
        fprintf (stderr, "before recv advance loop ... &recv_active = %p\n", &recv_active);
        while (recv_active != 0)
        {
          result = XMI_Context_advance (context[contextid], 100);
          if (result != XMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
            return 1;
          }
          fprintf (stderr, "------ recv advance loop ... &recv_active = %p\n", &recv_active);
        }
        recv_active = 1;
        fprintf (stderr, "... after recv advance loop\n");

        fprintf (stderr, "before send ...\n");
        result = XMI_Send (context[contextid], &parameters);
        fprintf (stderr, "... after send.\n");

        fprintf (stderr, "before send advance loop ... &send_active = %p\n", &send_active);
        while (send_active)
        {
          result = XMI_Context_advance (context[contextid], 100);
          if (result != XMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
            return 1;
          }
          fprintf (stderr, "------ send advance loop ... &send_active = %p\n", &send_active);
        }
        send_active = 1;
#ifdef TEST_REMOTE_CALLBACK
        send_active++;
#endif
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

  if (__data_errors > 0)
  {
    fprintf (stdout, "Error. %zu data errors on task %d\n", __data_errors, task_id);
  }
  else
  {
    fprintf (stdout, "Success (%d)\n", task_id);
  }


  return 0;
};
