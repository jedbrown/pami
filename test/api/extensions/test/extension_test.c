/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/extensions/test/extension_test.c
 * \brief Simple test of the PAMI "test" extension
 */

#include <stdio.h>
#include <pami.h>

typedef struct
{
  size_t line;
  char   func[128];
} bar_info_t;

typedef void (*pami_extension_test_foo_fn) ();
typedef void (*pami_extension_test_bar_fn) (bar_info_t *);

#define DBG_FPRINTF(x) //fprintf x

int main(int argc, char ** argv)
{
  pami_client_t client;
  pami_context_t context;
  pami_result_t status = PAMI_ERROR;

  status = PAMI_Client_create("extension", &client, NULL, 0);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", status);
    return 1;
  }
  DBG_FPRINTF((stderr,"Client %p\n",client));

  status = PAMI_Context_createv(client, NULL, 0, &context, 1);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", status);
    return 1;
  }

  /* ------------------------------------------------------------------------ */


  pami_extension_t extension;
  status = PAMI_Extension_open (client, "EXT_test_extension", &extension);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. The \"EXT_test_extension\" extension is not implemented. result = %d\n", status);
    return 1;
  }

  pami_extension_test_foo_fn pamix_test_foo =
    (pami_extension_test_foo_fn) PAMI_Extension_function (extension, "foo");
  if (pamix_test_foo == (void *)NULL)
  {
    fprintf (stderr, "Error. The \"EXT_test_extension\" extension function \"foo\" is not implemented. result = %d\n", status);
    return 1;
  }

  pamix_test_foo ();

  pami_extension_test_bar_fn pamix_test_bar =
    (pami_extension_test_bar_fn) PAMI_Extension_function (extension, "bar");
  if (pamix_test_bar == (void *)NULL)
  {
    fprintf (stderr, "Error. The \"EXT_test_extension\" extension function \"bar\" is not implemented. result = %d\n", status);
    return 1;
  }

  bar_info_t info;
  pamix_test_bar (&info);
  printf ("line: %zu, function: %s\n", info.line, info.func);

  status = PAMI_Extension_close (extension);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. The \"EXT_test_extension\" extension could not be closed. result = %d\n", status);
    return 1;
  }


  /* ------------------------------------------------------------------------ */
  DBG_FPRINTF((stderr, "PAMI_Context_destroyv(&context, 1);\n"));
  status = PAMI_Context_destroyv(&context, 1);
  if(status != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy pami context. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "PAMI_Client_destroy(&client);\n"));
  status = PAMI_Client_destroy(&client);
  if(status != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "return 0;\n"));
  return 0;
}
