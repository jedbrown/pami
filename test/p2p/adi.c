/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/p2p/adi.c
 *
 * \brief This is an example of the sort of functions expected to be
 *        used in an MPICH2 ADI
 *
 * It is probably easiest to read this file from the bottom up.  To
 * avoid having to create function prototypes, all the helper
 * functions are written before they are used.  As such, their purpose
 * is a little opaque.
 *
 * I plan to use the following (integral) functions to determine the
 * local and remote contexts/endpoints in MPICH2.  I use a variation
 * of them in the code below, since there are not communicators.
 *
 *  local_context  = ( remote_rank + comm_id ) % num_contexts
 *
 *    This means that task 0 sending to both tasks 1 and 2 will send
 *    from different contexts on 0.
 *
 * remote_endpoint = (  local_rank + comm_id ) % num_contexts
 *
 *    This means that tasks 0 and 1 both sending to task 2 will send
 *    to different endpoints on 2.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/xmi.h>


#define SSIZE 4    /**< This is the size (in bytes) of the small send */
#define LSIZE 2345 /**< This is the size (in bytes) of the long  send */


static xmi_client_t client;
#define MAX_CONTEXTS 64
static xmi_context_t contexts[MAX_CONTEXTS];
static const size_t SHORT_DISPATCH=1, LONG_DISPATCH=12; /**< These dispatch IDs were chosen mostly at random, save one is even and one is odd */

static size_t task, size, num_contexts;
static unsigned sbuf[SSIZE]; /**< The buffer for the short send */
static unsigned lbuf[LSIZE]; /**< The buffer for the long  send */
static struct
{
  struct
  {
    volatile unsigned send;
    volatile unsigned recv;
  } slong;
  struct
  {
    volatile unsigned send;
    volatile unsigned recv;
  } sshort;
} done = { {0,0}, {0,0} }; /**< Both the long and short messages have send and recv callbacks that must complete. */


/**
 * The long-recv is asynchronous; this is used to signal its
 * completion.  We also print out the rest of the information and mark
 * it done.
 */
static void RecvLongDoneCB(xmi_context_t   context,
                           void          * cookie,
                           xmi_result_t    result)
{
  assert(cookie != NULL);

  unsigned *data = (unsigned*)cookie;
  printf("Task=%zu Channel=%p <Fini long  msg>   data=%x\n", task, context, data[0]);
  done.slong.recv = 1;
  free(data);
}

/**
 * We are getting a new message on the dispatch for the long messages.
 *
 * The data needs to be received into a malloced buffer.  We print out
 * the available information set up the done call-back.
 */
static void RecvLongCB(xmi_context_t   context,
                       void          * cookie,
                       void          * _msginfo,
                       size_t          msginfo_size,
                       void          * _addr,
                       size_t          size,
                       xmi_recv_t    * recv)
{
  assert(_addr == NULL);
  assert(size > 0);
  assert(msginfo_size == 4*sizeof(unsigned));

  unsigned* msginfo   = (unsigned*)_msginfo;
  size_t    contextid = (size_t)cookie;

  void* buf = malloc(size);
  recv->local_fn = RecvLongDoneCB;
  recv->cookie   = buf;
  recv->kind     = XMI_AM_KIND_SIMPLE;
  recv->data.simple.addr  = buf;
  recv->data.simple.bytes = size;

  printf("Task=%zu Channel=%p(%zu) <Got   long msg>   remote=%d msginfo=%x len=%zu\n",
         task, context, contextid, msginfo[0], msginfo[1], size);
}

/**
 * We are getting a new message on the dispatch for the short messages.
 *
 * The data is already received, so we just print out the information
 * and mark it done.
 */
