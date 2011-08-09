/**
 * \file test/api/p2p/rget/rget_perf.c
 * \brief PAMI_Rget() latency test
 **/

#include <pami.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

#include "../../init_util.h"

#define ITERATIONS 100
#define BUFFERSIZE 10240*32
#define WARMUP

typedef struct
{
  pami_memregion_t mr;
  size_t           bytes;
} mr_t;

typedef struct
{
  volatile size_t counter;
  mr_t            task[0];
} mr_exchange_t;


/* --------------------------------------------------------------- */

static void decrement (pami_context_t   context,
                       void           * cookie,
                       pami_result_t    result)
{
  unsigned * value = (unsigned *) cookie;
  /*fprintf (stderr, "decrement() cookie = %p, %d => %d\n", cookie, *value, *value - 1);*/
  --*value;
}

/* --------------------------------------------------------------- */

/**
 * \brief memory region exchange dispatch function
 */
void mr_exchange_fn (
  pami_context_t    context,      /**< IN: PAMI context */
  void            * cookie,       /**< IN: dispatch cookie */
  const void      * header,       /**< IN: header address */
  size_t            header_size,  /**< IN: header size */
  const void      * data,         /**< IN: address of PAMI pipe buffer */
  size_t            data_size,    /**< IN: size of PAMI pipe buffer */
  pami_endpoint_t   origin,
  pami_recv_t     * recv)         /**< OUT: receive message structure */
{
  pami_task_t id;
  size_t offset;
  PAMI_Endpoint_query (origin, &id, &offset);

  mr_exchange_t * exchange = (mr_exchange_t *) cookie;
  memcpy(&exchange->task[id].mr, data, sizeof(pami_memregion_t));
  exchange->task[id].bytes = *((size_t *) header);
  exchange->counter--;

  return;
}


int main (int argc, char ** argv)
{
  pami_client_t        client;
  pami_context_t       context;
  size_t               num_contexts = 1;
  pami_task_t          me, you;
  size_t               num_tasks;
  pami_result_t        result;

  int rc = pami_init (&client,        /* Client             */
                      &context,       /* Context            */
                      NULL,           /* Clientname=default */
                      &num_contexts,  /* num_contexts       */
                      NULL,           /* null configuration */
                      0,              /* no configuration   */
                      &me,            /* task id            */
                      &num_tasks);    /* number of tasks    */

  if (rc == 1)
    return 1;
    
  you = num_tasks - 1;

  pami_configuration_t configuration;
  configuration.name = PAMI_CLIENT_WTICK;
  PAMI_Client_query(client, &configuration, 1);
  double tick = configuration.value.doubleval;


  mr_exchange_t * exchange =
    (mr_exchange_t *) malloc (sizeof(mr_exchange_t) + sizeof(mr_t) * num_tasks);

  /* Initialize the exchange information */
  exchange->counter = num_tasks;

  mr_t * info = exchange->task;

  size_t dispatch = 10;
  pami_dispatch_callback_function fn;
  fn.p2p = mr_exchange_fn;
  pami_dispatch_hint_t options = {};

  result = PAMI_Dispatch_set (context,
                              dispatch,
                              fn,
                              (void *) exchange,
                              options);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }


  /* Allocate and intialize the local data buffer for the test. */
  uint8_t * local = (uint8_t *) malloc (BUFFERSIZE);

  /* Create a memory region for the local data buffer. */
  size_t bytes;
  pami_memregion_t mr;
  result = PAMI_Memregion_create (context, (void *) local, BUFFERSIZE,
                                  &bytes, &mr);

  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to create memory region. result = %d\n", result);
      return 1;
    }
  else if (bytes < BUFFERSIZE)
    {
      fprintf (stderr, "Error. Unable to create memory region of a large enough size. result = %d\n", result);
      return 1;
    }


  /* Broadcast the data location to all tasks */
  size_t i;

  for (i = 0; i < num_tasks; i++)
    {
      pami_send_immediate_t parameters;
      parameters.dispatch        = dispatch;
      parameters.header.iov_base = (void *) & bytes;
      parameters.header.iov_len  = sizeof(size_t);
      parameters.data.iov_base   = (void *) & mr;
      parameters.data.iov_len    = sizeof(pami_memregion_t);
      PAMI_Endpoint_create (client, i, 0, &parameters.dest);

      result = PAMI_Send_immediate (context, &parameters);
    }

  /* Wait until all tasks have exchanged the memory region information */
  while (exchange->counter > 0)
    PAMI_Context_advance (context, 100);




  /* **************************************************************************
   * Begin the test - all tasks, including the 'root' task, will use PAMI_Rget
   * to flood the 'root' task and (hopefully) fill network fifos and force
   * a message queue event.
   * **************************************************************************/
   
  /* Display some test header information */
  if (me == 0)
    {
      fprintf (stdout, "# PAMI_Rget() blocking latency performance test\n");
      fprintf (stdout, "# task 0 rget from task %d\n", you);
      fprintf (stdout, "#\n");
      fprintf (stdout, "#    bytes   cycles     usec\n");
      fflush (stdout);

      volatile unsigned active;

      pami_rget_simple_t parameters;
      parameters.rma.hints          = (pami_send_hint_t) {0};
      parameters.rma.cookie         = (void *) & active;
      parameters.rma.done_fn        = decrement;
      parameters.rdma.local.mr      = &info[me].mr;
      parameters.rdma.local.offset  = 0;
      parameters.rdma.remote.mr     = &info[you].mr;
      parameters.rdma.remote.offset = 0;
      PAMI_Endpoint_create (client, you, 0, &parameters.rma.dest);

      unsigned long long t0, t1, cycles;
      double usec;

      size_t nbytes = 1;

      for (; nbytes < BUFFERSIZE; nbytes = nbytes * 3 / 2 + 1)
        {
          parameters.rma.bytes = nbytes;

#ifdef WARMUP
          for (i = 0; i < ITERATIONS; i++)
            {
              active = 1;
              PAMI_Rget (context, &parameters);
              while (active > 0)
                PAMI_Context_advance (context, 100);
            }
#endif
          t0 = PAMI_Wtimebase (client);
          for (i = 0; i < ITERATIONS; i++)
            {
              active = 1;
              PAMI_Rget (context, &parameters);
              while (active > 0)
                PAMI_Context_advance (context, 100);
            }
          t1 = PAMI_Wtimebase(client);

          cycles = (t1 - t0) / ITERATIONS;
          usec   = cycles * tick * 1000000.0;
          fprintf (stdout, "%10zd %8lld %8.4f\n", nbytes, cycles, usec);
        }
    }

  rc = pami_shutdown(&client, context, &num_contexts);

  if (rc == 1)
    return 1;

  return 0;
};
