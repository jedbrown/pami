///
/// \file test/api/client/hello.c
/// \brief Simple hello after PAMI_Client_create()
///

#include <pami.h>
#include <stdio.h>


int main (int argc, char ** argv)
{
  pami_client_t client;
  pami_result_t result = PAMI_ERROR;
  char         cl_string[] = "TEST";

  fprintf (stdout, "Before PAMI_Client_create()\n");
  result = PAMI_Client_create (cl_string, &client, NULL, 0);
  fprintf (stdout, "After PAMI_Client_create(), result = %d\n", result);

  fprintf (stdout, "Before PAMI_Client_destroy()\n");
  result = PAMI_Client_destroy(&client);
  fprintf (stdout, "After PAMI_Client_destroy(), result = %d\n", result);

  return 0;
};
