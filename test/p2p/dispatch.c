///
/// \file test/p2p/dispatch.c
/// \brief Simple point-topoint XMI_Dispatch_set() test
///

#include "sys/xmi.h"
#include <stdio.h>

static void test_dispatch (
    xmi_context_t        context,      /**< IN: XMI context */
    size_t               contextid,
    void               * cookie,       /**< IN: dispatch cookie */
    xmi_task_t           task,         /**< IN: source task */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
{
  fprintf (stderr, "Called dispatch function.\n");

  return;
}

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

	{ int _n = 1; result = XMI_Context_createv(client, configuration, 0, &context, &_n); }
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
    return 1;
  }


  size_t dispatch = 0;
  xmi_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;
  xmi_send_hint_t options={0};
  result = XMI_Dispatch_set (context, dispatch, fn, NULL, options);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register xmi dispatch. result = %d\n", result);
    return 1;
  }


  result = XMI_Context_advance (client, 0, 1);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", result);
    return 1;
  }


  result = XMI_Context_destroy (context);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy xmi context. result = %d\n", result);
    return 1;
  }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
    return 1;
  }

  return 0;
};