static void RecvShortCB(xmi_context_t   context,
                        void          * cookie,
                        void          * _msginfo,
                        size_t          msginfo_size,
                        void          * _addr,
                        size_t          size,
                        xmi_recv_t    * recv)
{
  assert(_addr != NULL);
  assert(size > 0);
  assert(msginfo_size == 4*sizeof(unsigned));

  unsigned* msginfo   = (unsigned*)_msginfo;
  unsigned* data      = (unsigned*)_addr;
  size_t    contextid = (size_t)cookie;
  printf("Task=%zu Channel=%p(%zu) <Got  short msg>   remote=%d msginfo=%x len=%zu data=%x\n",
         task, context, contextid, msginfo[0], msginfo[1], size, data[0]);
  done.sshort.recv = 1;
}

/**
 * The long-send is asynchronous; this is used to signal its
 * completion and mark it done.
 */
static void SendLongDoneCB(xmi_context_t   context,
                           void          * cookie,
                           xmi_result_t    result)
{
  assert(cookie != NULL);

  unsigned *data = (unsigned*)cookie;
  printf("Task=%zu Channel=%p <Sent long  msg>   data=%x\n", task, context, data[0]);
  done.slong.send = 1;
}

/**
 * This does the actual long-send using XMI_Send.
 */
static xmi_result_t SendLongHandoff(xmi_context_t   context,
                                    void          * cookie)
{
  assert(cookie != NULL);

  unsigned* quad = (unsigned*)cookie;

  xmi_task_t remote_task = 1-task;
  size_t remote_context = (task+LONG_DISPATCH)&(num_contexts-1);
  xmi_endpoint_t dest = XMI_Client_endpoint(client, remote_task, remote_context);

  xmi_send_t parameters = { {{0,0}, {0,0}}, {0} };
  parameters.send.dispatch        = LONG_DISPATCH;
/*parameters.send.hints           = {0}; */
  parameters.send.dest            = dest;
  parameters.send.header.iov_base = quad;
  parameters.send.header.iov_len  = 4*sizeof(unsigned);
  parameters.send.data.iov_base   = lbuf;
  parameters.send.data.iov_len    = LSIZE;
  parameters.events.cookie        = lbuf;
  parameters.events.local_fn      = SendLongDoneCB;
  parameters.events.remote_fn     = NULL;

  XMI_Send(context, &parameters);
  return XMI_SUCCESS;
}

/**
 * This calculates a context to use and posts the long-send work to
 * it.  It then advances all contexts until the send is complete.
 *
 * Because SendLongHandoff may complete before sending the msginfo, I
 * declare it here and pass it in the cookie.  Since this function
 * will not exit until the local sen is complete, it is safe to put
 * the msginfo on the stack for this one.
 */
static void *SendLong(void *c)
{
  unsigned quad[] = {(unsigned)task, 0x11111, 0x22222, 0x33333};

  xmi_task_t remote_task = 1-task;
  size_t local_context = (remote_task+LONG_DISPATCH)&(num_contexts-1);

  XMI_Context_post(contexts[local_context], SendLongHandoff, quad);
  while (!done.slong.send)
    XMI_Context_multiadvance(contexts, num_contexts, 1);
  return NULL;
}

/**
 * This does the actual short-send using XMI_Send_immediate.  This
 * will cause the data to be injected immediately, avoiding call-backs
 * and allowing us to declare more info on the stack.  We can mark it
 * done at the end.
 */
static xmi_result_t SendShortHandoff(xmi_context_t   context,
                                     void          * cookie)
{
  assert(cookie == NULL);

  unsigned quad[] = {(unsigned)task, 0x11, 0x22, 0x33};

  xmi_task_t remote_task = 1-task;
  size_t remote_context = (task+SHORT_DISPATCH)&(num_contexts-1);
  xmi_endpoint_t dest = XMI_Client_endpoint(client, remote_task, remote_context);

  xmi_send_immediate_t parameters = { {0,0}, {0,0}, 0 };
  parameters.dispatch        = SHORT_DISPATCH;
/*parameters.hints           = {0}; */
  parameters.dest            = dest;
  parameters.header.iov_base = quad;
  parameters.header.iov_len  = sizeof(quad);
  parameters.data.iov_base   = sbuf;
  parameters.data.iov_len    = SSIZE;

  XMI_Send_immediate(context, &parameters);
  printf("Task=%zu Channel=%p <Sent short msg>   data=%x\n", task, context, sbuf[0]);
  done.sshort.send = 1;
  return XMI_SUCCESS;
}

