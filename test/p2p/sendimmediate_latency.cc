/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/p2p/sendimmediate_latency.cc
 * \brief ???
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <pami.h>
#include "../util.h"

//#define ITERATIONS 10
//#define ITERATIONS 1000
//#define ITERATIONS 10000
#define ITERATIONS 100

#define WARMUP

#ifndef BUFSIZE
//#define BUFSIZE 2048
//#define BUFSIZE 1024*256
#define BUFSIZE 256
//#define BUFSIZE 64
//#define BUFSIZE 2
//#define BUFSIZE 1024
#endif

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
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
#if 0
static void decrement (pami_context_t   context,
                       void          * cookie,
                       pami_result_t    result)
{
  unsigned * value = (unsigned *) cookie;
  TRACE_ERR((stderr, "(%zu) decrement() cookie = %p, %d => %d\n", _my_rank, cookie, *value, *value-1));
  --*value;
}
#endif
/* --------------------------------------------------------------- */
static void test_dispatch (
    pami_context_t        context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_recv_t         * recv)        /**< OUT: receive message structure */
{
  unsigned * value = (unsigned *) cookie;
  TRACE_ERR((stderr, "(%zu) short recv:  decrement cookie = %p, %d => %d\n", _my_rank, cookie, *value, *value-1));
  --*value;
  _recv_iteration++;
}

void send_once (pami_context_t context, pami_send_immediate_t * parameters)
{
  TRACE_ERR((stderr, "(%zu) before send_immediate()  \n", _my_rank));
  //pami_result_t result =
    PAMI_Send_immediate (context, parameters);
  TRACE_ERR((stderr, "(%zu) after send_immediate()  \n", _my_rank));
}

void recv_once (pami_context_t context)
{
  TRACE_ERR((stderr, "(%zu) recv_once() Before advance\n", _my_rank));
  while (_recv_active) PAMI_Context_advance (context, 100);

  _recv_active = 1;
  TRACE_ERR((stderr, "(%zu) recv_once()  After advance\n", _my_rank));
}

unsigned long long test (pami_context_t context, size_t dispatch, size_t hdrlen, size_t sndlen, pami_task_t myrank, pami_endpoint_t origin, pami_endpoint_t target)
{
  TRACE_ERR((stderr, "(%zu) Do test ... sndlen = %zu\n", myrank, sndlen));
  _recv_active = 1;
  _recv_iteration = 0;
  _send_active = 1;

  char metadata[BUFSIZE];
  char buffer[BUFSIZE];

  header_t header;
  header.sndlen = sndlen;

  pami_send_immediate_t parameters;
  parameters.dispatch = dispatch;
  parameters.header.iov_base = metadata;
  parameters.header.iov_len = hdrlen;
  parameters.data.iov_base  = buffer;
  parameters.data.iov_len = sndlen;

//  barrier ();

  unsigned i;
  unsigned long long t1 = PAMI_Wtimebase();
  if (myrank == 0)
  {
    parameters.dest = target;
    for (i = 0; i < ITERATIONS; i++)
    {
      TRACE_ERR((stderr, "(%zu) Starting Iteration %d of size %zu\n", myrank, i, sndlen));
      send_once (context, &parameters);
      _send_active = 1;
      recv_once (context);
      _recv_active = 1;
    }
  }
  else if (myrank == 1)
  {
    parameters.dest = origin;
    for (i = 0; i < ITERATIONS; i++)
    {
      TRACE_ERR((stderr, "(%zu) Starting Iteration %d of size %zu\n", myrank, i, sndlen));
      recv_once (context);
      _recv_active = 1;
      send_once (context, &parameters);
      _send_active = 1;
    }
  }
  unsigned long long t2 = PAMI_Wtimebase();

  return ((t2-t1)/ITERATIONS)/2;
}

int main (int argc, char ** argv)
{
  TRACE_ERR((stderr, "Start test ...\n"));
//	printf("sizeof size_t:%d\n", sizeof(size_t));
  size_t hdrcnt = argc;
  size_t hdrsize[1024];
  hdrsize[0] = 0;

  int arg;
  for (arg=1; arg<argc; arg++)
  {
    hdrsize[arg] = (size_t) strtol (argv[arg], NULL, 10);
  }

  pami_client_t client;
  char clientname[]="PAMI";
  TRACE_ERR((stderr, "... before PAMI_Client_create()\n"));
  PAMI_Client_create (clientname, &client);
  TRACE_ERR((stderr, "...  after PAMI_Client_create()\n"));
  pami_context_t context;
  TRACE_ERR((stderr, "... before PAMI_Context_create()\n"));
  { size_t _n = 1; PAMI_Context_createv (client, NULL, 0, &context, _n); }
  TRACE_ERR((stderr, "...  after PAMI_Context_create()\n"));

  TRACE_ERR((stderr, "... before barrier_init()\n"));
  barrier_init (client, context, 0);
  TRACE_ERR((stderr, "...  after barrier_init()\n"));

//  printf("size of size_t:%d\n", sizeof(size_t));

  /* Register the protocols to test */
  _dispatch_count = 0;

  _dispatch[_dispatch_count] = _dispatch_count + 1;

  pami_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  pami_send_hint_t options={0};
  TRACE_ERR((stderr, "Before PAMI_Dispatch_set() .. &_recv_active = %p, recv_active = %zu\n", &_recv_active, _recv_active));
  pami_result_t result = PAMI_Dispatch_set (context,
                                          _dispatch[_dispatch_count++],
                                          fn,
                                          (void *)&_recv_active,
                                          options);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }

  pami_configuration_t configuration;

  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  _my_rank = configuration.value.intval;

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);

  configuration.name = PAMI_WTICK;
  result = PAMI_Configuration_query(client, &configuration);
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

    fprintf (stdout, "# PAMI_Send_immediate() nearest-neighor half-pingpong blocking latency performance test\n");
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

  barrier ();

  for (unsigned i = 0; i < 10000000;i++){};

  unsigned long long cycles;
  double usec;

  pami_endpoint_t origin, target;
  PAMI_Endpoint_create (client, 0, 0, &origin);
  PAMI_Endpoint_create (client, 1, 0, &target);

  char str[10240];
//	fprintf(stdout,"starting the test\n") ;
  size_t sndlen;
  for (sndlen = 0; sndlen < BUFSIZE; sndlen = sndlen*3/2+1)
  //sndlen = 40;
  {
    int index = 0;
    index += sprintf (&str[index], "%10zd ", sndlen);

    for (unsigned i=0; i<hdrcnt; i++)
    {
#ifdef WARMUP
      test (context, _dispatch[0], hdrsize[i], sndlen, _my_rank, origin, target);
#endif
      cycles = test (context, _dispatch[0], hdrsize[i], sndlen, _my_rank, origin, target);
      usec   = cycles * tick * 1000000.0;
      //index += sprintf (&str[index], "%7lld %7.4f  ", cycles, usec);
      index += sprintf (&str[index], "%7lld  ", cycles);
    }

    if (_my_rank == 0)
      fprintf (stdout, "%s\n", str);
  }

  PAMI_Client_destroy (client);

  return 0;
}
#undef TRACE_ERR
