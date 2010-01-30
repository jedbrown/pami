/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file tests/dcmf/send/send_to_self_perf.c
 * \brief Test XMI_Send(), sending via loopback to ourself.
 *
 * The test starts with message size 0, and increases it up to the 
 * BUFSIZE, printing out cycles and microseconds.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "sys/xmi.h"
#define ITERATIONS 2
//#define ITERATIONS 1000
//#define ITERATIONS 100

#ifndef BUFSIZE
//#define BUFSIZE 2048
//#define BUFSIZE 1024*256
//#define BUFSIZE 16
#define BUFSIZE 1024
#endif

#define TRACE_ERR(x) //fprintf x

size_t _my_rank;

uint8_t _rbuf[BUFSIZE];
uint8_t _sbuf[BUFSIZE];
  xmi_client_t  client;
  xmi_context_t context;

volatile size_t _send_active;
volatile size_t _recv_active;

size_t dispatch = 0;

char testcase_str[10240];

unsigned long long T_RECV_DONE, T_DISPATCH, T_SEND_DONE_LOCAL, T_SEND_DONE_REMOTE, T_SEND_START, T_ADVANCE_DONE;

/* /\* --------------------------------------------------------------- *\/ */

/* static void decrement (void * clientdata, XMI_Error_t * err) */
/* { */
/*   unsigned* _clientdata = (unsigned *)clientdata; */
/*   TRACE_ERR((stderr, "(%zd) decrement() clientdata = %p, %d => %d\n", _my_rank, _clientdata, *_clientdata, *_clientdata-1)); */
/*   --*_clientdata; */
/*   if (*_clientdata < 0) exit(-1); */
/* } */

/* /\* --------------------------------------------------------------- *\/ */

/* static void cb_recv_new_short (void           * clientdata, */
/*                                const XMIQuad   * msginfo, */
/*                                unsigned         count, */
/*                                size_t           peer, */
/*                                const char     * src, */
/*                                size_t           bytes) */
/* { */
/*   unsigned* _clientdata = (unsigned *)clientdata; */
/*   if (*_clientdata <= 0) abort(); */
/*   TRACE_ERR((stderr, "(%zd) cb_recv_new_short() clientdata = %p, %d => %d\n", _my_rank, _clientdata, *_clientdata, *_clientdata-1)); */
/*   memcpy(_rbuf, src, bytes); */
/*   --*_clientdata; */
/* } */

/* /\* --------------------------------------------------------------- *\/ */

/* static XMI_Request_t * cb_recv_new (void                 * clientdata, */
/*                                      const XMIQuad         * msginfo, */
/*                                      unsigned               count, */
/*                                      size_t                 senderID, */
/*                                      size_t                 sndlen, */
/*                                      size_t               * rcvlen, */
/*                                      char                ** rcvbuf, */
/*                                      XMI_Callback_t      * cb_info) */
/* { */
/*   TRACE_ERR((stderr, "(%zd) cb_recv_new() clientdata = %p, sndlen = %zd\n", _my_rank, clientdata, sndlen)); */
/*   * rcvbuf = (char *) _rbuf; */
/*   * rcvlen = sndlen>0?sndlen:1; */
/*   cb_info->function   = decrement; */
/*   cb_info->clientdata = clientdata; */

/*   return &_recv_request; */
/* } */

static void recv_done (xmi_context_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  T_RECV_DONE = XMI_Wtimebase();
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE_ERR((stderr, "Called recv_done function.  cookie=%p, active: %zd -> %zd\n", cookie, *active, *active-1));
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
  T_DISPATCH = XMI_Wtimebase();
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE_ERR((stderr, "Called dispatch function.  cookie = %p, active: %zd, header_addr = %p, pipe_addr = %p\n", cookie, *active, header_addr, pipe_addr));

  recv->local_fn = recv_done;
  recv->cookie   = cookie;
  recv->kind = XMI_AM_KIND_SIMPLE;
  recv->data.simple.addr  = _rbuf;
  recv->data.simple.bytes = pipe_size;
  TRACE_ERR((stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn));

  return;
}

static void send_done_local (xmi_context_t   context,
                             void          * cookie,
                             xmi_result_t    result)
{
  T_SEND_DONE_LOCAL = XMI_Wtimebase();
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE_ERR((stderr, "Called send_done_local function.  cookie=%p, active: %zd -> %zd\n", cookie, *active, *active-1));
  (*active)--;
}

