///
/// \file test/context/post.c
/// \brief Simple XMI_Context_post() test
///

#include "sys/xmi.h"
#include <stdio.h>

volatile unsigned _value[2];

xmi_result_t do_work (xmi_context_t   context,
              void          * cookie)
{
  fprintf (stderr, "do_work() cookie = %p, %d -> %d\n", cookie, *((unsigned *)cookie), *((unsigned *)cookie)-1);
  (*((unsigned *)cookie))--;
  return XMI_SUCCESS;
}

int main (int argc, char ** argv)
{
  xmi_client_t client;
  xmi_context_t context[2];
  xmi_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

  size_t num = 2;
  result = XMI_Context_createv (client, configuration, 0, &context[0], &num);
  if (result != XMI_SUCCESS || num != 2)
  {
    fprintf (stderr, "Error. Unable to create two xmi context. result = %d\n", result);
    return 1;
  }

  _value[0] = 1;
  _value[1] = 1;



  /* Lock both contexts */
  result = XMI_Context_lock (context[0]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to lock the first xmi context. result = %d\n", result);
    return 1;
  }
  result = XMI_Context_lock (context[1]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to lock the second xmi context. result = %d\n", result);
    return 1;
  }



  /* Post some work to the contexts */
  result = XMI_Context_post (context[0], do_work, (void *)&_value[0]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to post work to the first xmi context. result = %d\n", result);
    return 1;
  }
  result = XMI_Context_post (context[1], do_work, (void *)&_value[1]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to post work to the second xmi context. result = %d\n", result);
    return 1;
  }


  while (_value[0] || _value[1])
  {
    result = XMI_Context_advance (context[0], 1);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to advance the first xmi context. result = %d\n", result);
      return 1;
    }
    result = XMI_Context_advance (context[1], 1);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to advance the second xmi context. result = %d\n", result);
      return 1;
    }
  }

  result = XMI_Context_destroy (context[0]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy first xmi context. result = %d\n", result);
    return 1;
  }
  result = XMI_Context_destroy (context[1]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy second xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  fprintf (stderr, "Success.\n");

  return 0;
};
