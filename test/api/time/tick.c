/**
 * \file test/api/time/tick.c
 * \brief Simple PAMI_Wtick() test before PAMI_Client_create()
 */

#include <pami.h>
#include <stdio.h>

int main (int argc, char ** argv)
{
  pami_client_t client;
  size_t num=1;
  pami_context_t context;
  pami_configuration_t query = {name:PAMI_CLIENT_WTICK};
  double value;

  PAMI_Client_create("PAMI Example", &client, NULL, 0);
  PAMI_Context_createv(client, NULL, 0, &context, num);

  fprintf (stdout, "Before PAMI_Wtick()\n");

  PAMI_Client_query(client, &query,1);
  value = query.value.doubleval;

  fprintf (stdout, "After PAMI_Wtick, value = %g\n", value);

  return 0;
};
