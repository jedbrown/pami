/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/coll_util.h
 * \brief ???
 */

#ifndef __test_api_coll_util_h__
#define __test_api_coll_util_h__

/* Define to assert on failures */
/* #define PAMI_TEST_STRICT     */

#include<assert.h>
#include "init_util.h"

char *gProtocolName = (char*)""; /* Global protocol name, some tests set it for error msgs   */
static size_t get_type_size(pami_type_t intype);
pami_context_t      gContext=NULL;

/* Docs09:  Done/Decrement call */
void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  if(gVerbose)
  {
    if(!ctxt) fprintf(stderr, "%s: Error. Null context received on cb_done.\n",gProtocolName);
    if(gContext != ctxt) fprintf(stderr, "%s: Error. Unexpected context received on cb_done %p != %p.\n",gProtocolName,gContext,ctxt);
#ifdef PAMI_TEST_STRICT
    assert(context);
    assert(gContext==ctxt);
#endif
    pami_configuration_t configs;
    configs.name         = PAMI_CONTEXT_DISPATCH_ID_MAX;
    configs.value.intval = -1;

    pami_result_t rc;
    rc = PAMI_Context_query (ctxt,&configs,1);

    if(rc != PAMI_SUCCESS && rc != PAMI_INVAL) fprintf(stderr,"%s: Error. Could not query the context(%u).\n",gProtocolName,rc);
#ifdef PAMI_TEST_STRICT
    assert(rc == PAMI_SUCCESS || rc == PAMI_INVAL);
#endif
  }

  int * active = (int *) clientdata;
  (*active)--;
}
/* Docs10:  Done/Decrement call */

/* Docs07:  Blocking Collective Call */
int blocking_coll (pami_context_t      context,
                   pami_xfer_t        *coll,
                   volatile unsigned  *active)
{
  pami_result_t result;
  gContext = context;
  (*active)++;
  result = PAMI_Collective(context, coll);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue  collective. result = %d\n", result);
      gContext = NULL;
      return 1;
    }
  while (*active)
    result = PAMI_Context_advance (context, 1);
  gContext = NULL;
  return 0;
}
/* Docs08:  Blocking Collective Call */

int query_geometry_world(pami_client_t       client,
                         pami_context_t      context,
                         pami_geometry_t    *world_geometry,
                         pami_xfer_type_t    xfer_type,
                         size_t             *num_algorithm,
                         pami_algorithm_t  **always_works_alg,
                         pami_metadata_t   **always_works_md,
                         pami_algorithm_t  **must_query_alg,
                         pami_metadata_t   **must_query_md)
{

  pami_result_t     result           = PAMI_SUCCESS;
  /* Docs01:  Get the World Geometry */
  result = PAMI_Geometry_world (client,world_geometry);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get world geometry: result = %d\n", result);
      return 1;
    }
  /* Docs02:  Get the World Geometry */

  /* Docs03:  Query the number of algorithms */
  result = PAMI_Geometry_algorithms_num(*world_geometry,
                                        xfer_type,
                                        num_algorithm);

  if (result != PAMI_SUCCESS || num_algorithm[0]==0)
    {
      fprintf (stderr,
               "Error. Unable to query algorithm, or no algorithms available result = %d\n",
               result);
      return 1;
    }
  /* Docs04:  Query the number of algorithms */

  *always_works_alg = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[0]);
  *always_works_md  = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[0]);
  *must_query_alg   = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[1]);
  *must_query_md    = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[1]);

  /* Docs05:  Query the algorithm lists */
  result = PAMI_Geometry_algorithms_query(*world_geometry,
                                          xfer_type,
                                          *always_works_alg,
                                          *always_works_md,
                                          num_algorithm[0],
                                          *must_query_alg,
                                          *must_query_md,
                                          num_algorithm[1]);
  /* Docs06:  Query the algorithm lists */
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get query algorithm. result = %d\n", result);
      return 1;
    }
  return 0;
}

