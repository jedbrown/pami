/**
 * \file test/time/tick.c
 * \brief Simple XMI_Wtick() test before XMI_Client_initialize()
 */

#include "sys/xmi.h"
#include <stdio.h>

int main (int argc, char ** argv)
{
  xmi_client_t client;
  size_t num=1;
  xmi_context_t context;
  xmi_configuration_t query = {name:XMI_WTICK};
  double value;

  XMI_Client_initialize("XMI Example", &client);
  XMI_Context_createv(client, NULL, 0, &context, &num);

  fprintf (stdout, "Before XMI_Wtick()\n");

  XMI_Configuration_query(client, &query);
  value = query.value.doubleval;

  fprintf (stdout, "After XMI_Wtick, value = %g\n", value);

  return 0;
};
