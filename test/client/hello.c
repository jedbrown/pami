///
/// \file test/client/hello.c
/// \brief Simple hello after PAMI_Client_initialize()
///

#include "sys/pami.h"
#include <stdio.h>


int main (int argc, char ** argv)
{
  pami_client_t client;
  pami_result_t result = PAMI_ERROR;
  char         cl_string[] = "TEST";

  fprintf (stdout, "Before PAMI_Client_initialize()\n");
  result = PAMI_Client_initialize (cl_string, &client);
  fprintf (stdout, "After PAMI_Client_initialize(), result = %d\n", result);

  fprintf (stdout, "Before PAMI_Client_finalize()\n");
  result = PAMI_Client_finalize (client);
  fprintf (stdout, "After PAMI_Client_finalize(), result = %d\n", result);

  return 0;
};
