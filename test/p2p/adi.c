/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/p2p/adi.c
 * \brief ???
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/xmi.h>


#define SSIZE 2
#define LSIZE 2048


static xmi_client_t client;
#warning How do I determine the optimal number of contexts?
#define NUM_CONTEXTS 2
static xmi_context_t contexts[NUM_CONTEXTS];
static size_t SHORT_DISPATCH=1, LONG_DISPATCH=13;


static size_t rank, size;
static unsigned sbuf[SSIZE];
static unsigned lbuf[LSIZE];
static struct
{
  struct
  {
    volatile unsigned s;
    volatile unsigned r;
  } l;
  struct
  {
    volatile unsigned s;
    volatile unsigned r;
  } s;
} done = { {0,0}, {0,0} };


static void RecvLongDoneCB(xmi_context_t   context,
                    void          * cookie,
                    xmi_result_t    result)
{
  unsigned *data = (unsigned*)cookie;
  printf("Rank=%zu Channel=%p <Fini long  msg>   data=%x\n", rank, context, data[0]);
  done.l.r = 1;
  free(cookie);
}

static void RecvLongCB(xmi_context_t   context,
                       void          * cookie,
                       size_t          remote_task,
                       void          * _msginfo,
                       size_t          msginfo_size,
                       void          * _addr,
                       size_t          size,
                       xmi_recv_t    * recv)
{
  assert(_addr == NULL);
  assert(msginfo_size >= sizeof(unsigned));
  unsigned* msginfo = (unsigned*)_msginfo;

  void* buf = malloc(size);
  recv->local_fn = RecvLongDoneCB;
  recv->cookie   = buf;
  recv->kind     = XMI_AM_KIND_SIMPLE;
  recv->data.simple.addr  = buf;
  recv->data.simple.bytes = size;

#warning I need each channel to have an id number/index to write a multi-context recv-queue.
  printf("Rank=%zu Channel=%p <Got  short msg>   remote=%zu msginfo=%x len=%zu\n", rank, context, remote_task, msginfo[0], size);
}

static void RecvShortCB(xmi_context_t   context,
                        void          * cookie,
                        size_t          remote_task,
                        void          * _msginfo,
                        size_t          msginfo_size,
                        void          * _addr,
                        size_t          size,
                        xmi_recv_t    * recv)
{
  assert(_addr != NULL);
  assert(msginfo_size >= sizeof(unsigned));
  unsigned* msginfo = (unsigned*)_msginfo;
  unsigned* data    = (unsigned*)_addr;
  printf("Rank=%zu Channel=%p <Got  short msg>   remote=%zu msginfo=%x len=%zu data=%x\n", rank, context, remote_task, msginfo[0], size, data[0]);
  done.s.r = 1;
}

static void SendLongDoneCB(xmi_context_t   context,
                    void          * cookie,
                    xmi_result_t    result)
{
  unsigned *data = (unsigned*)cookie;
  printf("Rank=%zu Channel=%p <Sent long  msg>   data=%x\n", rank, context, data[0]);
  done.l.s = 1;
}

static void SendLongHandoff(xmi_context_t   context,
                     void          * cookie,
                     xmi_result_t    result)
{
  int quad[] = {0x111, 0x222, 0x333, 0x444};

  xmi_send_simple_t parameters = { {0}, {0} };
  parameters.send.dispatch     = LONG_DISPATCH;
  parameters.send.header.addr  = quad;
  parameters.send.header.bytes = sizeof(quad);
  parameters.send.cookie       = lbuf;
  parameters.simple.addr       = lbuf;
  parameters.simple.bytes      = LSIZE;
  parameters.simple.local_fn   = SendLongDoneCB;

  XMI_Send(context, &parameters);
}

static void *SendLong(void *clientdata)
{
  XMI_Context_post(contexts[0], SendLongHandoff, NULL);
  while (!done.s.s)
    XMI_Context_multiadvance(contexts, NUM_CONTEXTS, 1);
  return NULL;
}

static void SendShortHandoff(xmi_context_t   context,
                      void          * cookie,
                      xmi_result_t    result)
{
  int quad[] = {0x111, 0x222, 0x333, 0x444};

  xmi_send_immediate_t parameters = { {0}, {0} };
  parameters.send.dispatch     = SHORT_DISPATCH;
  parameters.send.header.addr  = quad;
  parameters.send.header.bytes = sizeof(quad);
  parameters.immediate.addr    = sbuf;
  parameters.immediate.bytes   = SSIZE;

  XMI_Send_immediate(context, &parameters);
  printf("Rank=%zu Channel=%p <Sent short msg>   data=%x\n", rank, context, sbuf[0]);
  done.s.s = 1;
}

static void *SendShort(void *clientdata)
{
  XMI_Context_post(contexts[1], SendShortHandoff, NULL);
  while (!done.s.s)
    XMI_Context_multiadvance(contexts, NUM_CONTEXTS, 1);
  return NULL;
}

static void *advance(void* c)
{
  while (!(done.s.s && done.s.r && done.l.s && done.l.r))
    XMI_Context_multiadvance(contexts, NUM_CONTEXTS, 13);

  return NULL;
}

static void init()
{
  xmi_configuration_t query;
  xmi_send_hint_t options = {consistency:1};

  xmi_dispatch_callback_fn RecvShortFN;
  RecvShortFN.p2p = RecvShortCB;
  xmi_dispatch_callback_fn RecvLongFN;
  RecvLongFN.p2p = RecvLongCB;

  XMI_Client_initialize("XMId ADI Example", &client);

#warning Do I really have to loop to create all the contexts?
  unsigned i;
  for (i=0; i<NUM_CONTEXTS; ++i) {
    XMI_Context_create(client, NULL, 0, contexts+i);
    XMI_Dispatch_set(contexts[i],
                     SHORT_DISPATCH,
                     RecvShortFN,
                     NULL,
                     options);
    XMI_Dispatch_set(contexts[i],
                     LONG_DISPATCH,
                     RecvLongFN,
                     NULL,
                     options);
  }

  query.name = XMI_TASK_ID;
  XMI_Configuration_query (contexts[0], &query);
  rank = query.value.intval;

  query.name = XMI_NUM_TASKS;
  XMI_Configuration_query (contexts[0], &query);
  size = query.value.intval;

#warning We need to clairify the threading nature of XMI
  /* dcmf_config.thread_level = DCMF_THREAD_MULTIPLE; */

  printf("Rank=%zu Size=%zu    <XMI Initialized> thread-level=%d\n", rank, size, 13);
}

int main()
{
  pthread_t threads[3];
  sbuf[0] = 0xaaaaa;
  lbuf[0] = 0xbbbbb;

  init();

  pthread_create(threads+0, NULL, advance,  NULL);
  pthread_create(threads+1, NULL, SendLong, NULL);
  pthread_create(threads+2, NULL, SendShort, NULL);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);

  XMI_Client_finalize(client);
  printf("Rank=%zu           <DONE>\n", rank);
  return 0;
}
