/**
 * \file test/api/client/configuration.c
 * \brief Simple client configuration test
 */

#include <pami.h>
#include <stdio.h>


int main (int argc, char ** argv)
{
  pami_client_t client;
  pami_result_t result = PAMI_ERROR;
  char         cl_string[] = "TEST";

  result = PAMI_Client_create (cl_string, &client, NULL, 0);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to create client '%s'; result = %d\n", cl_string, result);
    return 1;
  }

  pami_configuration_t configuration;

  configuration.name = PAMI_CLIENT_CLOCK_MHZ;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_CLOCK_MHZ; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_CLOCK_MHZ = %zu\n", configuration.value.intval);

  configuration.name = PAMI_CLIENT_CONST_CONTEXTS;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_CONST_CONTEXTS; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_CONST_CONTEXTS = %zu\n", configuration.value.intval);

  configuration.name = PAMI_CLIENT_HWTHREADS_AVAILABLE;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_HWTHREADS_AVAILABLE; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_HWTHREADS_AVAILABLE = %zu\n", configuration.value.intval);

  configuration.name = PAMI_CLIENT_MEMREGION_SIZE;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_MEMREGION_SIZE; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_MEMREGION_SIZE = %zu\n", configuration.value.intval);

  configuration.name = PAMI_CLIENT_MEM_SIZE;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_MEM_SIZE; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_MEM_SIZE = %zu\n", configuration.value.intval);

  configuration.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_NUM_TASKS; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_NUM_TASKS = %zu\n", configuration.value.intval);

  configuration.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_NUM_CONTEXTS; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_NUM_CONTEXTS = %zu\n", configuration.value.intval);

  configuration.name = PAMI_CLIENT_PROCESSOR_NAME;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_PROCESSOR_NAME; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_PROCESSOR_NAME = %s\n", configuration.value.chararray);

  configuration.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_TASK_ID; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_TASK_ID = %zu\n", configuration.value.intval);

  configuration.name = PAMI_CLIENT_WTIMEBASE_MHZ;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_WTIMEBASE_MHZ; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_WTIMEBASE_MHZ = %zu\n", configuration.value.intval);

  configuration.name = PAMI_CLIENT_WTICK;
  result = PAMI_Client_query (client, &configuration, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to query configuration attribute PAMI_CLIENT_WTICK; result = %d\n", result);
    /*return 1; */
  }
  fprintf (stdout, "PAMI_CLIENT_WTICK = %f\n", configuration.value.doubleval);


  result = PAMI_Client_destroy(&client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Unable to destroy client '%s'; result = %d\n", cl_string, result);
    exit (1);
  }

  return 0;
};
