///
/// \file test/context/lock.c
/// \brief Simple XMI_Context_lock() test
///

#include "sys/xmi.h"
#include <stdio.h>

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

	{ size_t _n = 1; result = XMI_Context_createv(client, configuration, 0, &context, _n); }
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create the xmi context. result = %d\n", result);
    return 1;
  }


  /* Test a context lock */
  result = XMI_Context_lock (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to lock the xmi context. result = %d\n", result);
    return 1;
  }

  /* Test a context unlock */
  result = XMI_Context_unlock (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to unlock the xmi context. result = %d\n", result);
    return 1;
  }

  /* Test a context trylock */
  result = XMI_Context_trylock (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to aquire a context lock via trylock() on an uncontested lock. result = %d\n", result);
    return 1;
  }

  /* Test a context trylock .. should return XMI_EAGAIN. */
  result = XMI_Context_trylock (context);
  if (result == XMI_SUCCESS)
  {
    fprintf (stderr, "Error. trylock was 'successful' when it should have failed. result = %d\n", result);
    return 1;
  }
  if (result != XMI_EAGAIN)
  {
    fprintf (stderr, "Error. trylock did not return XMI_EAGAIN. result = %d\n", result);
    return 1;
  }

  /* Test a context unlock */
  result = XMI_Context_unlock (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to unlock the xmi context. result = %d\n", result);
    return 1;
  }



  /* ----------------------------------------------------------------------- */
  /* Need multiple threads to test XMI_Context_lock () when the lock is      */
  /* already aqcuired. XMI_Context_lock () will block until it acquires the  */
  /* lock.                                                                   */
  /* ----------------------------------------------------------------------- */



  /* Destroy the context */
  result = XMI_Context_destroy (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy the xmi context. result = %d\n", result);
    return 1;
  }

  /* Finalize (destroy) the client */
  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  fprintf (stderr, "Success.\n");

  return 0;
};
