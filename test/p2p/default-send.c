///
/// \file test/p2p/default-send.c
/// \brief Simple point-topoint PAMI_send() test
///

#include "sys/pami.h"
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


static void recv_done (pami_context_t   context,
                       void          * cookie,
                       pami_result_t    result)
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
    pami_context_t        context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_recv_t         * recv)        /**< OUT: receive message structure */
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
    recv->kind = PAMI_AM_KIND_SIMPLE;
    recv->data.simple.addr  = __recv_buffer;
    recv->data.simple.bytes = pipe_size;
    //fprintf (stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn);
  }

  return;
}

static void send_done_local (pami_context_t   context,
                             void          * cookie,
                             pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_local function.  active(%p): %zu -> %zu\n", active, *active, *active-1);
  (*active)--;
}

static void send_done_remote (pami_context_t   context,
                              void          * cookie,
                              pami_result_t    result)
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

  pami_client_t client;
  pami_context_t context[2];
  //pami_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_initialize (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

#ifdef TEST_CROSSTALK
  result = PAMI_Context_createv(client, NULL, 0, context, 2);
#else
  result = PAMI_Context_createv(client, NULL, 0, context, 1);
#endif
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
    return 1;
  }

  pami_configuration_t configuration;

  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  pami_task_t task_id = configuration.value.intval;
  fprintf (stderr, "My task id = %d\n", task_id);

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
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
  pami_send_hint_t options={0};
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

  pami_send_t parameters;
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
    fprintf (stdout, "PAMI_Send() functional test [crosstalk]\n");
    fprintf (stdout, "\n");
    parameters.send.dest = PAMI_Client_endpoint (client, 1, 1);
#else
    fprintf (stdout, "PAMI_Send() functional test [no crosstalk]\n");
    fprintf (stdout, "\n");
    parameters.send.dest = PAMI_Client_endpoint (client, 1, 0);
#endif

    for (h=0; h<hsize; h++)
    {
      parameters.send.header.iov_len = header_bytes[h];
      for (p=0; p<psize; p++)
      {
        parameters.send.data.iov_len = data_bytes[p];

        fprintf (stderr, "################################### %zu %zu\n", header_bytes[h], data_bytes[p]);
        fprintf (stderr, "before send ...\n");
        result = PAMI_Send (context[0], &parameters);
        fprintf (stderr, "... after send.\n");

        fprintf (stderr, "before send-recv advance loop ... &send_active = %p, &recv_active = %p\n", &send_active, &recv_active);
        while (send_active || recv_active)
        {
          result = PAMI_Context_advance (context[0], 100);
          if (result != PAMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
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
    parameters.send.dest = PAMI_Client_endpoint (client, 0, 0);

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
          result = PAMI_Context_advance (context[contextid], 100);
          if (result != PAMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
            return 1;
          }
          fprintf (stderr, "------ recv advance loop ... &recv_active = %p\n", &recv_active);
        }
        recv_active = 1;
        fprintf (stderr, "... after recv advance loop\n");

        fprintf (stderr, "before send ...\n");
        result = PAMI_Send (context[contextid], &parameters);
        fprintf (stderr, "... after send.\n");

        fprintf (stderr, "before send advance loop ... &send_active = %p\n", &send_active);
        while (send_active)
        {
          result = PAMI_Context_advance (context[contextid], 100);
          if (result != PAMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
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



  result = PAMI_Context_destroy (context[0]);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }
#ifdef TEST_CROSSTALK
  result = PAMI_Context_destroy (context[1]);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }
#endif

  result = PAMI_Client_finalize (client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
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