int query_geometry(pami_client_t       client,
                   pami_context_t      context,
                   pami_geometry_t     geometry,
                   pami_xfer_type_t    xfer_type,
                   size_t             *num_algorithm,
                   pami_algorithm_t  **always_works_alg,
                   pami_metadata_t   **always_works_md,
                   pami_algorithm_t  **must_query_alg,
                   pami_metadata_t   **must_query_md)
{
  pami_result_t     result           = PAMI_SUCCESS;

  result = PAMI_Geometry_algorithms_num(geometry,
                                        xfer_type,
                                        num_algorithm);

  if (result != PAMI_SUCCESS || num_algorithm[0]==0)
    {
      fprintf (stderr,
               "Error. query, Unable to query algorithm, or no algorithms available result = %d\n",
               result);
      return 1;
    }

  *always_works_alg = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[0]);
  *always_works_md  = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[0]);
  *must_query_alg   = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[1]);
  *must_query_md    = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[1]);
  result = PAMI_Geometry_algorithms_query(geometry,
                                          xfer_type,
                                          *always_works_alg,
                                          *always_works_md,
                                          num_algorithm[0],
                                          *must_query_alg,
                                          *must_query_md,
                                          num_algorithm[1]);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to get query algorithm. result = %d\n", result);
      return 1;
    }
  return 0;
}




int create_and_query_geometry(pami_client_t           client,
                              pami_context_t          context,  /* context for create callback */
                              pami_context_t          contextq, /* context for query */
                              pami_geometry_t         parent_geometry,
                              pami_geometry_t        *new_geometry,
                              pami_geometry_range_t  *range,
                              unsigned                numranges,
                              unsigned                id,
                              pami_xfer_type_t        xfer_type,
                              size_t                 *num_algorithm,
                              pami_algorithm_t      **always_works_alg,
                              pami_metadata_t       **always_works_md,
                              pami_algorithm_t      **must_query_alg,
                              pami_metadata_t       **must_query_md)
{
  pami_result_t result;
  int           geom_init=1;
  gContext = context;

  pami_configuration_t config;
  config.name = PAMI_GEOMETRY_OPTIMIZE;
  
  result = PAMI_Geometry_create_taskrange (client,
                                           0,
                                           &config, /*NULL*/
                                           1, /*0, */
                                           new_geometry,
                                           parent_geometry,
                                           id,
                                           range,
                                           numranges,
                                           context,
                                           cb_done,
                                           &geom_init);
    if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to create a new geometry. result = %d\n", result);
      return 1;
    }
    while (geom_init)
      result = PAMI_Context_advance (context, 1);

    gContext = NULL;

    return query_geometry(client,
                          contextq,
                          *new_geometry,
                          xfer_type,
                          num_algorithm,
                          always_works_alg,
                          always_works_md,
                          must_query_alg,
                          must_query_md);
}

void reduce_initialize_sndbuf(void *buf, int count, int op, int dt, int task_id, int num_tasks)
{

  int i;

  if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *ibuf = (unsigned int *)  buf;

    for (i = 0; i < count; i++)
    {
      ibuf[i] = i;
    }
  }
  else if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *dbuf = (double *)  buf;

    for (i = 0; i < count; i++)
    {
      dbuf[i] = 1.0 * i;
    }
  }
  else if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *dbuf = (float *)  buf;

    for (i = 0; i < count; i++)
    {
      dbuf[i] = 1.0 * i;
    }
  }
  else if ((op_array[op] == PAMI_DATA_MAX || op_array[op] == PAMI_DATA_MIN) && dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    memset(buf,  0,  count * sizeof(double));
    double *dbuf = (double *)  buf;

    for (i = task_id; i < count; i += num_tasks)
    {
      dbuf[i] = 1.0 * task_id;
    }
  }
  else
  {
    size_t sz=get_type_size(dt_array[dt]);
    memset(buf,  task_id,  count * sz);
  }
}

