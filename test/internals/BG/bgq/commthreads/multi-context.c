///
/// \file test/internals/BG/bgq/commthreads/multi-context.c
/// \brief Simple test for basic commthread functionality
///

#include "commthread_test.h"

int main(int argc, char ** argv) {
  pami_client_t client;
  pami_configuration_t *configuration = NULL;
  char cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;
  pami_context_t context[NUM_CONTEXTS];
  int x, y;
  char buf[64];
  sprintf(buf, "St %ld\n", pthread_self());
  int bufl = strlen(buf);

  result = PAMI_Client_create(cl_string, &client);
  if (result != PAMI_SUCCESS) {
    fprintf(stderr, "Error. Unable to initialize pami client. "
            "result = %d\n", result);
    return 1;
  }

  result = PAMI_Context_createv(client, configuration, 0, &context[0], NUM_CONTEXTS);
  if (result != PAMI_SUCCESS) {
    fprintf(stderr, "Error. Unable to create %d pami context. "
            "result = %d\n", NUM_CONTEXTS, result);
    return 1;
  }
  write(2, buf, bufl);
  for (x = 0; x < NUM_CONTEXTS; ++x) {
    result = PAMI_Client_add_commthread_context(client, context[x]);
    if (result != PAMI_SUCCESS) {
      fprintf(stderr, "Error. Unable to add commthread to context[%d]. "
              "result = %d (%d)\n", x, result, errno);
      return 1;
    }
  }
  for (y = 0; y < NUM_TESTRUNS; ++y) {
    for (x = 0; x < NUM_CONTEXTS; ++x) {
      _info[x].seq = y * NUM_CONTEXTS + x + 1;
    }

    result = run_test(client, context, NUM_CONTEXTS);
    if (result != PAMI_SUCCESS) {
      fprintf(stderr, "Error. Unable to run commthread test. "
              "result = %d\n", result);
      return 1;
    }

    if (y + 1 < NUM_TESTRUNS) {
      buf[0] = 'S'; buf[1] = 'p';
      write(2, buf, bufl);
      fprintf(stderr, "Sleeping...\n");
      /* sleep(5); */
      unsigned long long t0 = PAMI_Wtimebase();
      //fprintf(stderr, "Woke up after %lld cycles\n", (PAMI_Wtimebase() - t0));
      while (PAMI_Wtimebase() - t0 < 500000);
      buf[0] = 'W'; buf[1] = 'a';
      write(2, buf, bufl);
    }
  }

  buf[0] = 'S'; buf[1] = 'p';
  write(2, buf, bufl);
  {unsigned long long t0 = PAMI_Wtimebase();
    while (PAMI_Wtimebase() - t0 < 500000);}
  buf[0] = 'F'; buf[1] = 'i';
  write(2, buf, bufl);
  result = PAMI_Context_destroyv(context, NUM_CONTEXTS);
  result = PAMI_Client_destroy(&client);
  if (result != PAMI_SUCCESS) {
    fprintf(stderr, "Error. Unable to finalize pami client. "
            "result = %d\n", result);
    return 1;
  }

  buf[0] = 'S'; buf[1] = 'u';
  write(2, buf, bufl);
  fprintf(stderr, "Success.\n");

  return 0;
}
