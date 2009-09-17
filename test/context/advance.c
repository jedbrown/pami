///
/// \file test/context/advance.c
/// \brief Simple XMI_Context_advance() test
///

#include "sys/xmi.h"



int main (int argc, char ** argv)
{
  xmi_client_t client;
  xmi_context_t context;
  xmi_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_create (client, configuration, 0, &context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
    return 1;
  }

  if (result == XMI_SUCCESS)
  {
    fprintf (stdout, "Before XMI_Context_advance()\n");
    result = XMI_Context_advance (context, 1);
    fprintf (stdout, " After XMI_Context_advance(), result = %d\n", result);

    result = XMI_Context_destroy (context);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to destroy xmi context. result = %d\n", result);
      return 1;
    }
  }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  return 0;
};
