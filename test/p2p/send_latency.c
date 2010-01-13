/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/p2p/send_latency.c
 * \brief ???
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "sys/xmi.h"
#include "../util.h"

#define ITERATIONS 100
//#define ITERATIONS 1000
//#define ITERATIONS 1

#ifndef BUFSIZE
#define BUFSIZE 2048
//#define BUFSIZE 1024*256
//#define BUFSIZE 16
//#define BUFSIZE 1024
#endif

#define WARMUP

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

volatile unsigned _send_active;
volatile unsigned _recv_active;
volatile unsigned _recv_iteration;
char              _recv_buffer[BUFSIZE] __attribute__ ((__aligned__(16)));

size_t         _dispatch[100];
unsigned       _dispatch_count;

size_t _my_rank;

typedef struct
{
  size_t sndlen;
} header_t;

/* --------------------------------------------------------------- */

static void decrement (xmi_context_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  unsigned * value = (unsigned *) cookie;
  TRACE_ERR((stderr, "(%zu) decrement() cookie = %p, %d => %d\n", _my_rank, cookie, *value, *value-1));
  --*value;
}

/* --------------------------------------------------------------- */
static void test_dispatch (
    xmi_context_t        context,      /**< IN: XMI context */
    size_t               contextid,
    void               * cookie,       /**< IN: dispatch cookie */
    xmi_task_t           task,         /**< IN: source task */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
{
  if (pipe_addr != NULL)
  {
    //memcpy (_recv_buffer, pipe_addr, pipe_size);
    unsigned * value = (unsigned *) cookie;
    TRACE_ERR((stderr, "(%zu) test_dispatch() short recv:  cookie = %p, decrement: %d => %d\n", _my_rank, cookie, *value, *value-1));
    --*value;
  }
  else
  {
    header_t * header = (header_t *) header_addr;
    TRACE_ERR((stderr, "(%zu) test_dispatch() async recv:  cookie = %p, pipe_size = %zu\n", _my_rank, cookie, pipe_size));

    recv->local_fn = decrement;
    recv->cookie   = cookie;
    recv->kind = XMI_AM_KIND_SIMPLE;
    recv->data.simple.addr  = _recv_buffer;
    recv->data.simple.bytes = pipe_size;
  }

  _recv_iteration++;
}

void send_once (xmi_context_t context, xmi_send_t * parameters)
{
  xmi_result_t result = XMI_Send (context, parameters);
  TRACE_ERR((stderr, "(%zu) send_once() Before advance\n", _my_rank));
  while (_send_active) XMI_Context_advance (context, 100);
  TRACE_ERR((stderr, "(%zu) send_once()  After advance\n", _my_rank));
}

void recv_once (xmi_context_t context)
{
  TRACE_ERR((stderr, "(%zu) recv_once() Before advance\n", _my_rank));
  while (_recv_active) XMI_Context_advance (context, 100);

  _recv_active = 1;
  TRACE_ERR((stderr, "(%zu) recv_once()  After advance\n", _my_rank));
}

unsigned long long test (xmi_context_t context, size_t dispatch, size_t hdrsize, size_t sndlen, size_t myrank)
{
  TRACE_ERR((stderr, "(%zu) Do test ... sndlen = %zu\n", myrank, sndlen));
  _recv_active = 1;
  _recv_iteration = 0;
  _send_active = 1;

  char metadata[BUFSIZE];
  char buffer[BUFSIZE];

  header_t header;
  header.sndlen = sndlen;

  xmi_send_t parameters;
  parameters.send.dispatch        = dispatch;
  parameters.send.header.iov_base = metadata;
  parameters.send.header.iov_len  = hdrsize;
  parameters.send.data.iov_base   = buffer;
  parameters.send.data.iov_len    = sndlen;
  parameters.events.cookie        = (void *) &_send_active;
  parameters.events.local_fn      = decrement;
  parameters.events.remote_fn     = NULL;

  barrier ();

  unsigned i;
  unsigned long long t1 = XMI_Wtimebase();
  if (myrank == 0)
  {
    parameters.send.task = 1;
    for (i = 0; i < ITERATIONS; i++)
    {
      TRACE_ERR((stderr, "(%zu) Starting Iteration %d of size %zu\n", myrank, i, sndlen));
      send_once (context, &parameters);
      recv_once (context);

      _recv_active = 1;
      _send_active = 1;
    }
  }
  else if (myrank == 1)
  {
    parameters.send.task = 0;
    for (i = 0; i < ITERATIONS; i++)
    {
      TRACE_ERR((stderr, "(%zu) Starting Iteration %d of size %zu\n", myrank, i, sndlen));
      recv_once (context);
      send_once (context, &parameters);

      _recv_active = 1;
      _send_active = 1;
    }
  }
  unsigned long long t2 = XMI_Wtimebase();

  return ((t2-t1)/ITERATIONS)/2;
}

int main (int argc, char ** argv)
{
  TRACE_ERR((stderr, "Start test ...\n"));

  size_t hdrcnt = argc;
  size_t hdrsize[1024];
  hdrsize[0] = 0;

  int arg;
  for (arg=1; arg<argc; arg++)
  {
    hdrsize[arg] = (size_t) strtol (argv[arg], NULL, 10);
  }

  char clientname[] = "XMI";
  xmi_client_t client;
  TRACE_ERR((stderr, "... before XMI_Client_initialize()\n"));
  XMI_Client_initialize (clientname, &client);
  TRACE_ERR((stderr, "...  after XMI_Client_initialize()\n"));
  xmi_context_t context;
  TRACE_ERR((stderr, "... before XMI_Context_create()\n"));
  { int _n = 1; XMI_Context_createv (client, NULL, 0, &context, &_n); }
  TRACE_ERR((stderr, "...  after XMI_Context_create()\n"));

  TRACE_ERR((stderr, "... before barrier_init()\n"));
  barrier_init (client, context, 0);
  TRACE_ERR((stderr, "...  after barrier_init()\n"));


  /* Register the protocols to test */
  char testcase_str[10240];
  unsigned i, j, k = 0;
  _dispatch_count = 0;

  _dispatch[_dispatch_count] = _dispatch_count + 1;

  size_t dispatch = 1;
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options={0};
  TRACE_ERR((stderr, "Before XMI_Dispatch_set() .. &_recv_active = %p, recv_active = %zu\n", &_recv_active, _recv_active));
  xmi_result_t result = XMI_Dispatch_set (context,
                                          _dispatch[_dispatch_count++],
                                          fn,
                                          (void *)&_recv_active,
                                          options);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register xmi dispatch. result = %d\n", result);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query(client, &configuration);
  size_t _my_rank = configuration.value.intval;

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query(client, &configuration);
  size_t num_tasks = configuration.value.intval;

  configuration.name = XMI_WTICK;
  result = XMI_Configuration_query(client, &configuration);
  double tick = configuration.value.doubleval;

  /* Display some test header information */
  if (_my_rank == 0)
  {
    char str[2][1024];
    int index[2];
    index[0] = 0;
    index[1] = 0;

    index[0] += sprintf (&str[0][index[0]], "#          ");
    index[1] += sprintf (&str[1][index[1]], "#    bytes ");

    fprintf (stdout, "# XMI_Send() nearest-neighor half-pingpong blocking latency performance test\n");
    fprintf (stdout, "#\n");
    unsigned i;
    for (i=0; i<hdrcnt; i++)
    {
      if (i==0)
        fprintf (stdout, "# testcase %d : header bytes = %3zd\n", i, hdrsize[i]);
      else
        fprintf (stdout, "# testcase %d : header bytes = %3zd (argv[%d])\n", i, hdrsize[i], i);
      index[0] += sprintf (&str[0][index[0]], "[-- testcase %d --] ", i);
      index[1] += sprintf (&str[1][index[1]], "  cycles     usec  ");
    }
    fprintf (stdout, "#\n");
    fprintf (stdout, "%s\n", str[0]);
    fprintf (stdout, "%s\n", str[1]);
    fflush (stdout);
  }

  barrier ();

  unsigned long long cycles;
  double usec;

  char str[10240];

  size_t sndlen;
  for (sndlen = 0; sndlen < BUFSIZE; sndlen = sndlen*3/2+1)
  {
    int index = 0;
    index += sprintf (&str[index], "%10zd ", sndlen);

    unsigned i;
    for (i=0; i<hdrcnt; i++)
    {
#ifdef WARMUP
      test (context, _dispatch[0], hdrsize[i], sndlen, _my_rank);
#endif
      cycles = test (context, _dispatch[0], hdrsize[i], sndlen, _my_rank);
      usec   = cycles * tick * 1000000.0;
      index += sprintf (&str[index], "%8lld %8.4f  ", cycles, usec);
    }

    if (_my_rank == 0)
      fprintf (stdout, "%s\n", str);
  }

  XMI_Client_finalize (client);

  return 0;
}
#undef TRACE_ERR
