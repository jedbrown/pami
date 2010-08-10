/*
 *  \file test/api/context/post-multithreaded-perf.c
 *  \brief PAMI_Context_post() performance test using comm-threads
 */

#define NCONTEXTS  2
#define ITERATIONS (1<<17)


#define NDEBUG
#define MIN(a,b) ((a<b)?a:b)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pami.h>
extern pami_result_t PAMI_Client_add_commthread_context(pami_client_t client, pami_context_t context);


#ifndef __pami_target_bgq__
#include <pthread.h>
static void*
advance(void* arg)
{
  pami_context_t context = *(pami_context_t*)arg;
  /* printf("%s:%d  context=%p\n", __FUNCTION__, __LINE__, context); */
  for (;;)
    PAMI_Context_advance(context, (size_t)-1);
  return NULL;
}
#endif


static pami_result_t
posted(pami_context_t context, void *cookie)
{
  volatile size_t* value = (volatile size_t*)cookie;
  ++(*value);
  return PAMI_SUCCESS;
}


static pami_result_t
finish(pami_context_t context, void *cookie)
{
  /* printf("%s:%d\n", __FUNCTION__, __LINE__); */
  volatile size_t* value = (volatile size_t*)cookie;
  *value = 1;
  return PAMI_SUCCESS;
}


static void
test(pami_context_t contexts[], size_t ncontexts, size_t iterations, uint64_t* _posted_time, uint64_t* _done_time)
{
  pami_result_t rc;
  size_t context, i;
  uint64_t start_time, posted_time, done_time;

  volatile size_t posted_list   [NCONTEXTS];             memset((void*)posted_list, 0, NCONTEXTS*sizeof(size_t));
  pami_work_t     posted_request[NCONTEXTS][iterations]; memset(posted_request,     0, NCONTEXTS*sizeof(pami_work_t)*iterations);
  volatile size_t finish_list   [NCONTEXTS];             memset((void*)finish_list, 0, NCONTEXTS*sizeof(size_t));
  pami_work_t     finish_request[NCONTEXTS];             memset(finish_request,     0, NCONTEXTS*sizeof(pami_work_t));

  start_time = PAMI_Wtimebase();

  for (i=0; i<iterations; ++i) {
    for (context = 1; context<ncontexts; ++context) {
      rc = PAMI_Context_post(contexts[context],
                             &posted_request[context][i],
                             posted,
                             (void*)&posted_list[context]);
      assert(rc == PAMI_SUCCESS);
    }
  }

  posted_time = PAMI_Wtimebase();

  for (context = 1; context<ncontexts; ++context) {
    rc = PAMI_Context_post(contexts[context],
                           &finish_request[context],
                           finish,
                           (void*)&finish_list[context]);
    assert(rc == PAMI_SUCCESS);
  }

  for (context = 1; context<ncontexts; ++context) {
    while (!finish_list[context]) {
      rc = PAMI_Context_advance(contexts[0], 1);
      assert(rc == PAMI_SUCCESS);
    }
    assert(posted_list[context] == iterations);
  }

  done_time = PAMI_Wtimebase();


  *_posted_time = posted_time - start_time;
  *_done_time   = done_time   - start_time;
}


int
main(int argc, char **argv)
{
  pami_result_t rc;

  pami_client_t  client;
  pami_context_t contexts[NCONTEXTS];
  size_t ncontexts = NCONTEXTS;
#ifndef __pami_target_bgq__
  pthread_t threads[NCONTEXTS];
#endif

  rc = PAMI_Client_create("TEST", &client, NULL, 0);
  assert(rc == PAMI_SUCCESS);

  {
    pami_configuration_t configuration;

    configuration.name = PAMI_CLIENT_CONST_CONTEXTS;
    rc = PAMI_Client_query(client, &configuration, 1);
    assert(rc == PAMI_SUCCESS);
    assert(configuration.value.intval != 0);

/* We don't use this because NUM_CONTEXTS always returns 1 */
#if 0
    configuration.name = PAMI_CLIENT_NUM_CONTEXTS;
    rc = PAMI_Client_query(client, &configuration, 1);
    assert(rc == PAMI_SUCCESS);
    ncontexts = MIN(ncontexts, configuration.value.intval);
#endif

/* We don't use this because HWTHREADS_AVAILABLE isn't implemented */
#if 0
    configuration.name = PAMI_CLIENT_HWTHREADS_AVAILABLE;
    rc = PAMI_Client_query(client, &configuration, 1);
    assert(rc == PAMI_SUCCESS);
    ncontexts = MIN(ncontexts, configuration.value.intval);
#endif
  }

  assert(ncontexts > 1);
  pami_configuration_t params = {
  name  : PAMI_CLIENT_CONST_CONTEXTS,
  value : { intval : 1, },
  };
  rc = PAMI_Context_createv(client, &params, 1, contexts, ncontexts);
  assert(rc == PAMI_SUCCESS);


  {
    size_t context;
    for (context = 1; context<ncontexts; ++context) {
#ifdef __pami_target_bgq__
      rc = PAMI_Client_add_commthread_context(client, contexts[context]);
      assert(rc == PAMI_SUCCESS);
#else
      int result;
      result = pthread_create(&threads[context], NULL, advance, &contexts[context]);
      assert(result == 0);
#endif
    }

    uint64_t posted_time, done_time;
    test(contexts, ncontexts, ITERATIONS>>4, &posted_time, &done_time);
    test(contexts, ncontexts, ITERATIONS,    &posted_time, &done_time);

    uint64_t posted_time_per = posted_time / (ncontexts*ITERATIONS);
    uint64_t done_time_per   = done_time   / (ncontexts*ITERATIONS);
    printf("Posted %u functions on each of %zu contexts in %llu cycles (%llu cyclces per post).  The functions were called in %llu cycles (%llu cyclces per call).\n",
           ITERATIONS,
           ncontexts-1,
           (long long unsigned)posted_time,
           (long long unsigned)posted_time_per,
           (long long unsigned)done_time,
           (long long unsigned)done_time_per);
  }


#ifdef __pami_target_bgq__
  rc = PAMI_Context_destroyv(contexts, ncontexts);
  assert(rc == PAMI_SUCCESS);

  rc = PAMI_Client_destroy(&client);
  assert(rc == PAMI_SUCCESS);
#endif

  return 0;
};
