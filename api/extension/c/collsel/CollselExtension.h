/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file api/extension/c/collsel/Extension.h
 */
#ifndef __api_extension_c_collsel_CollselExtension_h__
#define __api_extension_c_collsel_CollselExtension_h__

#include "sys/pami.h"

namespace PAMI{

typedef void* advisor_t;
typedef void* advisor_table_t;
typedef enum {

}  advisor_attribute_name_t;

typedef union
{
  size_t         intval;
  double         doubleval;
  const char *   chararray;
  const size_t * intarray;
} advisor_attribute_value_t;

typedef struct
{
  advisor_attribute_name_t  name;
  advisor_attribute_value_t value;
} advisor_configuration_t;

typedef struct {
   pami_xfer_type_t  *collectives;
   size_t             num_collectives;
   size_t            *geometry_sizes;
   size_t             num_geometry_sizes;
   size_t            *message_sizes;
   size_t             num_message_sizes;
} advisor_params_t;

typedef struct
{
  pami_algorithm_t            algo;
  pami_metadata_t             md;
  int                         must_query;
} sorted_algorithm_t;

typedef void* fast_query_t;

class CollselExtension
{
public:
  static pami_result_t Collsel_init_fn(pami_client_t            client,
                             advisor_configuration_t  configuration[],
                             size_t                   num_configs,
                             pami_context_t           contexts[],
                             size_t                   num_contexts,
                             advisor_t               *advisor);

  static pami_result_t Collsel_destroy_fn(advisor_t *advisor);

  static pami_result_t Collsel_table_generate_fn(advisor_t         advisor,
                                       char             *filename,
                                       advisor_params_t *params,
                                       int               mode);

  static pami_result_t Collsel_table_load_fn(advisor_t        advisor,
                                   char            *filename,
                                   advisor_table_t *advisor_table);

  static pami_result_t Collsel_table_unload_fn(advisor_table_t *advisor_table);

  static pami_result_t Collsel_query_fn(advisor_table_t *advisor_table,
                              pami_geometry_t  geometry,
                              fast_query_t    *fast_query );

  static pami_result_t Collsel_advise_fn(fast_query_t        fast_query,
                               pami_xfer_type_t    xfer_type,
                               pami_xfer_t        *xfer,
                               sorted_algorithm_t  algorithms_optimized[],
                               size_t              max_algorithms);
};
};

#endif // __api_extension_c_collsel_Extension_h__
