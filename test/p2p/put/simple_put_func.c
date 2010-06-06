///
/// \file test/p2p/put/simple_put_func.c
/// \brief Simple point-to-point PAMI_Get() test
///
/// This test implements a very simple "ping-ack" communication and
/// depends on a functional PAMI_Send_immediate() function.
///

#include <pami.h>
#include <stdio.h>
#include <stdint.h>

//#define TEST_CROSSTALK

//#define USE_SHMEM_OPTION
//#define NO_SHMEM_OPTION

#define DISPATCH_ID 10

#define BUFFERSIZE 16

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

typedef struct
{
  pami_endpoint_t   origin;
  void            * dst;
} info_t;

typedef struct
{
  pami_endpoint_t   origin;
  size_t            bytes;
  size_t            pad;
  uint32_t          buffer[BUFFERSIZE<<1];
  volatile size_t * value;
} get_info_t;

volatile size_t  _nonzero;
volatile size_t  _done;

static void done (pami_context_t   context,
                  void           * cookie,
                  pami_result_t    result)
{
  size_t * value = (size_t *) cookie;

  TRACE_ERR((stderr, ">> done() cookie = %p (value = %d => %d), result = %zu\n", cookie, *(value), *(value)-1, result));

  size_t status = 0; // success
  if (result != PAMI_SUCCESS)
  {
    TRACE_ERR((stderr, "   done() PAMI_Put failed\n"));
    status = 1; // get failed
  }

  ++*(value);
  TRACE_ERR((stderr, "<< done()\n"));
}

static void dispatch_send (
    pami_context_t       context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_endpoint_t origin,
pami_recv_t        * recv)        /**< OUT: receive message structure */
{
  volatile size_t * value = (volatile size_t *) cookie;
  fprintf (stderr, ">> 'send' dispatch function.  cookie = %p (_done: %zu), header_size = %zu, pipe_size = %zu, recv = %p\n", cookie, *value, header_size, pipe_size, recv);

  info_t * info = (info_t *) header_addr;
  fprintf (stderr, "   'send' dispatch function.  info->origin = 0x%08x, info->dst = %p\n", info->origin, info->dst);

  _nonzero = 1;

  pami_put_simple_t parameters;
  parameters.rma.dest    = info->origin;
  //parameters.rma.hints   = {0};
  parameters.rma.bytes   = sizeof(size_t);
  parameters.rma.cookie  = cookie;
  parameters.rma.done_fn = done;
  parameters.addr.local  = (void *) &_nonzero;
  parameters.addr.remote = info->dst;
  PAMI_Put (context, &parameters);

  fprintf (stderr, "<< 'send' dispatch function.\n");

  return;
}


int main (int argc, char ** argv)
{
  _done = 0;

  pami_client_t client;
  pami_context_t context[2];

  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami client. result = %d\n", result);
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

  pami_send_hint_t options={0};

#ifdef USE_SHMEM_OPTION
  options.use_shmem = 1;
  fprintf (stderr, "##########################################\n");
  fprintf (stderr, "shared memory optimizations forced ON\n");
  fprintf (stderr, "##########################################\n");
#elif defined(NO_SHMEM_OPTION)
  options.no_shmem = 1;
  fprintf (stderr, "##########################################\n");
  fprintf (stderr, "shared memory optimizations forced OFF\n");
  fprintf (stderr, "##########################################\n");
#endif

  size_t i = 0;
#ifdef TEST_CROSSTALK
  for (i=0; i<2; i++)
#endif
  {
    pami_dispatch_callback_fn fn;

    fprintf (stderr, "Before PAMI_Dispatch_set(%d) .. &_done = %p, _done = %zu\n", DISPATCH_ID, &_done, _done);
    fn.p2p = dispatch_send;
    result = PAMI_Dispatch_set (context[i],
                                DISPATCH_ID,
                                fn,
                                (void *)&_done,
                                options);
    if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }
  }

  if (task_id == 0)
  {
    pami_send_immediate_t parameters;
#ifdef TEST_CROSSTALK
    fprintf (stdout, "PAMI_Put('simple') functional test [crosstalk]\n");
    fprintf (stdout, "\n");
    PAMI_Endpoint_create (client, 1, 1, &parameters.dest);
#else
    fprintf (stdout, "PAMI_Put('simple') functional test\n");
    fprintf (stdout, "\n");
    PAMI_Endpoint_create (client, 1, 0, &parameters.dest);
#endif


    // Send a message to the target task
    info_t info;
    PAMI_Endpoint_create (client, 0, 0, &info.origin);
    info.dst = (void *) &_done;

    parameters.dispatch        = DISPATCH_ID;
    parameters.header.iov_base = &info;
    parameters.header.iov_len  = sizeof(info_t);
    parameters.data.iov_base   = NULL;
    parameters.data.iov_len    = 0;
fprintf (stderr, "Before PAMI_Send_immediate()\n");
    PAMI_Send_immediate (context[0], &parameters);

    // wait for the 'ack'
fprintf (stderr, "Wait for 'ack', _done (%p) = %zu\n", &_done, _done);
    while (!_done)
    {
      result = PAMI_Context_advance (context[0], 100);
      if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
        return 1;
      }
    }
  }
  else if (task_id == 1)
  {
#ifdef TEST_CROSSTALK
      size_t contextid = 1;
#else
      size_t contextid = 0;
#endif

    // wait for the 'rts'
fprintf (stderr, "Wait for message, _done = %zu, contextid = %zu\n", _done, contextid);
    while (!_done != 0)
    {
      result = PAMI_Context_advance (context[contextid], 100);
      if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
        return 1;
      }
    }
  }
fprintf (stderr, "Test completed .. cleanup\n");

  result = PAMI_Context_destroyv(context, num);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy (client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami client. result = %d\n", result);
    return 1;
  }

  //fprintf (stdout, "Success (%d)\n", task_id);

  return 0;
};
