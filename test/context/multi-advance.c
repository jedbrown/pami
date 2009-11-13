///
/// \file test/context/multi-advance.c
/// \brief Simple XMI_Context_advance() multiple test
///

#include "sys/xmi.h"
#include <stdio.h>


int main (int argc, char ** argv)
{
  xmi_client_t client;
  xmi_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_createv (client, configuration, 0, 2);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create both xmi context. result = %d\n", result);
    return 1;
  }

  if (result == XMI_SUCCESS)
  {
    fprintf (stdout, "Before XMI_Context_advance()\n");
    result = XMI_Context_advance (client, 0, 1);
    fprintf (stdout, " After XMI_Context_advance(), result = %d\n", result);

  }

  if (result == XMI_SUCCESS)
  {
    fprintf (stdout, "Before XMI_Context_advance()\n");
    result = XMI_Context_advance (client, 1, 1);
    fprintf (stdout, " After XMI_Context_advance(), result = %d\n", result);

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
