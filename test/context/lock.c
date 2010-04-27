///
/// \file test/context/lock.c
/// \brief Simple PAMI_Context_lock() test
///

#include "sys/pami.h"
#include <stdio.h>

int main (int argc, char ** argv)
{
  pami_client_t client;
  pami_context_t context;
  pami_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

        { size_t _n = 1; result = PAMI_Context_createv(client, configuration, 0, &context, _n); }
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create the pami context. result = %d\n", result);
    return 1;
  }


  /* Test a context lock */
  result = PAMI_Context_lock (context);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to lock the pami context. result = %d\n", result);
    return 1;
  }

  /* Test a context unlock */
  result = PAMI_Context_unlock (context);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to unlock the pami context. result = %d\n", result);
    return 1;
  }

  /* Test a context trylock */
  result = PAMI_Context_trylock (context);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to aquire a context lock via trylock() on an uncontested lock. result = %d\n", result);
    return 1;
  }

  /* Test a context trylock .. should return PAMI_EAGAIN. */
  result = PAMI_Context_trylock (context);
  if (result == PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. trylock was 'successful' when it should have failed. result = %d\n", result);
    return 1;
  }
  if (result != PAMI_EAGAIN)
  {
    fprintf (stderr, "Error. trylock did not return PAMI_EAGAIN. result = %d\n", result);
    return 1;
  }

  /* Test a context unlock */
  result = PAMI_Context_unlock (context);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to unlock the pami context. result = %d\n", result);
    return 1;
  }



  /* ----------------------------------------------------------------------- */
  /* Need multiple threads to test PAMI_Context_lock () when the lock is      */
  /* already aqcuired. PAMI_Context_lock () will block until it acquires the  */
  /* lock.                                                                   */
  /* ----------------------------------------------------------------------- */



  /* Destroy the context */
  result = PAMI_Context_destroy (context);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy the pami context. result = %d\n", result);
    return 1;
  }

  /* Finalize (destroy) the client */
  result = PAMI_Client_destroy (client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  fprintf (stderr, "Success.\n");

  return 0;
};
