/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/init.h
 * \brief ???
 */

#ifndef __test_api_init_util_h__
#define __test_api_init_util_h__

#include <pami.h>
int pami_init(pami_client_t        * client,          /* in/out:  client      */
              pami_context_t       * context,         /* in/out:  context     */
              char                 * clientname,      /* in/out:  clientname  */
              size_t               * num_contexts,    /* in/out:  num_contexts*/
              pami_configuration_t * configuration,   /* in:      config      */
              size_t                 num_config,      /* in:      num configs */
              pami_task_t          * task_id,         /* out:     task id     */
              size_t               * num_tasks)       /* out:     num tasks   */
{
  pami_result_t        result        = PAMI_ERROR;
  char                 cl_string[]   = "TEST";
  pami_configuration_t l_configuration;
  size_t               max_contexts;
  
  if(clientname == NULL)
    clientname = cl_string;

  /* Docs01:  Create the client */
  result = PAMI_Client_create (clientname, client);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize pami client %s: result = %d\n",
                 clientname,result);
        return 1;
      }
  /* Docs02:  Create the client */

  /* Docs03:  Create the client */
  l_configuration.name = PAMI_NUM_CONTEXTS;
  result = PAMI_Configuration_query(client, &l_configuration);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                 l_configuration.name, result);
        return 1;
      }
  max_contexts = l_configuration.value.intval;
  *num_tasks = MIN(*num_tasks,max_contexts);

  l_configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &l_configuration);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                 l_configuration.name, result);
        return 1;
      }
  *task_id = l_configuration.value.intval;

  l_configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &l_configuration);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                 l_configuration.name, result);
        return 1;
      }
  *num_tasks = l_configuration.value.intval;
  /* Docs04:  Create the client */
  
  /* Docs05:  Create the client */
  result = PAMI_Context_createv(*client, configuration, num_config, context, *num_contexts);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to create pami context: result = %d\n",
                 result);
        return 1;
      }
  /* Docs06:  Create the client */
  return 0;
}

int pami_shutdown(pami_client_t        * client,          /* in/out:  client      */
                  pami_context_t       * context,         /* in/out:  context     */
                  size_t               * num_contexts)    /* in/out:  num_contexts*/
{
  pami_result_t result;
  /* Docs07:  Destroy the client and contexts */
  result = PAMI_Context_destroyv(context, 1);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
      return 1;
    }

  result = PAMI_Client_destroy(client);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
      return 1;
    }
  /* Docs08:  Destroy the client and contexts*/
  return 0;
}

#endif /* __test_api_init_h__ */

