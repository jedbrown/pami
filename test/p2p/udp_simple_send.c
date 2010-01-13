/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/p2p/udp_simple_send.c
 * \brief ???
 */
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "sys/xmi.h"
#include "../util.h"


#define ITERATIONS 100
//#define ITERATIONS 1


#define BUFSIZE 255


#define WARMUP

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  fprintf x
#endif

volatile unsigned _send_active;
volatile unsigned _recv_active;
volatile unsigned _recv_iteration;
char              _recv_buffer[BUFSIZE] __attribute__ ((__aligned__(16)));

size_t         _dispatch[100];
unsigned       _dispatch_count;

//char buffer[BUFSIZE];

size_t _my_rank;

//typedef struct
//{
//  size_t sndlen;
//} header_t;

void printHexLine2( char * data, size_t num, size_t pad )
{
  size_t i;
  std::cout << "<x>: " << (void *)data << ": ";
  for ( i=0; i<num; i++ )
  {
    printf("%02x ", (uint8_t)data[i]);
  }
  for ( i=0; i<pad; i++ )
  {
    printf("   ");
  }
  printf(" ");
  for ( i=0; i<num; i++ )
  {
    if ( isgraph((unsigned)data[i]))
    {
      printf ("%c", data[i] );
    } else {
      printf(".");
    }
  }
  printf("\n");
}
/* --------------------------------------------------------------- */

static void rcvdecrement (xmi_context_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  unsigned * value = (unsigned *) cookie;
  TRACE_ERR((stderr, "(%zd) ***** in rcvdecrement() cookie = %p, %d => %d\n", _my_rank, cookie, *value, *value-1));
  std::cout << _recv_buffer << std::endl;
  --*value;
}

static void snddecrement (xmi_context_t   context,
                       void          * cookie,
                       xmi_result_t    result)
{
  unsigned * value = (unsigned *) cookie;
  TRACE_ERR((stderr, "(%zd) ***** in snddecrement() cookie = %p, %d => %d\n", _my_rank, cookie, *value, *value-1));
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
   std::cout << "****** in test_dispatch() " << std::endl;
  if (pipe_addr != NULL)
  {
    memcpy (_recv_buffer, pipe_addr, pipe_size);
    std::cout << "Msg = " << _recv_buffer << std::endl;
    unsigned * value = (unsigned *) cookie;
    TRACE_ERR((stderr, "(%zd) ****** in test_dispatch() short recv:  cookie = %p, decrement: %d => %d\n", _my_rank, cookie, *value, *value-1));
    --*value;
  }
  else
  {
    //header_t * header = (header_t *) header_addr;
    TRACE_ERR((stderr, "(%zd) ****** in test_dispatch() async recv:  cookie = %p, pipe_size = %zd\n", _my_rank, cookie, pipe_size));

    recv->local_fn = rcvdecrement;
    recv->cookie   = cookie;
    recv->kind = XMI_AM_KIND_SIMPLE;
    recv->data.simple.addr  = _recv_buffer;
    recv->data.simple.bytes = pipe_size;

  }

  _recv_iteration++;
}

void send_once (xmi_context_t context, xmi_send_t * parameters)
{
 std::cout << __FILE__ << ":" << __LINE__ << std::endl;
  xmi_result_t result = XMI_Send (context, parameters);
  TRACE_ERR((stderr, "(%zd) send_once() Before advance\n", _my_rank));
  while (_send_active) XMI_Context_advance (context, 100);
  _send_active = 1;
  TRACE_ERR((stderr, "(%zd) send_once()  After advance\n", _my_rank));
}

void recv_once (xmi_context_t context)
{
 std::cout << __FILE__ << __LINE__ << std::endl;
  TRACE_ERR((stderr, "(%zd) recv_once() Before advance\n", _my_rank));
  while (_recv_active) XMI_Context_advance (context, 100);
  //print received buffer
  fprintf (stdout, "\n Received Message = %s\n",_recv_buffer);
  printHexLine2( _recv_buffer, 16, 0 );
  fflush (stdout);
  _recv_active = 1;
  TRACE_ERR((stderr, "(%zd) recv_once()  After advance\n", _my_rank));
}

