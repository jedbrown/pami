///
/// \file test/context/post-multithreaded.c
/// \brief Multithreaded XMI_Context_post() test
///

#include "sys/xmi.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

//#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif


typedef struct endpoint
{
  xmi_context_t   context;
  volatile size_t recv;
} endpoint_t;

typedef struct work
{
  void            * addr;
  size_t            bytes;
  volatile size_t   active;
  size_t            from;
  size_t            to;
} work_t;


endpoint_t _endpoint[2];

void do_work (xmi_context_t   context,
              void          * cookie,
              xmi_result_t    result)
{
  TRACE((stderr, ">> do_work (%0x08x, %p, %d)\n", (unsigned)context, cookie, result));
  work_t * work = (work_t *) cookie;

  /* copy the data to a local buffer. */
  uint8_t local_buffer[1024];
  size_t i, n = work->bytes<1024?work->bytes:1024;
  size_t to = work->to;
  TRACE((stderr, "   do_work (), work->bytes = %zd, work->addr = %p, to = %zd, n = %zd\n", work->bytes, work->addr, to, n));
  memcpy ((void *) local_buffer, work->addr, n);

  /* Notify the 'sender' that the 'receive' is complete. */
  work->active = 0;

  /* decrement the recv flag */
  TRACE((stderr, "   do_work (), _endpoint[%zd].recv = %zd -> %zd\n", to, _endpoint[to].recv, _endpoint[to].recv - 1));
  _endpoint[to].recv--;

  TRACE((stderr, "<< do_work ()\n"));
  return;
}

void * endpoint (void * arg)
{
  size_t id = (size_t) arg;
  TRACE((stderr, ">> endpoint (%zd)\n", id));

  /* Lock this context */
  xmi_result_t result = XMI_Context_lock (_endpoint[id].context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to lock the xmi context. result = %d\n", result);
    exit(1);
  }

  uint8_t send[1024];
  uint8_t recv[1024];
  size_t i;
  for (i=0; i<1024; i++)
  {
    send[i] = (uint8_t) -1;
    recv[i] = 0;
  }

  if (id == 0)
  {
    /* send a message to endpoint 10 */
    work_t work;
    work.addr = (void *) send;
    work.bytes = 1024;
    work.active = 1;
    work.from = 0;
    work.to   = 1;

    result = XMI_Context_post (_endpoint[1].context, do_work, (void *)&work);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to post work to the xmi context. result = %d\n", result);
      exit(1);
    }

    /* block until the work has been "received" */
    TRACE((stderr, "   endpoint(%zd), before blocking 'advance', work.active = %zd\n", id, work.active));
    while (work.active);
    TRACE((stderr, "   endpoint(%zd),  after blocking 'advance', work.active = %zd\n", id, work.active));

    /* wait to "receive" a message from endpoint 1 */
    TRACE((stderr, "   endpoint(%zd), before blocking advance for recv, _endpoint[0].recv = %zd\n", id, _endpoint[0].recv));
    while (_endpoint[0].recv)
    {
      result = XMI_Context_advance (_endpoint[0].context, 1);
      if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance the xmi context. result = %d\n", result);
        exit(1);
      }
    }
    TRACE((stderr, "   endpoint(%zd),  after blocking advance for recv, _endpoint[0].recv = %zd\n", id, _endpoint[0].recv));
  }
  else if (id == 1)
  {
    /* wait to "receive" a message from endpoint 0 */
    TRACE((stderr, "   endpoint(%zd), before blocking advance for recv, _endpoint[1].recv = %zd\n", id, _endpoint[1].recv));
    while (_endpoint[1].recv)
    {
      result = XMI_Context_advance (_endpoint[1].context, 1);
      if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance the xmi context. result = %d\n", result);
        exit(1);
      }
    }
    TRACE((stderr, "   endpoint(%zd),  after blocking advance for recv, _endpoint[1].recv = %zd\n", id, _endpoint[1].recv));

    /* send a message to endpoint 1 */
    work_t work;
    work.addr = (void *) send;
    work.bytes = 1024;
    work.active = 1;
    work.from = 1;
    work.to   = 0;

    result = XMI_Context_post (_endpoint[0].context, do_work, (void *)&work);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to post work to the xmi context. result = %d\n", result);
      exit(1);
    }

    /* block until the work has been "received" */
    TRACE((stderr, "   endpoint(%zd), before blocking 'advance', work.active = %zd\n", id, work.active));
    while (work.active);
    TRACE((stderr, "   endpoint(%zd),  after blocking 'advance', work.active = %zd\n", id, work.active));
  }

  /* Unlock the context and exit */
  result = XMI_Context_unlock (_endpoint[id].context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to unlock the xmi context. result = %d\n", result);
    exit(1);
  }

  TRACE((stderr, "<< endpoint(%zd)\n", id));
  return NULL;
}


int main (int argc, char ** argv)
{
  xmi_client_t client;
  xmi_context_t context[2];
  xmi_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

  _endpoint[0].recv = 1;
  {
  int num = 2;
  result = XMI_Context_createv (client, configuration, 0, &context[0], &num);
  if (result != XMI_SUCCESS || num != 2)
  {
    fprintf (stderr, "Error. Unable to create first xmi context. result = %d\n", result);
    return 1;
  }
  _endpoint[0].context = context[0];
  _endpoint[1].context = context[1];
  }

  _endpoint[1].recv = 1;

  /* Create the "helper" or "endpoint" thread */
  pthread_t thread;
  int rc = pthread_create (&thread, NULL, endpoint, (void *)1);
  if (rc != 0)
  {
    fprintf (stderr, "Error. Unable to create the second pthread. rc = %d\n", rc);
    return 1;
  }

  /* enter the "endpoint function" for the main thread */
  endpoint ((void *)0);


  result = XMI_Context_destroy (_endpoint[0].context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy first xmi context. result = %d\n", result);
    return 1;
  }
  result = XMI_Context_destroy (_endpoint[1].context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy second xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  fprintf (stderr, "Success.\n");

  return 0;
};
