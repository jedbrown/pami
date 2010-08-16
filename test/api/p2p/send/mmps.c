/*
 *  \file test/api/context/post-multithreaded-perf.c
 *  \brief PAMI_Context_post() performance test using comm-threads
 */

#define  TRACE_ON

#define NCONTEXTS  64      /* The maximum number of contexts */
#define WINDOW     16      /* The number of sends/recvs before a "wait" */
#define ITERATIONS (1<<5)  /* The number of windows exectuted */
#define HEADER     16      /* Size of header */
#define DATA       0       /* Size of data */
#define MAX_SIZE   32      /* The number of processes that can participate */

#define DISPATCH   0       /* Dispatch ID */


/* #define NDEBUG */
#define MIN(a,b) ((a<b)?a:b)

#ifdef TRACE_ON
#ifdef __GNUC__
#define TRACE_ALL(fd, format, ...) fprintf(fd, "(rank=%02zu,line=%03u,func=%s) " format, rank, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRACE_OUT(format, ...) TRACE_ALL(stdout, format, ##__VA_ARGS__)
#define TRACE_ERR(format, ...) TRACE_ALL(stderr, format, ##__VA_ARGS__)
#else
#define TRACE_OUT(format...) fprintf(stdout, format)
#define TRACE_ERR(format...) fprintf(stderr, format)
#endif
#else
#define TRACE_OUT(format...)
#define TRACE_ERR(format...)
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pami.h>
#undef __pami_target_bgq__ /** \todo Use the BGQ extentions when they are working better */
#ifdef __pami_target_bgq__
extern pami_result_t
PAMI_Client_add_commthread_context(pami_client_t client, pami_context_t context);
#endif


static pami_client_t  client;
static pami_context_t contexts[NCONTEXTS];
static volatile size_t recv_list[MAX_SIZE] = {0};
static volatile size_t send_list[MAX_SIZE] = {0};
static size_t ncontexts = NCONTEXTS;
static size_t rank=99, size=(size_t)-1;
#ifndef __pami_target_bgq__
static pthread_t threads[NCONTEXTS];
#endif


#ifndef __pami_target_bgq__
#include <pthread.h>
static void*
advance(void* arg)
{
  pami_result_t rc;
  pami_context_t context = *(pami_context_t*)arg;
  TRACE_ERR("context=%p\n", context);
  for (;;) {
    rc = PAMI_Context_advance(context, (size_t)-1);
    assert(rc == PAMI_SUCCESS);
  }
  return NULL;
}
#endif


static pami_result_t
send(pami_context_t context, void *_dest)
{
  const size_t mod = ncontexts>>1;
  pami_result_t rc;
  size_t dest = (size_t)_dest;
  TRACE_ERR("running posted work  dest=%zu[%zu] arg=%p\n", dest, (rank%mod)+mod, _dest);

  struct
  {
    char header[HEADER];
    char data[DATA];
  } buffer;

  pami_send_immediate_t params = {
  header : {
    iov_base : &buffer.header,
    iov_len  : HEADER,
  },
  data : {
    iov_base : &buffer.data,
    iov_len  : DATA,
  },
  dispatch : DISPATCH,
  };
  rc = PAMI_Endpoint_create(client, dest, (rank%mod)+mod, &params.dest);
  assert(rc == PAMI_SUCCESS);
  rc = PAMI_Send_immediate(context, &params);
  assert(rc == PAMI_SUCCESS);

  ++send_list[dest];
  return PAMI_SUCCESS;
}


static void
recv(pami_context_t    context,
     void            * _contextid,
     const void      * _msginfo,
     size_t            msginfo_size,
     const void      * sndbuf,
     size_t            sndlen,
     pami_endpoint_t   _sender,
     pami_recv_t     * recv)
{
  assert(recv == NULL);

  pami_result_t rc;
  pami_task_t sender;
  size_t contextid;

  rc = PAMI_Endpoint_query(_sender, &sender, &contextid);
  assert(rc == PAMI_SUCCESS);

  TRACE_ERR("dest=%zu[%zu] contextid=%zu\n", (size_t)sender, contextid, (size_t)_contextid);
  ++recv_list[sender];
}


static double
master()
{
  TRACE_ERR("Starting master\n");

  const size_t mod = ncontexts>>1;
  size_t dest;
  pami_result_t rc;
  size_t iteration, i;
  pami_work_t work_list[WINDOW][size];

  for (iteration=0; iteration<ITERATIONS; ++iteration) {
    TRACE_ERR("starting sends  interation=%zu\n", iteration);
    for (i=0; i<WINDOW; ++i) {
      for (dest=1; dest<size; ++dest) {
        size_t contextid = dest%mod;
        pami_context_t context = contexts[contextid];
        TRACE_ERR("Posting work  contextid=%zu dest=%zu arg=%p iteration=%zu window=%zu\n", contextid, dest, (void*)dest, iteration, i);
        rc = PAMI_Context_post(context, &work_list[i][dest], send, (void*)dest);
        assert(rc == PAMI_SUCCESS);
      }
    }

    TRACE_ERR("Starting completion check  interation=%zu\n", iteration);
    /* Check that everything is done */
    for (dest=1; dest<size; ++dest) {
      TRACE_ERR("           interation=%zu dest=%zu\n", iteration, dest);
      while(send_list[dest] < WINDOW*(iteration+1));
      TRACE_ERR("send done  interation=%zu dest=%zu\n", iteration, dest);
      while(recv_list[dest] < WINDOW*(iteration+1));
      TRACE_ERR("recv done  interation=%zu dest=%zu\n", iteration, dest);
    }
  }
  return 0.0;
}


