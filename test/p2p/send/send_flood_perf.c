/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/p2p/send/send_flood_perf.c
 * \brief ???
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#include "sys/xmi.h"
//#include "../../util.h"

#define ITERATIONS 1
//#define ITERATIONS 1000
//#define ITERATIONS 1000

#define WARMUP

#define TEST_SEND_IMMEDIATE

#ifdef TEST_SEND_IMMEDIATE
#define BUFSIZE 256
#else
#ifndef BUFSIZE
#define BUFSIZE 2048
//#define BUFSIZE 1024*256
#endif
#endif


#define MSGCOUNT 1024
//#define MSGCOUNT 16

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

volatile unsigned _recv_active;
uint8_t _tmpbuffer[BUFSIZE];
size_t _my_rank;

/* --------------------------------------------------------------- */

static void decrement (xmi_context_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  unsigned * value = (unsigned *) cookie;
  TRACE_ERR((stderr, "(%zd) decrement() cookie = %p, %d => %d\n", _my_rank, cookie, *value, *value-1));
  --*value;
}

/* --------------------------------------------------------------- */
static void test_dispatch (
    xmi_context_t        context,      /**< IN: XMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
{
  unsigned * value = (unsigned *) cookie;
  if (pipe_addr != NULL)
  {
    TRACE_ERR((stderr, "(%zd) short recv:  decrement cookie = %p, %d => %d\n", _my_rank, cookie, *value, *value-1));
    memcpy((void *)_tmpbuffer, pipe_addr, pipe_size);
    --*value;
    return;
  }

  TRACE_ERR((stderr, "(%zd) long recvn", _my_rank));
  recv->local_fn = decrement;
  recv->cookie   = cookie;
  recv->kind = XMI_AM_KIND_SIMPLE;
  recv->data.simple.addr  = (void *)_tmpbuffer;
  recv->data.simple.bytes = pipe_size;
}

unsigned long long test (xmi_context_t context, size_t dispatch, size_t hdrlen, size_t sndlen, size_t myrank, size_t ntasks)
{
  TRACE_ERR((stderr, "(%zd) Do test ... sndlen = %zd\n", myrank, sndlen));

  char metadata[BUFSIZE];
  char buffer[BUFSIZE];
  unsigned i, j;

  xmi_client_t client = 0;
  if (myrank == 0)
    _recv_active = (ntasks - 1) * MSGCOUNT * ITERATIONS;
  else
    _recv_active = 1;

  volatile unsigned send_active = MSGCOUNT * ITERATIONS;

  xmi_endpoint_t endpoint[ntasks];
  for (i=0; i<ntasks; i++)
    endpoint[i] = XMI_Client_endpoint (client, i, 0);

  xmi_send_t parameters;
  parameters.send.dest = endpoint[0];
  parameters.send.dispatch = dispatch;
  parameters.send.header.iov_base = metadata;
  parameters.send.header.iov_len = hdrlen;
  parameters.send.data.iov_base  = buffer;
  parameters.send.data.iov_len = sndlen;
  parameters.events.cookie        = (void *) &send_active;
  parameters.events.local_fn      = decrement;
  parameters.events.remote_fn     = NULL;

  xmi_send_immediate_t ack;
  ack.dispatch = dispatch;
  ack.header.iov_base = NULL;
  ack.header.iov_len = 0;
  ack.data.iov_base  = NULL;
  ack.data.iov_len = 0;

  //barrier ();
  usleep(1000);

  unsigned long long t1 = XMI_Wtimebase();
  if (myrank == 0)
  {
    while (_recv_active) XMI_Context_advance (context, 100);
    for (i = 1; i < ntasks; i++)
    {
      ack.dest = endpoint[i];
      XMI_Send_immediate (context, &ack);
    }
  }
  else
  {
    for (i = 0; i < ITERATIONS; i++)
    {
      for (j = 0; j < MSGCOUNT; j++)
      {
#ifdef TEST_SEND_IMMEDIATE
        XMI_Send_immediate (context, &parameters.send);
#else
        XMI_Send (context, &parameters);
#endif
      }
    }
    while (_recv_active > 0) XMI_Context_advance (context, 100);
  }
  unsigned long long t2 = XMI_Wtimebase();

  return ((t2-t1)/ITERATIONS)/MSGCOUNT;
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

  xmi_client_t client;
  char clientname[]="XMI";
  TRACE_ERR((stderr, "... before XMI_Client_initialize()\n"));
  XMI_Client_initialize (clientname, &client);
  TRACE_ERR((stderr, "...  after XMI_Client_initialize()\n"));
  xmi_context_t context;
  TRACE_ERR((stderr, "... before XMI_Context_create()\n"));
  XMI_Context_createv (client, NULL, 0, &context, 1);
  TRACE_ERR((stderr, "...  after XMI_Context_create()\n"));

  //TRACE_ERR((stderr, "... before barrier_init()\n"));
  //barrier_init (client, context, 0);
  //TRACE_ERR((stderr, "...  after barrier_init()\n"));


  /* Register the protocols to test */
  size_t dispatch = 1;
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options={0};
  TRACE_ERR((stderr, "Before XMI_Dispatch_set() .. &_recv_active = %p, recv_active = %zd\n", &_recv_active, _recv_active));
  xmi_result_t result = XMI_Dispatch_set (context,
                                          dispatch,
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
  _my_rank = configuration.value.intval;

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

    fprintf (stdout, "# send flood performance test\n");
    fprintf (stdout, "#   Number of tasks 'flooding' task 0: %zu\n", num_tasks-1);
    fprintf (stdout, "#\n");

    unsigned i;
    for (i=0; i<hdrcnt; i++)
    {
      if (i==0)
        fprintf (stdout, "# testcase %d : header bytes = %3zd\n", i, hdrsize[i]);
      else
        fprintf (stdout, "# testcase %d : header bytes = %3zd (argv[%d])\n", i, hdrsize[i], i);
      index[0] += sprintf (&str[0][index[0]], "[- testcase %d -] ", i);
      index[1] += sprintf (&str[1][index[1]], " cycles    usec  ");
    }

    fprintf (stdout, "#\n");
    fprintf (stdout, "%s\n", str[0]);
    fprintf (stdout, "%s\n", str[1]);
    fflush (stdout);
  }

  //barrier ();

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
      test (context, dispatch, hdrsize[i], sndlen, _my_rank, num_tasks);
#endif
      cycles = test (context, dispatch, hdrsize[i], sndlen, _my_rank, num_tasks);
      usec   = cycles * tick * 1000000.0;
      index += sprintf (&str[index], "%7lld %7.4f  ", cycles, usec);
    }

    if (_my_rank == 0)
      fprintf (stdout, "%s\n", str);
  }

  XMI_Client_finalize (client);

  return 0;
}
#undef TRACE_ERR
