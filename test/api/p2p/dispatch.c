/**
 * \file test/api/p2p/dispatch.c
 * \brief Simple point-topoint PAMI_Dispatch_set() test
 */

#include <pami.h>
#include <stdio.h>

static void test_dispatch (
    pami_context_t        context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_endpoint_t origin,
pami_recv_t         * recv)        /**< OUT: receive message structure */
{
  fprintf (stderr, "Called dispatch function.\n");

  return;
}

int main (int argc, char ** argv)
{
  pami_client_t client;
  pami_context_t context;
  pami_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client, NULL, 0);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

        {  result = PAMI_Context_createv(client, configuration, 0, &context, 1); }
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
    return 1;
  }


  size_t dispatch = 0;
  pami_dispatch_callback_function fn;
  fn.p2p = test_dispatch;
  pami_dispatch_hint_t options={};
  result = PAMI_Dispatch_set (context, dispatch, fn, NULL, options);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }


  result = PAMI_Context_advance (context, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
    return 1;
  }


  result = PAMI_Context_destroyv(&context, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy(&client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  return 0;
};
