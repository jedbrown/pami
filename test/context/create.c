///
/// \file test/context/create.c
/// \brief Simple PAMI_Context_createv() test
///

#include <pami.h>
#include <stdio.h>


int main (int argc, char ** argv)
{
  pami_client_t client;
  pami_context_t context;
  pami_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";

  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

        { size_t _n = 1; result = PAMI_Context_createv(client, configuration, 0, &context, _n); }
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Context_destroy (context);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy (client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  return 0;
};