static void send_done_remote (xmi_context_t   context,
                              void          * cookie,
                              xmi_result_t    result)
{
  T_SEND_DONE_REMOTE = XMI_Wtimebase();
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE_ERR((stderr, "Called send_done_remote function.  cookie=%p, active: %zd -> %zd\n", cookie, *active, *active-1));
  (*active)--;
}

unsigned long long test (size_t sndlen, size_t myrank)
{
  xmi_quad_t msginfo;
  xmi_result_t result = XMI_ERROR;
  xmi_send_t parameters;
  parameters.send.dispatch        = dispatch;
  parameters.send.header.iov_base = (void *)&msginfo; // send *something*
  parameters.send.header.iov_len  = sizeof(msginfo);
  parameters.send.data.iov_base   = (void *)_sbuf; // send *something*
  parameters.send.data.iov_len    = sndlen;
  parameters.events.cookie    = (void *) &_send_active;
  parameters.events.local_fn  = send_done_local;
  parameters.events.remote_fn = send_done_remote;

  unsigned i;
  unsigned long long t1 = 0;
  //Allow warmup of 1 iteration
  for (i = 0; i <= ITERATIONS; i++)
  {
    parameters.send.dest = XMI_Client_endpoint (client, myrank, 0);
    TRACE_ERR((stderr, "(%zd)\n(%zd) Starting Iteration %d of size %zd\n", _my_rank, _my_rank, i, sndlen));
    if (i == 1)
      t1 = XMI_Wtimebase();

    _send_active = 2;
    _recv_active = 1;
    TRACE_ERR((stderr,"test():  Calling XMI_Send\n"));

    T_SEND_START = XMI_Wtimebase();
    result = XMI_Send (context, &parameters);
    TRACE_ERR((stderr,"test():  Back from XMI_Send\n"));

    while (_send_active || _recv_active) 
      {
	TRACE_ERR((stderr,"test():  Calling Advance\n"));
	result = XMI_Context_advance (context, 100);
	TRACE_ERR((stderr,"test():  Back from Advance\n"));
      }
    T_ADVANCE_DONE = XMI_Wtimebase();
  }
  unsigned long long t2 = XMI_Wtimebase();

  return ((t2-t1)/ITERATIONS);
}

int main ()
{
  printf("Start test ...\n");

  char          cl_string[] = "TEST";
  xmi_result_t  result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize client. result = %d\n", result);
    return 1;
  }

  { size_t _n = 1; result = XMI_Context_createv(client, NULL, 0, &context, _n); }
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create context. result = %d\n", result);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query (context, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration task ID (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  fprintf (stderr, "My task id = %zd\n", task_id);
  _my_rank = task_id;

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query (context, &configuration);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration NumTasks (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  printf("(%zd) after Initializing client and context.  num_tasks=%zu\n", task_id, num_tasks);

  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options={0};
  TRACE_ERR((stderr, "Before XMI_Dispatch_set() .. &_recv_active = %p, _recv_active = %lu\n", &_recv_active, _recv_active));
  result = XMI_Dispatch_set (context,
                             dispatch,
                             (xmi_dispatch_callback_fn) fn,
                             (void *)&_recv_active,
                             options);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register xmi dispatch. result = %d\n", result);
    return 1;
  }
  
  double clockMHz = 1600.0;

/*   /\* Register the protocols to test *\/ */
/*   DCMF_Send_Configuration_t p; */
/*   p.cb_recv_short            = cb_recv_new_short; */
/*   p.cb_recv_short_clientdata = (void *)&_recv_active; */
/*   p.cb_recv                  = cb_recv_new; */
/*   p.cb_recv_clientdata       = (void *)&_recv_active; */

/*   char testcase_str[10240]; */
/*   unsigned i, j, k = 0; */
/*   _protocol_count = 0; */
/*   for (i = 1; i < DCMF_SEND_PROTOCOL_COUNT; i++) */
/*   { */
/*     /\* don't test the rzv protocol *\/ */
/*     /\*if (i == DCMF_RZV_SEND_PROTOCOL) continue;*\/ */

/*     /\* don't test the eager protocol *\/ */
/*     /\*if (i == DCMF_EAGER_SEND_PROTOCOL) continue;*\/ */

