/**
 * \file test/api/client/multi-client.c
 * \brief Simple hello after PAMI_Client_create()
 */

#include <pami.h>
#include <stdio.h>

#define MAX_CLIENTS 2

int main (int argc, char ** argv)
{
  pami_client_t client[MAX_CLIENTS];
  pami_result_t result = PAMI_ERROR;
  char         cl_string[] = "TEST";
  
  int max_clients=MAX_CLIENTS,i;
  for(i=0;i<max_clients;i++)
  {
    fprintf (stdout, "Creating Client %d of %d\n", i, max_clients);
    result = PAMI_Client_create (cl_string, &client[i], NULL, 0);
    if(result!=PAMI_SUCCESS)
      fprintf(stderr, "--->error creating client %d\n", i);
  }
  for(i=0;i<max_clients;i++)
  {
    fprintf (stdout, "Destroying Client %d of %d\n", i, max_clients);
    result = PAMI_Client_destroy(&client[i]);
    if(result!=PAMI_SUCCESS)
      fprintf(stderr, "--->error destroying client %d\n", i);
  }
  return 0;
};
