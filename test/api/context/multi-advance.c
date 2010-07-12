///
/// \file test/api/context/multi-advance.c
/// \brief Simple PAMI_Context_advance() multiple test
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
  result = PAMI_Context_createv (client, configuration, 0, context, num);
  if (result != PAMI_SUCCESS || num != 2)
  {
    fprintf (stderr, "Error. Unable to create both pami context. result = %d\n", result);
    return 1;
  }

  if (result == PAMI_SUCCESS)
  {
    fprintf (stdout, "Before PAMI_Context_advance()\n");
    result = PAMI_Context_advance (context[0], 1);
    fprintf (stdout, " After PAMI_Context_advance(), result = %d\n", result);
  }

  if (result == PAMI_SUCCESS)
  {
    fprintf (stdout, "Before PAMI_Context_advance()\n");
    result = PAMI_Context_advance (context[1], 1);
    fprintf (stdout, " After PAMI_Context_advance(), result = %d\n", result);
  }

  result = PAMI_Context_destroyv (context, num);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy first pami context. result = %d\n", result);
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