static void
worker()
{
  TRACE_ERR("Starting worker\n");

  const size_t mod = ncontexts>>1;
  const size_t dest = 0;
  pami_result_t rc;
  size_t iteration, i;
  pami_work_t work_list[WINDOW][1];

  for (iteration=0; iteration<ITERATIONS; ++iteration) {
    TRACE_ERR("starting sends  interation=%zu\n", iteration);
    for (i=0; i<WINDOW; ++i) {
      {
        size_t contextid = dest%mod;
        pami_context_t context = contexts[contextid];
        TRACE_ERR("Posting work  contextid=%zu dest=%zu arg=%p iteration=%zu window=%zu\n", contextid, dest, (void*)dest, iteration, i);
        rc = PAMI_Context_post(context, &work_list[i][dest], send, (void*)dest);
        assert(rc == PAMI_SUCCESS);
      }
    }

    /* Check that everything is done */
    TRACE_ERR("Starting completion check  interation=%zu\n", iteration);
    {
      TRACE_ERR("           interation=%zu dest=%zu\n", iteration, dest);
      while(send_list[dest] < WINDOW*(iteration+1));
      TRACE_ERR("send done  interation=%zu dest=%zu\n", iteration, dest);
      while(recv_list[dest] < WINDOW*(iteration+1));
      TRACE_ERR("recv done  interation=%zu dest=%zu\n", iteration, dest);
    }
  }
}


static pami_configuration_t
client_query(pami_client_t         client,
             pami_attribute_name_t name)
{
  pami_result_t rc;
  pami_configuration_t query;
  query.name = name;
  rc = PAMI_Client_query(client, &query, 1);
  assert(rc == PAMI_SUCCESS);
  return query;
}


static void
init()
{
  size_t i;
  pami_result_t rc;
  pami_configuration_t configuration;

  rc = PAMI_Client_create("TEST", &client, NULL, 0);
  assert(rc == PAMI_SUCCESS);

  rank = client_query(client, PAMI_CLIENT_TASK_ID).value.intval;
  size = client_query(client, PAMI_CLIENT_NUM_TASKS).value.intval;
  assert(size > 1);

  assert(client_query(client, PAMI_CLIENT_CONST_CONTEXTS).value.intval != 0);

  size_t query;
  query = client_query(client, PAMI_CLIENT_NUM_CONTEXTS).value.intval;
  ncontexts = MIN(ncontexts, query);
  query = client_query(client, PAMI_CLIENT_HWTHREADS_AVAILABLE).value.intval;
  assert(query > 2); /* This requires a send helper and a recv helper */
  /* ncontexts = MIN(ncontexts, query-2); */
  ncontexts = MIN(ncontexts, query);

  ncontexts &= ~(size_t)1;  /* Make it even */
  assert(ncontexts>1); /* There must be at least 2 */
  assert((ncontexts&1) == 0); /* There must be an even number */

  pami_configuration_t params = {
  name  : PAMI_CLIENT_CONST_CONTEXTS,
  value : { intval : 1, },
  };
  rc = PAMI_Context_createv(client, &params, 1, contexts, ncontexts);
  TRACE_ERR("%d = PAMI_Context_createv(client=%p, {PAMI_CLIENT_CONST_CONTEXTS = 1}, contexts=%p, ncontexts=%zu);\n", rc, client, contexts, ncontexts);
  assert(rc == PAMI_SUCCESS);

  for (i=0; i<ncontexts; ++i) {
  /** \todo Add back in when trac 242 is done */
#ifdef TRAC_242_IS_FIXED
    configuration.name = PAMI_CONTEXT_DISPATCH_ID_MAX;
    rc = PAMI_Context_query(contexts[i], &configuration, 1);
    assert(rc == PAMI_SUCCESS);
    assert(configuration.value.intval >= DISPATCH);
#endif

    pami_dispatch_callback_fn _recv = {p2p:recv};
    pami_send_hint_t options = {
    consistency:    PAMI_HINT2_ON,
    no_long_header: PAMI_HINT2_ON,
    };
    PAMI_Dispatch_set(contexts[i],
                      DISPATCH,
                      _recv,
                      (void*)i,
                      options);

    configuration.name = PAMI_DISPATCH_RECV_IMMEDIATE_MAX;
    rc = PAMI_Dispatch_query(contexts[i], DISPATCH, &configuration, 1);
    assert(rc == PAMI_SUCCESS);
    assert(configuration.value.intval >= (HEADER+DATA));
    configuration.name = PAMI_DISPATCH_SEND_IMMEDIATE_MAX;
    rc = PAMI_Dispatch_query(contexts[i], DISPATCH, &configuration, 1);
    assert(rc == PAMI_SUCCESS);
    assert(configuration.value.intval >= (HEADER+DATA));

#ifdef __pami_target_bgq__
      rc = PAMI_Client_add_commthread_context(client, contexts[i]);
      assert(rc == PAMI_SUCCESS);
#else
      int result;
      result = pthread_create(&threads[i], NULL, advance, &contexts[i]);
      assert(result == 0);
#endif
  }
}


int
main(int argc, char **argv)
{
  TRACE_ERR("Before init\n");
  init();


  TRACE_ERR("after init, starting master/worker\n");
  if (rank==0)
    master();
  else if (rank < MAX_SIZE)
    worker();
  else
    ;

  TRACE_ERR("Comm done on this process\n");

#ifdef __pami_target_bgq__
  pami_result_t rc;

  rc = PAMI_Context_destroyv(contexts, ncontexts);
  assert(rc == PAMI_SUCCESS);

  rc = PAMI_Client_destroy(&client);
  assert(rc == PAMI_SUCCESS);
#endif

  return 0;
};
