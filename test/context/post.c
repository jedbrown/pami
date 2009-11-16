///
/// \file test/context/post.c
/// \brief Simple XMI_Context_post() test
///

#include "sys/xmi.h"
#include <stdio.h>

volatile unsigned _value[2];

void do_work (xmi_client_t client, size_t   context,
              void          * cookie,
              xmi_result_t    result)
{
  fprintf (stderr, "do_work() cookie = %p, %d -> %d\n", cookie, *((unsigned *)cookie), *((unsigned *)cookie)-1);
  (*((unsigned *)cookie))--;
  return;
}

int main (int argc, char ** argv)
{
  xmi_client_t client;
  xmi_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;

  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
    return 1;
  }

  result = XMI_Context_create(client, configuration, 0, 2);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create two xmi context. result = %d\n", result);
    return 1;
  }

  _value[0] = 1;
  _value[1] = 1;



  /* Lock both contexts */
  result = XMI_Context_lock (client, 0);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to lock the first xmi context. result = %d\n", result);
    return 1;
  }
  result = XMI_Context_lock (client, 1);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to lock the second xmi context. result = %d\n", result);
    return 1;
  }



  /* Post some work to the contexts */
  result = XMI_Context_post (client, 0, do_work, (void *)&_value[0]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to post work to the first xmi context. result = %d\n", result);
    return 1;
  }
  result = XMI_Context_post (client, 1, do_work, (void *)&_value[1]);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to post work to the second xmi context. result = %d\n", result);
    return 1;
  }


  while (_value[0] || _value[1])
  {
    result = XMI_Context_advance (client, 0, 1);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to advance the first xmi context. result = %d\n", result);
      return 1;
    }
    result = XMI_Context_advance (client, 1, 1);
    if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to advance the second xmi context. result = %d\n", result);
      return 1;
    }
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
