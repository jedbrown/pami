/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/init_util.h
 * \brief ???
 */

#ifndef __test_api_init_util_h__
#define __test_api_init_util_h__

#include <pami.h>
#include <stdio.h>
enum opNum
{
  OP_COPY,
  OP_NOOP,
  OP_MAX,
  OP_MIN,
  OP_SUM,
  OP_PROD,
  OP_LAND,
  OP_LOR,
  OP_LXOR,
  OP_BAND,
  OP_BOR,
  OP_BXOR,
  OP_MAXLOC,
  OP_MINLOC,
  OP_COUNT,
};
static const int    op_count = OP_COUNT;
pami_data_function  op_array[OP_COUNT];
const char         *op_array_str[OP_COUNT];

enum dtNum
{
  DT_NULL,
  DT_BYTE,
  DT_SIGNED_CHAR,
  DT_UNSIGNED_CHAR,
  DT_SIGNED_SHORT,
  DT_UNSIGNED_SHORT,
  DT_SIGNED_INT,
  DT_UNSIGNED_INT,
  DT_SIGNED_LONG,
  DT_UNSIGNED_LONG,
  DT_SIGNED_LONG_LONG,
  DT_UNSIGNED_LONG_LONG,
  DT_FLOAT,
  DT_DOUBLE,
  DT_LONG_DOUBLE,
  DT_LOGICAL,
  DT_SINGLE_COMPLEX,
  DT_DOUBLE_COMPLEX,
  DT_LOC_2INT,
  DT_LOC_2FLOAT,
  DT_LOC_2DOUBLE,
  DT_LOC_SHORT_INT,
  DT_LOC_FLOAT_INT,
  DT_LOC_DOUBLE_INT,
  DT_COUNT,
};
int          dt_count = DT_COUNT;
pami_type_t  dt_array[DT_COUNT];
const char * dt_array_str[DT_COUNT];

