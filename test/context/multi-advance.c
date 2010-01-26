///
/// \file test/context/multi-advance.c
/// \brief Simple XMI_Context_advance() multiple test
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

  size_t num = 2;
  result = XMI_Context_createv (client, configuration, 0, &context[0], num);
  if (result != XMI_SUCCESS || num != 2)
  {
    fprintf (stderr, "Error. Unable to create both xmi context. result = %d\n", result);
    return 1;
  }

  if (result == XMI_SUCCESS)
  {
    fprintf (stdout, "Before XMI_Context_advance()\n");
    result = XMI_Context_advance (context[0], 1);
    fprintf (stdout, " After XMI_Context_advance(), result = %d\n", result);

    result = XMI_Context_destroy (context[0]);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to destroy first xmi context. result = %d\n", result);
      return 1;
    }
  }

  if (result == XMI_SUCCESS)
  {
    fprintf (stdout, "Before XMI_Context_advance()\n");
    result = XMI_Context_advance (context[1], 1);
    fprintf (stdout, " After XMI_Context_advance(), result = %d\n", result);

    result = XMI_Context_destroy (context[1]);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to destroy second xmi context. result = %d\n", result);
      return 1;
    }
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
