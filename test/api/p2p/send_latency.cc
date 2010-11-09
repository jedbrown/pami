/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/p2p/send_latency.cc
 * \brief ???
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <pami.h>

#define ITERATIONS 100
//#define ITERATIONS 1000
//#define ITERATIONS 1

#ifndef BUFSIZE
#define BUFSIZE 262144
//#define BUFSIZE 1024*256
//#define BUFSIZE 16
//#define BUFSIZE 1024
#endif

#define WARMUP

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  // fprintf x
#endif

#ifdef ENABLE_MAMBO_WORKAROUNDS
extern "C" unsigned __isMambo();
#endif


volatile unsigned _send_active = 1;
volatile unsigned _recv_active = 1;
volatile unsigned _recv_iteration;
char              _recv_buffer[BUFSIZE] __attribute__ ((__aligned__(16)));

size_t         _dispatch[100];
unsigned       _dispatch_count;

size_t _my_task;

typedef struct
{
  size_t sndlen;
} header_t;

/* --------------------------------------------------------------- */

static void decrement (pami_context_t   context,
                       void          * cookie,
                       pami_result_t    result)
{
  unsigned * value = (unsigned *) cookie;
  TRACE_ERR((stderr, "(%zu) decrement() cookie = %p, %d => %d\n", _my_task, cookie, *value, *value-1));
  --*value;
}

/* --------------------------------------------------------------- */
static void test_dispatch (
    pami_context_t        context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_endpoint_t      origin,
    pami_recv_t         * recv)        /**< OUT: receive message structure */
{
  if (recv)
  {
    //header_t * header = (header_t *) header_addr;
    TRACE_ERR((stderr, "(%zu) test_dispatch() async recv:  cookie = %p, pipe_size = %zu\n", _my_task, cookie, pipe_size));    
    recv->local_fn = decrement;
    recv->cookie   = cookie;
    recv->type     = PAMI_BYTE;
    recv->addr     = _recv_buffer;
    recv->offset   = 0;
    recv->data_fn  = PAMI_DATA_COPY;
  }
  else
  {
    //memcpy (_recv_buffer, pipe_addr, pipe_size);
    unsigned * value = (unsigned *) cookie;
    TRACE_ERR((stderr, "(%zu) test_dispatch() short recv:  cookie = %p, decrement: %d => %d\n", _my_task, cookie, *value, *value-1));
    --*value;
  }
  _recv_iteration++;
}

void send_once (pami_context_t context, pami_send_t * parameters)
{
  //pami_result_t result =
    PAMI_Send (context, parameters);
  TRACE_ERR((stderr, "(%zu) send_once() Before advance\n", _my_task));
  while (_send_active) PAMI_Context_advance (context, 100);

  _send_active = 1;
  TRACE_ERR((stderr, "(%zu) send_once()  After advance\n", _my_task));
}

void recv_once (pami_context_t context)
{
  TRACE_ERR((stderr, "(%zu) recv_once() Before advance\n", _my_task));
  while (_recv_active) PAMI_Context_advance (context, 100);

  _recv_active = 1;
  TRACE_ERR((stderr, "(%zu) recv_once()  After advance\n", _my_task));
}