/*     /\* don't test the user0 protocol *\/ */
/*     if (i == DCMF_USER0_SEND_PROTOCOL) continue; */

/*     /\* don't test the user1 protocol *\/ */
/*     if (i == DCMF_USER1_SEND_PROTOCOL) continue; */

/*     p.protocol = (DCMF_Send_Protocol) i; */
/*     for (j = 0; j < XMI_NETWORK_COUNT; j++) */
/*     { */
/*       /\* don't test the default network *\/ */
/*       if (j == XMI_DEFAULT_NETWORK) continue; */

/*       /\* don't test the torus network *\/ */
/*       /\*if (j == DCMF_TORUS_NETWORK) continue;*\/ */

/*       /\* don't test the shmem network *\/ */
/*       /\*if (j == DCMF_SHMEM_NETWORK) continue;*\/ */

/*       p.network = (XMI_Network) j; */
/*       TRACE_ERR((stderr, "(%zd) before DCMF_Send_register(), network = %d, protocol = %d\n", DCMF_Messager_rank (), j, i)); */
/*       XMI_Result result = DCMF_Send_register (&_protocol[_protocol_count], &p); */
/*       TRACE_ERR((stderr, "(%zd) after DCMF_Send_register(), network = %d, protocol = %d, result = %d\n", DCMF_Messager_rank (), j, i, result)); */

/*       if (result == XMI_SUCCESS) */
/*       { */
/*         k += sprintf (&testcase_str[k], "# testcase %d == %s: %s\n", _protocol_count, _send_names[i], _network_names[j]); */
/*         _protocol_count++; */
/*       } */
/*     } */
/*   } */

/*   /\* Display some test header information *\/ */
/*   _my_rank = DCMF_Messager_rank (); */

  unsigned int _protocol_count = 1;

  if (_my_rank == 0)
  {
    char hdrstr[2][1024];
    int index[2];
    index[0] = 0;
    index[1] = 0;

    index[0] += sprintf (&hdrstr[0][index[0]], "#          ");
    index[1] += sprintf (&hdrstr[1][index[1]], "#    bytes ");

    unsigned i;
    for (i=0; i<_protocol_count; i++)
    {
      index[0] += sprintf (&hdrstr[0][index[0]], "[--- testcase %d ---] ", i);
      index[1] += sprintf (&hdrstr[1][index[1]], "    cycles     usec  ");
    }

    fprintf (stdout, "# XMI_Send() send-to-self blocking latency performance test\n");
    fprintf (stdout, "#\n");
    fprintf (stdout, "%s", testcase_str);
    fprintf (stdout, "#\n");
    fprintf (stdout, "%s\n", hdrstr[0]);
    fprintf (stdout, "%s\n", hdrstr[1]);
    fflush (stdout);
  
    unsigned long long cycles;
    double usec;

    char str[10240];

    // Init the send buffer.
    unsigned j;
    for (j=0; j<BUFSIZE; j++)
      {
	_sbuf[j]=j*5+3;
      }

    size_t sndlen;
    for (sndlen = 0; sndlen < BUFSIZE; sndlen = sndlen*3/2+1)
    {
      int index = 0;
      index += sprintf (&str[index], "%10zd ", sndlen);

      unsigned i;
      for (i=0; i<_protocol_count; i++)
      {
        /* warmup */
        //test (&_protocol[i], sndlen, _my_rank);

        cycles = test (sndlen, _my_rank);
        usec   = cycles/clockMHz;

	// Check the buffer.
	unsigned j;
	for (j=0; j<sndlen; j++)
	{
	  if (_sbuf[j] != _rbuf[j]) printf("Data Miscompare at size %zd, _sbuf[%u] = 0x%02x, _rbuf = 0x%02x\n",sndlen, j, _sbuf[j], _rbuf[j]);
	}

	TRACE_ERR(("sndlen=%u, START=%llu, SEND_DONE_LOCAL=%llu, DISPATCH=%llu, RECV_DONE=%llu, SEND_DONE_REMOTE=%llu, ADVANCE_DONE=%llu\n",
		   sndlen, T_SEND_START,T_SEND_DONE_LOCAL,T_DISPATCH,T_RECV_DONE,T_SEND_DONE_REMOTE,T_ADVANCE_DONE));
        index += sprintf (&str[index], "%10lld %8.4f  ", cycles, usec);
      }

      fprintf (stdout, "%s\n", str);
    }
  }

}