/**
 * This calculates a context to use and posts the short-send work to
 * it.  It then advances all contexts until the send is complete.
 */
static void *SendShort(void *c)
{
  xmi_task_t remote_task = 1-task;
  size_t local_context = (remote_task+SHORT_DISPATCH)&(num_contexts-1);

  XMI_Context_post(contexts[local_context], SendShortHandoff, NULL);
  while (!done.sshort.send)
    XMI_Context_multiadvance(contexts, num_contexts, 1);
  return NULL;
}

/**
 * This just advances all contexts until the messages are done.
 */
static void *advance(void* c)
{
  while (!
         (
          done.sshort.send &&
          done.sshort.recv &&
          done.slong.send &&
          done.slong.recv
         )
        )
    /* I'm using "13" for the poll-iterations just because I like the number */
    XMI_Context_multiadvance(contexts, num_contexts, 13);

  return NULL;
}

/**
 * \brief Initialize the XMI message layer.
 *
 * This gets the task ID, number of tasks, number of contexts, and
 * checks that each process has the same max number of contexts.  It
 * then creates the contexts initializes the dispatch tables.
 */
static void init()
{
  xmi_configuration_t query;
  xmi_send_hint_t options = {consistency:1};

  XMI_Client_initialize("XMId ADI Example", &client);

  query.name = XMI_TASK_ID;
  XMI_Configuration_query (client, &query);
  task = query.value.intval;

  query.name = XMI_NUM_TASKS;
  XMI_Configuration_query (client, &query);
  size = query.value.intval;
  assert(size > 1);

  query.name = XMI_NUM_CONTEXTS;
  XMI_Configuration_query (client, &query);
  num_contexts = query.value.intval;
  assert(num_contexts <= MAX_CONTEXTS);
  assert((num_contexts&(num_contexts-1)) == 0);

  query.name = XMI_CONST_CONTEXTS;
  XMI_Configuration_query (client, &query);
  assert(query.value.intval);

  query.value.intval = 1;
  XMI_Context_createv(client, &query, 1, contexts, num_contexts);

  xmi_dispatch_callback_fn RecvShortFN;
  RecvShortFN.p2p = RecvShortCB;
  xmi_dispatch_callback_fn RecvLongFN;
  RecvLongFN.p2p = RecvLongCB;

  /*
   * The context index is used for the dispatch cookie.  This allows
   * the call-back to identify the context index in use.
   */
  size_t i;
  for (i=0; i<num_contexts; ++i) {
    XMI_Dispatch_set(contexts[i],
                     SHORT_DISPATCH,
                     RecvShortFN,
                     (void*)i,
                     options);
    XMI_Dispatch_set(contexts[i],
                     LONG_DISPATCH,
                     RecvLongFN,
                     (void*)i,
                     options);
  }

  printf("Task=%zu Size=%zu    <XMI Initialized> thread-level=%d\n", task, size, 13);
}

/**
 * The goal of the application is to use threads to send/recv data.
 *    Thread 0: Initial thread, just manages the others.
 *    Thread 1: Advance thread, advances all contexts until the sends and recvs are done.
 *    Thread 2: Long-send thread.
 *    Thread 3: Short-send thread.
 */
int main()
{
  pthread_t threads[3];
  sbuf[0] = 0xaaaaa;
  lbuf[0] = 0xbbbbb;

  init();

  if (task < 2)
    {
      int rc;
      rc = pthread_create(threads+0, NULL, advance,  NULL);  assert(rc == 0);
      rc = pthread_create(threads+1, NULL, SendLong, NULL);  assert(rc == 0);
      rc = pthread_create(threads+2, NULL, SendShort, NULL); assert(rc == 0);

      pthread_join(threads[0], NULL);
      pthread_join(threads[1], NULL);
      pthread_join(threads[2], NULL);
    }

  XMI_Client_finalize(client);
  printf("Task=%zu           <DONE>\n", task);
  return 0;
}
