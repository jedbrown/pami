///
/// \file test/context/create.c
/// \brief Simple XMI_Context_create() test
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

  result = XMI_Context_createv(client, configuration, 0, 1);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  return 0;
};