size_t ** alloc2DContig(int nrows, int ncols)
{
  int i;
  size_t **array;

  array        = (size_t**)malloc(nrows * sizeof(size_t*));
  array[0]     = (size_t *)calloc(sizeof(size_t), nrows * ncols);

  for (i = 1; i < nrows; i++)
    array[i]   = array[0] + i * ncols;

  return array;
}


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
  result = PAMI_Client_create (clientname, client, NULL, 0);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize pami client %s: result = %d\n",
                 clientname,result);
        return 1;
      }
  /* Docs02:  Create the client */

  /* Docs03:  Create the client */
  l_configuration.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query(*client, &l_configuration,1);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                 l_configuration.name, result);
        return 1;
      }
  max_contexts = l_configuration.value.intval;
  *num_tasks = (*num_tasks<max_contexts)?*num_tasks:max_contexts;

  l_configuration.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query(*client, &l_configuration,1);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                 l_configuration.name, result);
        return 1;
      }
  *task_id = l_configuration.value.intval;

  l_configuration.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query(*client, &l_configuration,1);
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

  op_array[0]=PAMI_DATA_COPY;
  op_array[1]=PAMI_DATA_NOOP;
  op_array[2]=PAMI_DATA_MAX;
  op_array[3]=PAMI_DATA_MIN;
  op_array[4]=PAMI_DATA_SUM;
  op_array[5]=PAMI_DATA_PROD;
  op_array[6]=PAMI_DATA_LAND;
  op_array[7]=PAMI_DATA_LOR;
  op_array[8]=PAMI_DATA_LXOR;
  op_array[9]=PAMI_DATA_BAND;
  op_array[10]=PAMI_DATA_BOR;
  op_array[11]=PAMI_DATA_BXOR;
  op_array[12]=PAMI_DATA_MAXLOC;
  op_array[13]=PAMI_DATA_MINLOC;

  op_array_str[0]="PAMI_DATA_COPY";
  op_array_str[1]="PAMI_DATA_NOOP";
  op_array_str[2]="PAMI_DATA_MAX";
  op_array_str[3]="PAMI_DATA_MIN";
  op_array_str[4]="PAMI_DATA_SUM";
  op_array_str[5]="PAMI_DATA_PROD";
  op_array_str[6]="PAMI_DATA_LAND";
  op_array_str[7]="PAMI_DATA_LOR";
  op_array_str[8]="PAMI_DATA_LXOR";
  op_array_str[9]="PAMI_DATA_BAND";
  op_array_str[10]="PAMI_DATA_BOR";
  op_array_str[11]="PAMI_DATA_BXOR";
  op_array_str[12]="PAMI_DATA_MAXLOC";
  op_array_str[13]="PAMI_DATA_MINLOC";

  dt_array[0]=PAMI_TYPE_NULL;
  dt_array[1]=PAMI_TYPE_BYTE;
  dt_array[2]=PAMI_TYPE_SIGNED_CHAR;
  dt_array[3]=PAMI_TYPE_UNSIGNED_CHAR;
  dt_array[4]=PAMI_TYPE_SIGNED_SHORT;
  dt_array[5]=PAMI_TYPE_UNSIGNED_SHORT;
  dt_array[6]=PAMI_TYPE_SIGNED_INT;
  dt_array[7]=PAMI_TYPE_UNSIGNED_INT;
  dt_array[8]=PAMI_TYPE_SIGNED_LONG;
  dt_array[9]=PAMI_TYPE_UNSIGNED_LONG;
  dt_array[10]=PAMI_TYPE_SIGNED_LONG_LONG;
  dt_array[11]=PAMI_TYPE_UNSIGNED_LONG_LONG;
  dt_array[12]=PAMI_TYPE_FLOAT;
  dt_array[13]=PAMI_TYPE_DOUBLE;
  dt_array[14]=PAMI_TYPE_LONG_DOUBLE;
  dt_array[15]=PAMI_TYPE_LOGICAL;
  dt_array[16]=PAMI_TYPE_SINGLE_COMPLEX;
  dt_array[17]=PAMI_TYPE_DOUBLE_COMPLEX;
  dt_array[18]=PAMI_TYPE_LOC_2INT;
  dt_array[19]=PAMI_TYPE_LOC_2FLOAT;
  dt_array[20]=PAMI_TYPE_LOC_2DOUBLE;
  dt_array[21]=PAMI_TYPE_LOC_SHORT_INT;
  dt_array[22]=PAMI_TYPE_LOC_FLOAT_INT;
  dt_array[23]=PAMI_TYPE_LOC_DOUBLE_INT;

  dt_array_str[0]="PAMI_TYPE_NULL";
  dt_array_str[1]="PAMI_TYPE_BYTE";
  dt_array_str[2]="PAMI_TYPE_SIGNED_CHAR";
  dt_array_str[3]="PAMI_TYPE_UNSIGNED_CHAR";
  dt_array_str[4]="PAMI_TYPE_SIGNED_SHORT";
  dt_array_str[5]="PAMI_TYPE_UNSIGNED_SHORT";
  dt_array_str[6]="PAMI_TYPE_SIGNED_INT";
  dt_array_str[7]="PAMI_TYPE_UNSIGNED_INT";
  dt_array_str[8]="PAMI_TYPE_SIGNED_LONG";
  dt_array_str[9]="PAMI_TYPE_UNSIGNED_LONG";
  dt_array_str[10]="PAMI_TYPE_SIGNED_LONG_LONG";
  dt_array_str[11]="PAMI_TYPE_UNSIGNED_LONG_LONG";
  dt_array_str[12]="PAMI_TYPE_FLOAT";
  dt_array_str[13]="PAMI_TYPE_DOUBLE";
  dt_array_str[14]="PAMI_TYPE_LONG_DOUBLE";
  dt_array_str[15]="PAMI_TYPE_LOGICAL";
  dt_array_str[16]="PAMI_TYPE_SINGLE_COMPLEX";
  dt_array_str[17]="PAMI_TYPE_DOUBLE_COMPLEX";
  dt_array_str[18]="PAMI_TYPE_LOC_2INT";
  dt_array_str[19]="PAMI_TYPE_LOC_2FLOAT";
  dt_array_str[20]="PAMI_TYPE_LOC_2DOUBLE";
  dt_array_str[21]="PAMI_TYPE_LOC_SHORT_INT";
  dt_array_str[22]="PAMI_TYPE_LOC_FLOAT_INT";
  dt_array_str[23]="PAMI_TYPE_LOC_DOUBLE_INT";
  /* Docs06:  Create the client */
  return 0;
}

int pami_shutdown(pami_client_t        * client,          /* in/out:  client      */
                  pami_context_t       * context,         /* in/out:  context     */
                  size_t               * num_contexts)    /* in/out:  num_contexts*/
{
  pami_result_t result;
  /* Docs07:  Destroy the client and contexts */
  result = PAMI_Context_destroyv(context, *num_contexts);
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
