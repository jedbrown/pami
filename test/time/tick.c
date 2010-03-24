/**
 * \file test/time/tick.c
 * \brief Simple PAMI_Wtick() test before PAMI_Client_initialize()
 */

#include "sys/pami.h"
#include <stdio.h>

int main (int argc, char ** argv)
{
  pami_client_t client;
  size_t num=1;
  pami_context_t context;
  pami_configuration_t query = {name:PAMI_WTICK};
  double value;

  PAMI_Client_initialize("PAMI Example", &client);
  PAMI_Context_createv(client, NULL, 0, &context, num);

  fprintf (stdout, "Before PAMI_Wtick()\n");

  PAMI_Configuration_query(client, &query);
  value = query.value.doubleval;

  fprintf (stdout, "After PAMI_Wtick, value = %g\n", value);

  return 0;
};