unsigned long long test (xmi_context_t context, size_t dispatch, size_t hdrsize, size_t sndlen, size_t myrank)
{
  std::cout << __FILE__ << __LINE__ << std::endl;
  _recv_active = 1;
  _recv_iteration = 0;
  _send_active = 1;

  char metadata[BUFSIZE];
  char buffer[255];
  size_t sndlen1;
  unsigned i;
  xmi_send_t parameters;

  if (myrank == 0)
  {
    fprintf (stdout, "\n Enter the message to send: ");
    fflush (stdout);
    fgets(buffer,sizeof(buffer),stdin);
    //scanf("%s",buffer);
    sndlen1 =strlen(buffer);
    printHexLine2( buffer, sndlen1, 0 );

    TRACE_ERR((stderr, "(%zd) Do test ... sndlen = %zd\n", myrank, sndlen1));

    //header_t header;
    //header.sndlen = sndlen1;


    parameters.send.dispatch        = dispatch;
    parameters.send.header.iov_base = metadata;
    parameters.send.header.iov_len  = hdrsize;
    parameters.send.data.iov_base   = buffer;
    parameters.send.data.iov_len    = sndlen1;
    parameters.events.cookie        = (void *) &_send_active;
    parameters.events.local_fn      = snddecrement;
    parameters.events.remote_fn     = NULL;

    //barrier ();



    parameters.send.task = 1;
    for (i = 0; i < ITERATIONS; i++)
    {
      TRACE_ERR((stderr, "(%zd) Starting Iteration %d of size %zd\n", myrank, i, ITERATIONS));
      send_once (context, &parameters);
    }
  }
  else if (myrank == 1)
  {
    for (i = 0; i < ITERATIONS; i++)
    {
      TRACE_ERR((stderr, "(%zd) Starting Iteration %d of size %zd\n", myrank, i, ITERATIONS));
      recv_once (context);
    }
  }

  return 0;
}

int main (int argc, char ** argv)
{
 std::cout << __FILE__ << __LINE__ << std::endl;
  TRACE_ERR((stderr, "Start test ...\n"));

  size_t hdrcnt = 1;
  size_t hdrsize =0;


  char clientname[] = "XMI";
  xmi_client_t client;
  TRACE_ERR((stderr, "... before XMI_Client_initialize()\n"));
  XMI_Client_initialize (clientname, &client);
  TRACE_ERR((stderr, "...  after XMI_Client_initialize()\n"));
  xmi_context_t context;
  TRACE_ERR((stderr, "... before XMI_Context_create()\n"));
  { int _n = 1; XMI_Context_createv (client, NULL, 0, &context, &_n); }
  TRACE_ERR((stderr, "...  after XMI_Context_create()\n"));

  //TRACE_ERR((stderr, "... before barrier_init()\n"));
  //barrier_init (client, context, 0);
  //TRACE_ERR((stderr, "...  after barrier_init()\n"));


  /* Register the protocols to test */
  char testcase_str[10240];
  unsigned i, j, k = 0;
  _dispatch_count = 0;

  _dispatch[_dispatch_count] = _dispatch_count + 1;

  size_t dispatch = 1;
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options={0};
  TRACE_ERR((stderr, "Before XMI_Dispatch_set() .. &_recv_active = %p, recv_active = %zd\n", &_recv_active, _recv_active));
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
  _my_rank = configuration.value.intval;
  std::cout << "Rank = " << _my_rank << std::endl;

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query(client, &configuration);
  size_t num_tasks = configuration.value.intval;
  std::cout << "Size = " << num_tasks << std::endl;

  configuration.name = XMI_WTICK;
  result = XMI_Configuration_query(client, &configuration);
  double tick = configuration.value.doubleval;

   size_t val = argc;


   if (argc==1)
       val=1;
   else
       val =atoi(argv[1]) ;

	fprintf (stdout, "** The test will run %d times ***\n", val);
    fflush(stdout);

  for(i=0;i<val;i++){

//	  /* Display some test header information */
//	  if (_my_rank == 0)
//	  {

		   test (context, _dispatch[0], hdrsize, 0, _my_rank);

//	  }
  }
	fprintf (stdout, "** Test completed. **\n");

  XMI_Client_finalize (client);

  return 0;
}
#undef TRACE_ERR
