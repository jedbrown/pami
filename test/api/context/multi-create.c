/**
 * \file test/api/context/multi-create.c
 * \brief Simple PAMI_Context_createv() test for multiple contexts
 */

#include <pami.h>
#include <stdio.h>


int main (int argc, char ** argv)
{
  pami_client_t client;
  pami_context_t context[2];
  pami_configuration_t configuration;
  char                  cl_string[] = "TEST";

  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client, NULL, 0);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

  configuration.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query(client, &configuration, 1);
  size_t max = configuration.value.intval;

  size_t num = max;
  if (getenv("PAMI_DEVICE")) {
	fprintf(stderr, "PAMI_DEVICE = %s\n", getenv("PAMI_DEVICE"));
  } else {
	fprintf(stderr, "PAMI_DEVICE = D\n");
  }
  if (getenv("NUM_CONTEXTS")) {
	num = strtoul(getenv("NUM_CONTEXTS"), NULL, 0);
	// if (num > max) num = max;
  }
  fprintf (stderr, "PAMI_CLIENT_NUM_CONTEXTS = %zu, using %zu\n", max, num);
  size_t tmp = num;

  result = PAMI_Context_createv (client, &configuration, 0, context, num);
  if (result != PAMI_SUCCESS || num != tmp)
  {
    fprintf (stderr, "Error. Unable to create %zu pami context(s). result = %d\n", tmp, result);
    return 1;
  }

  result = PAMI_Context_destroyv (context, num);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy the pami context(s). result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy(&client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  fprintf (stderr, "Success.\n");

  return 0;
};