int reduce_check_rcvbuf(void *buf, int count, int op, int dt, int task_id, int num_tasks)
{

  int i;

  int err = 0;

  if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *rcvbuf = (unsigned int *)  buf;

    for (i = 0; i < count; i++)
    {
      if (rcvbuf[i] != (unsigned) i * num_tasks)
      {
        fprintf(stderr, "%s:Check %s/%s(%d) failed rcvbuf[%d] %u != %u\n", gProtocolName, dt_array_str[dt], op_array_str[op], count, i, rcvbuf[i], i*num_tasks);
        err = -1;
        return err;
      }
    }
  }
  else if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *rcvbuf = (double *)  buf;

    for (i = 0; i < count; i++)
    {
      if (rcvbuf[i] != 1.0 * i * num_tasks)
      {
        fprintf(stderr, "%s:Check %s/%s(%d) failed rcvbuf[%d] %f != %f\n", gProtocolName, dt_array_str[dt], op_array_str[op], count, i, rcvbuf[i], (double)1.0*i*num_tasks);
        err = -1;
        return err;
      }
    }
  }
  else if (op_array[op] == PAMI_DATA_SUM && dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *rcvbuf = (float *)  buf;

    for (i = 0; i < count; i++)
    {
      if (rcvbuf[i] != 1.0 * i * num_tasks)
      {
        fprintf(stderr, "%s:Check %s/%s(%d) failed rcvbuf[%d] %f != %f\n", gProtocolName, dt_array_str[dt], op_array_str[op], count, i, rcvbuf[i], (float)1.0*i*num_tasks);
        err = -1;
        return err;
      }
    }
  }
  else if (op_array[op] == PAMI_DATA_MIN && dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *rcvbuf = (double *)  buf;

    for (i = 0; i < count; i++)
    {
      if (rcvbuf[i] != 0.0)
      {
        fprintf(stderr, "%s:Check %s/%s(%d) failed rcvbuf[%d] %f != %f\n", gProtocolName, dt_array_str[dt], op_array_str[op], count, i, rcvbuf[i], (double)0.0);
        err = -1;
        return err;
      }
    }
  }
  else if (op_array[op] == PAMI_DATA_MAX && dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *rcvbuf = (double *)  buf;

    for (i = 0; i < count; i++)
    {
      if (rcvbuf[i] != 1.0 * (i % num_tasks))
      {
        fprintf(stderr, "%s:Check %s/%s(%d) failed rcvbuf[%d] %f != %f\n", gProtocolName, dt_array_str[dt], op_array_str[op], count, i, rcvbuf[i], (double)1.0*(i % num_tasks));
        err = -1;
        return err;
      }
    }
  }

  return err;
}

static size_t get_type_size(pami_type_t intype)
{
  pami_result_t        res;
  pami_configuration_t config;


  config.name=PAMI_TYPE_DATA_SIZE;
  res        =PAMI_Type_query (intype,&config,1);
  if(res != PAMI_SUCCESS)
  {
    fprintf(stderr, "Fatal:  error querying size of type:  rc=%d\n",res);
    exit(0);
  }

  size_t sz =(size_t)config.value.intval;
  return sz;
}

unsigned primitive_dt(pami_type_t dt)
{
  unsigned found = 0,i;
  for(i = 0; i < DT_COUNT; ++i)
  {
    if(dt_array[i] == dt) found = 1;
  }
  return found;
}

metadata_result_t check_metadata(pami_metadata_t md,
                                 pami_xfer_t xfer,
                                 pami_type_t s_dt, size_t s_size, char* s_buffer,
                                 pami_type_t r_dt, size_t r_size, char* r_buffer
                                 )
{
  metadata_result_t result;
  if (md.check_fn)
  {
    result = md.check_fn(&xfer);
  }
  else /* Must check parameters ourselves... */
  {
    uint64_t  mask=0;
    result.bitmask = 0;
    if(md.check_correct.values.sendminalign)
    {
      mask  = md.send_min_align - 1;
      result.check.align_send_buffer = (((size_t)s_buffer & (size_t)mask) == 0) ? 0:1;
    }
    if(md.check_correct.values.recvminalign)
    {
      mask  = md.recv_min_align - 1;
      result.check.align_recv_buffer = (((size_t)r_buffer & (size_t)mask) == 0) ? 0:1;
    }
    if(md.check_correct.values.rangeminmax)
    {
      result.check.range  = !((s_size <= md.range_hi) &&
                              (s_size >= md.range_lo));
      result.check.range |= !((r_size <= md.range_hi) &&
                              (r_size >= md.range_lo));
    }
    /* Very basic checks (primitives only) for continuous/contiguous */
    if(md.check_correct.values.contigsflags)
      result.check.contiguous_send = !primitive_dt(s_dt);
    if(md.check_correct.values.contigrflags)
      result.check.contiguous_recv = !primitive_dt(r_dt);
    if(md.check_correct.values.continsflags)
      result.check.continuous_send = !primitive_dt(s_dt);
    if(md.check_correct.values.continrflags)
      result.check.continuous_recv = !primitive_dt(r_dt);
  }
  if(gVerbose==2)
    printf("Query Result: unspec=%u range=%u align_rbuf=%u align_sbuf=%u dt_op=%u cgs=%u cgr=%u cts=%u ctr=%u nonlocal=%u\n",
           result.check.unspecified,
           result.check.range,
           result.check.align_send_buffer,
           result.check.align_recv_buffer,
           result.check.datatype_op,
           result.check.contiguous_send,
           result.check.contiguous_recv,
           result.check.continuous_send,
           result.check.continuous_recv,
           result.check.nonlocal);
  return result;
}

#endif /* __test_api_coll_util_h__*/
