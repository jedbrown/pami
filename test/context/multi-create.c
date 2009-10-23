///
/// \file test/context/multi-create.c
/// \brief Simple XMI_Context_create() test for multiple contexts
///

#include "sys/xmi.h"
#include <stdio.h>


int main (int argc, char ** argv)
{
  xmi_client_t client;
  xmi_context_t context[2];
  xmi_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";

  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_create (client, configuration, 0, &context[0]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create first xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_create (client, configuration, 0, &context[1]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create second xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_destroy (context[0]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy first xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_destroy (context[1]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy second xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  fprintf (stderr, "Success.\n");

  return 0;
};
