///
/// \file test/context/multi-create.c
/// \brief Simple PAMI_Context_createv() test for multiple contexts
///

#include <pami.h>
#include <stdio.h>


int main (int argc, char ** argv)
{
  pami_client_t client;
  pami_context_t context[2];
  pami_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";

  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

  size_t num = 2;
  result = PAMI_Context_createv (client, configuration, 0, &context[0], num);
  if (result != PAMI_SUCCESS || num != 2)
  {
    fprintf (stderr, "Error. Unable to create two pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Context_destroy (context[0]);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy first pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Context_destroy (context[1]);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy second pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy (client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  fprintf (stderr, "Success.\n");

  return 0;
};
