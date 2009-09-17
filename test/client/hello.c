///
/// \file test/client/hello.c
/// \brief Simple hello after XMI_Client_initialize()
///

#include "sys/xmi.h"



int main (int argc, char ** argv)
{
  xmi_client_t client;
  xmi_result_t result = XMI_ERROR;
  char         cl_string[] = "TEST";

  fprintf (stdout, "Before XMI_Client_initialize()\n");
  result = XMI_Client_initialize (cl_string, &client);
  fprintf (stdout, "After XMI_Client_initialize(), result = %d\n", result);

  fprintf (stdout, "Before XMI_Client_finalize()\n");
  result = XMI_Client_finalize (client);
  fprintf (stdout, "After XMI_Client_finalize(), result = %d\n", result);

  return 0;
};