unsigned long long test (pami_context_t context, size_t dispatch, size_t hdrsize, size_t sndlen, pami_task_t mytask, pami_task_t origintask, pami_endpoint_t origin, pami_task_t targettask, pami_endpoint_t target)
{
  TRACE_ERR((stderr, "(%u) Do test ... sndlen = %zu\n", mytask, sndlen));
  _recv_iteration = 0;

  char metadata[BUFSIZE];
  char buffer[BUFSIZE];

  header_t header;
  header.sndlen = sndlen;

  pami_send_t parameters;
  parameters.send.dispatch        = dispatch;
  parameters.send.header.iov_base = metadata;
  parameters.send.header.iov_len  = hdrsize;
  parameters.send.data.iov_base   = buffer;
  parameters.send.data.iov_len    = sndlen;
  parameters.events.cookie        = (void *) &_send_active;
  parameters.events.local_fn      = decrement;
  parameters.events.remote_fn     = NULL;
  memset(&parameters.send.hints, 0, sizeof(parameters.send.hints));

//  parameters.send.hints.use_rdma = 0;
  
//  barrier ();

  unsigned i;
  unsigned long long t1 = PAMI_Wtimebase();
  if (mytask == origintask)
  {
    parameters.send.dest = target;
    for (i = 0; i < ITERATIONS; i++)
    {
      TRACE_ERR((stderr, "(%u) Starting Iteration %d of size %zu\n", mytask, i, sndlen));
      send_once (context, &parameters);
      recv_once (context);
    }
  }
  else if (mytask == targettask)
  {
    parameters.send.dest = origin;
    for (i = 0; i < ITERATIONS; i++)
    {
      TRACE_ERR((stderr, "(%u) Starting Iteration %d of size %zu\n", mytask, i, sndlen));
      recv_once (context);
      send_once (context, &parameters);
    }
  }
  unsigned long long t2 = PAMI_Wtimebase();

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

  char clientname[] = "TEST";
  pami_client_t client;
  TRACE_ERR((stderr, "... before PAMI_Client_create()\n"));
  PAMI_Client_create (clientname, &client, NULL, 0);
  TRACE_ERR((stderr, "...  after PAMI_Client_create()\n"));
  pami_context_t context;
  TRACE_ERR((stderr, "... before PAMI_Context_createv()\n"));
  { size_t _n = 1; PAMI_Context_createv (client, NULL, 0, &context, _n); }
  TRACE_ERR((stderr, "...  after PAMI_Context_createv()\n"));

//  TRACE_ERR((stderr, "... before barrier_init()\n"));
//  barrier_init (client, context, 0);
//  TRACE_ERR((stderr, "...  after barrier_init()\n"));


  /* Register the protocols to test */
  _dispatch_count = 0;

  _dispatch[_dispatch_count] = _dispatch_count + 1;

  pami_dispatch_callback_function fn;
  fn.p2p = test_dispatch;
  pami_send_hint_t options={0};
  TRACE_ERR((stderr, "Before PAMI_Dispatch_set() .. &_recv_active = %p, recv_active = %u\n", &_recv_active, _recv_active));
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

  configuration.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query(client, &configuration,1);
  _my_task = configuration.value.intval;

  configuration.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query(client, &configuration,1);
  size_t num_tasks = configuration.value.intval;

  configuration.name = PAMI_CLIENT_WTICK;
  result = PAMI_Client_query(client, &configuration,1);
  double tick = configuration.value.doubleval;

  // Use task 0 to last task (arbitrary)
  pami_task_t origin_task = 0;
  pami_task_t target_task = num_tasks -1;

  /* Display some test header information */
  if (_my_task == origin_task)
  {
    char str[2][1024];
    int index[2];
    index[0] = 0;
    index[1] = 0;

    index[0] += sprintf (&str[0][index[0]], "#          ");
    index[1] += sprintf (&str[1][index[1]], "#    bytes ");

    fprintf (stdout, "# PAMI_Send() nearest-neighor half-pingpong blocking latency performance test\n");
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

//  barrier ();

  unsigned long long cycles;
  double usec;

  char str[10240];

  pami_endpoint_t origin, target;
  PAMI_Endpoint_create (client, origin_task, 0, &origin);
  PAMI_Endpoint_create (client, target_task, 0, &target);

#ifdef ENABLE_MAMBO_WORKAROUNDS
  size_t sndlen = __isMambo()? 1 : 0;
#else
  size_t sndlen = 0;
#endif
  for (; sndlen < BUFSIZE; sndlen = sndlen*3/2+1)
  {
    int index = 0;
    index += sprintf (&str[index], "%10zd ", sndlen);

    unsigned i;
    for (i=0; i<hdrcnt; i++)
    {
#ifdef WARMUP
      test (context, _dispatch[0], hdrsize[i], sndlen, _my_task, origin_task, origin, target_task, target);
#endif
      cycles = test (context, _dispatch[0], hdrsize[i], sndlen, _my_task, origin_task, origin, target_task, target);
      usec   = cycles * tick * 1000000.0;
      index += sprintf (&str[index], "%8lld %8.4f  ", cycles, usec);
    }

    if (_my_task == origin_task)
      fprintf (stdout, "%s\n", str);
  }

  PAMI_Client_destroy(&client);

  return 0;
}
#undef TRACE_ERR
