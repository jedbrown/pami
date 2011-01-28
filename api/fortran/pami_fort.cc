/**
 * \file api/fortran/pami_fort.cc
 * \brief PAMI C interface common implementation
 */
#ifndef pami_LAPI_IMPL
//#include "config.h"
#endif //pami_LAPI_IMPL

//#include "Global.h"
//#include "Client.h"
//#include "Context.h"
#include <string.h>
#include <pami.h>
//#include "util/common.h"
//#include "algorithms/protocols/allreduce/ReduceFunctions.h" // For dt_query

extern "C" {
    void *pami_addr_null = NULL;
#define PAMI_ADDR_NULL pami_addr_null
    struct {
        pami_type_t         PAMI_TYPE_CONTIGUOUS;
        pami_data_function  PAMI_DATA_COPY; 
        pami_geometry_t     PAMI_NULL_GEOMETRY;
    } pami_fort_globals;
}

#define FIX_PAMI_ADDR_NULL(x, y) \
{   \
    if ((x)==(void*)&pami_addr_null) {  \
        (y) = 0;    \
    } else {    \
        (y) = *(x); \
    }   \
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_misc.h                                                  //
////////////////////////////////////////////////////////////////////////////////

extern "C" void pami_error_text (char* string, size_t* length, size_t* result)
{
    *result = PAMI_Error_text(string, *length);
}

extern "C" void pami_wtime (double* result)
{
    *result = PAMI_Wtime();
}

extern "C" void pami_wtimebase(unsigned long long *result)
{
    *result = PAMI_Wtimebase();
}

extern "C" void pami_endpoint_create (pami_client_t*    client,
                                      pami_task_t*      task,
                                      size_t*           offset,
                                      pami_endpoint_t*  endpoint,
                                      pami_result_t*    result)
{
    *result = PAMI_Endpoint_create(*client, *task, *offset, endpoint);
}

extern "C" void pami_endpoint_createv (pami_client_t*    client,
                                       pami_task_t*      task,
                                       pami_endpoint_t*  endpoints,
                                       size_t*           count,
                                       pami_result_t*    result)
{
    *result = PAMI_Endpoint_createv(*client, *task, endpoints, count);
}

extern "C" void pami_endpoint_query (pami_endpoint_t*  endpoint,
                                     pami_task_t*      task,
                                     size_t*           offset,
                                     pami_result_t*    result)
{
    *result = PAMI_Endpoint_query(*endpoint, task, offset);
}


extern "C" void pami_context_post (pami_context_t*      context,
                                   pami_work_t*         work,
                                   pami_work_function   fn,
                                   void*                cookie,
                                   pami_result_t*       result)
{
    *result = PAMI_Context_post(*context, work, fn, cookie);
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_fence.h                                                 //
////////////////////////////////////////////////////////////////////////////////

extern "C" void pami_fence_begin (pami_context_t*     context, 
                                  pami_result_t*      result)
{
    *result = PAMI_Fence_begin(*context);
}

extern "C" void pami_fence_end (pami_context_t*     context,
                                pami_result_t*      result)
{
    *result = PAMI_Fence_end(*context);
}

extern "C" void pami_fence_all (pami_context_t*       context,
                                pami_event_function*  done_fn,
                                void*                 cookie,
                                pami_result_t*        result)
{
    *result = PAMI_Fence_all(*context, *done_fn, cookie);
}

extern "C" void pami_fence_endpoint (pami_context_t*       context,
                                     pami_event_function   done_fn,
                                     void*                 cookie,
                                     pami_endpoint_t*      endpoint,
                                     pami_result_t*        result)
{
    *result = PAMI_Fence_endpoint(*context, done_fn, cookie, *endpoint);
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_collectives.h                                           //
////////////////////////////////////////////////////////////////////////////////


extern "C" void pami_geometry_create_taskrange (pami_client_t*          client,
                                                pami_configuration_t    configuration[],
                                                size_t*                 num_configs,
                                                pami_geometry_t*        geometry,
                                                pami_geometry_t*        parent,
                                                unsigned*               id,
                                                pami_geometry_range_t*  task_slices,
                                                size_t*                 slice_count,
                                                pami_context_t*         context,
                                                pami_event_function     fn,
                                                void*                   cookie,
                                                pami_result_t*          result)
{
    *result = PAMI_Geometry_create_taskrange(*client, configuration,
            *num_configs, geometry, *parent, *id, task_slices, *slice_count,
            *context, fn, cookie);
}

extern "C" void pami_geometry_create_tasklist (pami_client_t*              client,
                                               pami_configuration_t        configuration[],
                                               size_t*                     num_configs,
                                               pami_geometry_t*            geometry,
                                               pami_geometry_t*            parent,
                                               unsigned*                   id,
                                               pami_task_t*                tasks,
                                               size_t*                     task_count,
                                               pami_context_t*             context,
                                               pami_event_function         fn,
                                               void*                       cookie,
                                               pami_result_t*              result)
{
    *result = PAMI_Geometry_create_tasklist(*client, configuration,
            *num_configs, geometry, *parent, *id, tasks, *task_count, *context,
            fn, cookie);
}


extern "C" void pami_geometry_query (pami_geometry_t*      geometry,
                                     pami_configuration_t  configuration[],
                                     size_t*               num_configs,
                                     pami_result_t*        result)
{
    *result = PAMI_Geometry_query(*geometry, configuration, *num_configs);
}

extern "C"  void pami_geometry_update (pami_geometry_t*      geometry,
                                       pami_configuration_t  configuration[],
                                       size_t*               num_configs,
                                       pami_context_t*       context,
                                       pami_event_function   fn,
                                       void*                 cookie,
                                       pami_result_t*        result)
{
    *result = PAMI_Geometry_update(*geometry, configuration, *num_configs,
            *context, fn, cookie);
}

extern "C" void pami_geometry_world (pami_client_t*               client,
                                     pami_geometry_t*             world_geometry,
                                     pami_result_t*               result)
{
    *result = PAMI_Geometry_world(*client, world_geometry);
}

extern "C" void pami_geometry_destroy(pami_client_t*   client,
                                      pami_geometry_t* geometry,
                                      pami_result_t*   result)
{
    *result = PAMI_Geometry_destroy(*client, geometry);
}

extern "C" void pami_collective (pami_context_t*  context,
                                 pami_xfer_t*     parameters,
                                 pami_result_t*   result)
{
    *result = PAMI_Collective(*context, parameters);
}

extern "C" void pami_geometry_algorithms_num (pami_context_t*    context,
                                              pami_geometry_t*   geometry,
                                              pami_xfer_type_t*  coll_type,
                                              size_t             lists_lengths[2],
                                              pami_result_t*     result)
{
    *result = PAMI_Geometry_algorithms_num(*context, *geometry, *coll_type,
            lists_lengths);
}

extern "C"  void pami_geometry_algorithms_query (pami_context_t*     context,
                                                 pami_geometry_t*    geometry,
                                                 pami_xfer_type_t*   colltype,
                                                 pami_algorithm_t*   algs0,
                                                 pami_metadata_t*    mdata0,
                                                 size_t*             num0,
                                                 pami_algorithm_t*   algs1,
                                                 pami_metadata_t*    mdata1,
                                                 size_t*             num1,
                                                 pami_result_t*      result)
{
    *result = PAMI_Geometry_algorithms_query(*context, *geometry, *colltype,
            algs0, mdata0, *num0, algs1, mdata1, *num1);
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_dispatch.h                                              //
////////////////////////////////////////////////////////////////////////////////

extern "C" void pami_amcollective_dispatch_set(pami_context_t*            context,
        pami_algorithm_t*          algorithm,
        size_t*                    dispatch,
        pami_dispatch_callback_function  fn,
        void*                      cookie,
        pami_collective_hint_t*    options,
        pami_result_t*             result)
{
    *result = PAMI_AMCollective_dispatch_set(*context, *algorithm, *dispatch,
            fn, cookie, *options);
}

////////////////////////////////////////////////////////////////////////////////
// Type function implementations                                              //
////////////////////////////////////////////////////////////////////////////////

extern "C" void pami_type_create (pami_type_t* type, pami_result_t* result)
{
    *result = PAMI_Type_create(type);
}

extern "C" void pami_type_add_simple (pami_type_t*       type,
        size_t*            bytes,
        size_t*            offset,
        size_t*            count,
        size_t*            stride,
        pami_result_t*     result)
{
    *result = PAMI_Type_add_simple(*type, *bytes, *offset, *count, *stride);
}

extern "C" void pami_type_add_typed (pami_type_t*       type,
        pami_type_t*       subtype,
        size_t*            offset,
        size_t*            count,
        size_t*            stride,
        pami_result_t*     result)
{
    *result = PAMI_Type_add_typed(*type, *subtype, *offset, *count, *stride);
}

extern "C" void pami_type_complete (pami_type_t*         type, 
        size_t*              atom_size,
        pami_result_t*       result)
{
    *result = PAMI_Type_complete(*type, *atom_size);
}

extern "C" void pami_type_destroy (pami_type_t* type, pami_result_t* result)
{
    *result = PAMI_Type_destroy(type);
}

extern "C" void pami_type_serialize (pami_type_t*       type,
        void**             address,
        size_t*            size,
        pami_result_t*     result)
{
    *result = PAMI_Type_serialize(*type, address, size);
}

extern "C" void pami_type_deserialize (pami_type_t*     type,
        void**           address,
        size_t*          size,
        pami_result_t*   result)
{
    *result = PAMI_Type_deserialize(type, *address, *size);
}

extern "C" void pami_type_query (pami_type_t*          type,
        pami_configuration_t  config[],
        size_t*               num_configs,
        pami_result_t*        result)
{
    *result = PAMI_Type_query(*type, config, *num_configs);
}

extern "C" void pami_type_transform_data (void**               src_addr,
        pami_type_t*         src_type,
        size_t*              src_offset,
        void**               dst_addr,
        pami_type_t*         dst_type,
        size_t*              dst_offset,
        size_t*              size,
        pami_data_function   data_fn,
        void*                cookie,
        pami_result_t*       result)
{
    *result = PAMI_Type_transform_data(*src_addr, *src_type, *src_offset,
            *dst_addr, *dst_type, *dst_offset, *size, data_fn, cookie);
}

#ifdef USE_COMMTHREADS // from (bgq/) Client.h
/// \todo #warning pami_client_add_commthread_context() is non-standard API
extern "C" void pami_client_add_commthread_context(pami_client_t*   client,
        pami_context_t*  context,
        pami_result_t*   result)
{
    *result = PAMI_Client_add_commthread_context(*client, context);
}
#endif // USE_COMMTHREADS


extern "C" void pami_client_create (const char*             name,
                                    pami_client_t*          client,
                                    pami_configuration_t*   configuration[],
                                    size_t*                 num_configs,
                                    pami_result_t*          result)
{
    pami_configuration_t*   conf;
    FIX_PAMI_ADDR_NULL(configuration, conf);
    *result = PAMI_Client_create(name, client, conf, *num_configs);

    // initialize global variables in pami.h for FORTRAN
    // TODO: pami_fort_globals need to be synchronized with these globals
    pami_fort_globals.PAMI_TYPE_CONTIGUOUS             = PAMI_TYPE_CONTIGUOUS;
    pami_fort_globals.PAMI_DATA_COPY        = PAMI_DATA_COPY;
    pami_fort_globals.PAMI_NULL_GEOMETRY    = PAMI_NULL_GEOMETRY;
}

extern "C" void pami_client_destroy (pami_client_t*     client,
                                     pami_result_t*     result)
{
    *result = PAMI_Client_destroy(client);
}

extern "C" void pami_context_createv (pami_client_t*          client,
                                      pami_configuration_t*   configuration[],
                                      size_t*                 count,
                                      pami_context_t*         context,
                                      size_t*                 ncontexts,
                                      pami_result_t*          result)
{
    pami_configuration_t*   conf;
    FIX_PAMI_ADDR_NULL(configuration, conf);
    *result = PAMI_Context_createv(*client, conf, *count, context, *ncontexts);
}

extern "C" void pami_context_destroyv (pami_context_t* contexts,
        size_t*         ncontexts,
        pami_result_t*  result)
{
    *result = PAMI_Context_destroyv(contexts, *ncontexts);
}

extern "C" void pami_client_query (pami_client_t*        client,
        pami_configuration_t  configuration[],
        size_t*               num_configs,
        pami_result_t*        result)
{
    *result = PAMI_Client_query(*client, configuration, *num_configs);
}

extern "C" void pami_client_update (pami_client_t*        client,
        pami_configuration_t  configuration[],
        size_t*               num_configs,
        pami_result_t*        result)
{
    *result = PAMI_Client_update(*client, configuration, *num_configs);
}

extern "C" void pami_dispatch_query (pami_context_t*       context,
        size_t*               dispatch,
        pami_configuration_t  configuration[],
        size_t*               num_configs,
        pami_result_t*        result)
{
    *result = PAMI_Dispatch_query(*context, *dispatch, configuration,
            *num_configs);
}

extern "C" void pami_dispatch_update (pami_context_t*       context,
        size_t*               dispatch,
        pami_configuration_t  configuration[],
        size_t*               num_configs,
        pami_result_t*        result)
{
    *result = PAMI_Dispatch_update(*context, *dispatch, configuration,
            *num_configs);
}

extern "C" void pami_context_query (pami_context_t*       context,
        pami_configuration_t  configuration[],
        size_t*               num_configs,
        pami_result_t*        result)
{
    *result = PAMI_Context_query(*context, configuration, *num_configs);
}

extern "C" void pami_context_update (pami_context_t*       context,
        pami_configuration_t  configuration[],
        size_t*               num_configs,
        pami_result_t*        result)
{
    *result = PAMI_Context_update(*context, configuration, *num_configs);
}

extern "C" void pami_dispatch_set (pami_context_t*            context,
        size_t*                    dispatch,
        pami_dispatch_callback_function fn,
        void*                      cookie,
        pami_send_hint_t*          options,
        pami_result_t*             result)
{
    *result = PAMI_Dispatch_set(*context, *dispatch, fn, cookie, *options);
}

extern "C" void pami_context_lock(pami_context_t* context, pami_result_t* result) 
{
    *result = PAMI_Context_lock(*context);
}

extern "C" void pami_context_trylock(pami_context_t* context, pami_result_t* result) 
{
    *result = PAMI_Context_trylock(*context);
}

extern "C" void pami_context_unlock(pami_context_t* context, pami_result_t* result)
{
    *result = PAMI_Context_unlock(*context);
}

extern "C" void pami_context_advance (pami_context_t* context, size_t* maximum,
        pami_result_t* result)
{
    *result = PAMI_Context_advance(*context, *maximum);
}

extern "C" void pami_context_advancev (pami_context_t context[],
        size_t*        count,
        size_t*        maximum,
        pami_result_t* result)
{
    *result = PAMI_Context_advancev(context, *count, *maximum);
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_p2p.h                                                   //
////////////////////////////////////////////////////////////////////////////////

extern "C" void pami_send (pami_context_t*  context,
        pami_send_t*     parameters,
        pami_result_t*   result)
{
    *result = PAMI_Send(*context, parameters);
}

extern "C" void pami_send_immediate (pami_context_t*         context,
        pami_send_immediate_t*  parameters,
        pami_result_t*          result)
{
    *result = PAMI_Send_immediate(*context, parameters);
}

extern "C" void pami_send_typed (pami_context_t*     context,
        pami_send_typed_t*  parameters,
        pami_result_t*      result)
{
    *result = PAMI_Send_typed(*context, parameters);
}


extern "C" void pami_put (pami_context_t*     context,
        pami_put_simple_t*  parameters,
        pami_result_t*      result)
{
    *result = PAMI_Put(*context, parameters);
}


extern "C" void pami_put_typed (pami_context_t*     context,
        pami_put_typed_t*   parameters,
        pami_result_t*      result)
{
    *result = PAMI_Put_typed(*context, parameters);
}

extern "C" void pami_get (pami_context_t*     context,
        pami_get_simple_t*  parameters,
        pami_result_t*      result)
{
    *result = PAMI_Get(*context, parameters);
}


extern "C" void pami_get_typed (pami_context_t*     context,
        pami_get_typed_t*   parameters,
        pami_result_t*      result)
{
    *result = PAMI_Get_typed(*context, parameters);
}


extern "C" void pami_rmw (pami_context_t*     context,
        pami_rmw_t*         parameters,
        pami_result_t*      result)
{
    *result = PAMI_Rmw(*context, parameters);
}

extern "C" void pami_memregion_create (pami_context_t*    context,
        void**             address,
        size_t*            bytes_in,
        size_t*            bytes_out,
        pami_memregion_t*  memregion,
        pami_result_t*     result)
{
    *result = PAMI_Memregion_create(*context, *address, *bytes_in, bytes_out,
            memregion); 
}

extern "C" void pami_memregion_destroy (pami_context_t*    context,
        pami_memregion_t*  memregion,
        pami_result_t*     result)
{
    *result = PAMI_Memregion_destroy(*context, memregion);
}

extern "C" void pami_rput (pami_context_t*     context,
        pami_rput_simple_t* parameters,
        pami_result_t*      result)
{
    *result = PAMI_Rput(*context, parameters);
}


extern "C" void pami_rput_typed (pami_context_t*     context,
        pami_rput_typed_t*  parameters,
        pami_result_t*      result)
{
    *result = PAMI_Rput_typed(*context, parameters);
}

extern "C" void pami_rget (pami_context_t*     context,
        pami_rget_simple_t* parameters,
        pami_result_t*      result)
{
    *result = PAMI_Rget(*context, parameters);
}


extern "C" void pami_rget_typed (pami_context_t*     context,
        pami_rget_typed_t*  parameters,
        pami_result_t*      result)
{
    *result = PAMI_Rget_typed(*context, parameters);
}


extern "C" void pami_purge(pami_context_t*   context,
        pami_endpoint_t*  dest,
        size_t*           count,
        pami_result_t*    result)
{
    *result = PAMI_Purge(*context, dest, *count);
}


extern "C"   void pami_resume (pami_context_t*   context,
        pami_endpoint_t*  dest,
        size_t*           count,
        pami_result_t*    result)
{
    *result = PAMI_Resume(*context, dest, *count);
}

extern "C" void pami_dt_query (pami_dt*          dt, 
        size_t*           size,
        pami_result_t*    result)
{
    *result = PAMI_Dt_query(*dt, size);
}

//#include "api/extension/Extension.h"
//#include "api/extension/registry.h"

extern "C" void pami_extension_open (pami_client_t*     client,
        const char*        name,
        pami_extension_t   extension,
        pami_result_t*     result)
{
    *result = PAMI_Extension_open(*client, name, &extension);
}

extern "C" void pami_extension_close (pami_extension_t  extension,
        pami_result_t*    result)
{
    *result = PAMI_Extension_close(extension);
}

extern "C" void pami_extension_symbol (pami_extension_t   extension,
        const char*        fn,
        void*              result)
{
    result = PAMI_Extension_symbol(extension, fn);
}

////////////////////////////////////////////////////////////////////////////////
// Functions for Fortran only
////////////////////////////////////////////////////////////////////////////////
// TODO: update PAMI C API is needed

extern "C" pami_result_t PAMI_Address (void* addr_in, size_t* addr_out)
{
    pami_result_t result = PAMI_SUCCESS;
    if (addr_in == NULL || addr_out == NULL) {
        *addr_out = NULL;
        result = PAMI_INVAL;
    } else {
        *addr_out = (size_t)addr_in;
    }

    return result;
}

extern "C" void pami_address (void* addr_in, size_t* addr_out, pami_result_t* result)
{
    *result = PAMI_Address(addr_in, addr_out);
}

// include function translations
// pami_*lowercase* <-- PAMI_*UPPERCASE*

#define ALIAS(FUNC2,FUNC,PARAMS) \
    void PAMI_##FUNC PARAMS __attribute__ ((weak,alias("pami_"#FUNC)));

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_misc.h                                                  //
////////////////////////////////////////////////////////////////////////////////

ALIAS(ERROR_TEXT,error_text,(char* string,size_t* length,size_t* result));

ALIAS(WTIME,wtime,(double* result));

ALIAS(WTIMEBASE,wtimebase,(unsigned long long *result));

ALIAS(ENDPOINT_CREATE,endpoint_create,(pami_client_t* client,pami_task_t* task,size_t* offset,pami_endpoint_t* endpoint,pami_result_t* result));

ALIAS(ENDPOINT_CREATEV,endpoint_createv,(pami_client_t* client,pami_task_t* task,pami_endpoint_t* endpoints,size_t* count,pami_result_t* result));

ALIAS(ENDPOINT_QUERY,endpoint_query,(pami_endpoint_t* endpoint,pami_task_t* task,size_t* offset,pami_result_t* result));


ALIAS(CONTEXT_POST,context_post,(pami_context_t* context,pami_work_t* work,pami_work_function fn,void* cookie,pami_result_t* result));

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_fence.h                                                 //
////////////////////////////////////////////////////////////////////////////////

ALIAS(FENCE_BEGIN,fence_begin,(pami_context_t* context,pami_result_t* result));

ALIAS(FENCE_END,fence_end,(pami_context_t* context,pami_result_t* result));

ALIAS(FENCE_ALL,fence_all,(pami_context_t* context,pami_event_function done_fn,void* cookie,pami_result_t* result));

ALIAS(FENCE_ENDPOINT,fence_endpoint,(pami_context_t* context,pami_event_function done_fn,void* cookie,pami_endpoint_t* endpoint,pami_result_t* result));

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_collectives.h                                           //
////////////////////////////////////////////////////////////////////////////////


ALIAS(GEOMETRY_CREATE_TASKRANGE,geometry_create_taskrange,(pami_client_t* client,pami_configuration_t configuration[],size_t* num_configs,pami_geometry_t* geometry,pami_geometry_t* parent,unsigned* id,pami_geometry_range_t* task_slices,size_t* slice_count,pami_context_t* context,pami_event_function fn,void* cookie,pami_result_t* result));

ALIAS(GEOMETRY_CREATE_TASKLIST,geometry_create_tasklist,(pami_client_t* client,pami_configuration_t configuration[],size_t* num_configs,pami_geometry_t* geometry,pami_geometry_t* parent,unsigned* id,pami_task_t* tasks,size_t* task_count,pami_context_t* context,pami_event_function fn,void* cookie,pami_result_t* result));


ALIAS(GEOMETRY_QUERY,geometry_query,(pami_geometry_t* geometry,pami_configuration_t  configuration[],size_t* num_configs,pami_result_t* result));

ALIAS(GEOMETRY_UPDATE,geometry_update,(pami_geometry_t* geometry,pami_configuration_t  configuration[],size_t* num_configs,pami_context_t* context,pami_event_function fn,void* cookie,pami_result_t* result));

ALIAS(GEOMETRY_WORLD,geometry_world,(pami_client_t* client,pami_geometry_t* world_geometry,pami_result_t* result));

ALIAS(GEOMETRY_DESTROY,geometry_destroy,(pami_client_t* client,pami_geometry_t* geometry,pami_result_t* result));

ALIAS(COLLECTIVE,collective,(pami_context_t* context,pami_xfer_t* parameters,pami_result_t* result));

ALIAS(GEOMETRY_ALGORITHMS_NUM,geometry_algorithms_num,(pami_context_t* context,pami_geometry_t* geometry,pami_xfer_type_t* coll_type,size_t lists_lengths[2],pami_result_t* result));

ALIAS(GEOMETRY_ALGORITHMS_QUERY,geometry_algorithms_query,(pami_context_t* context,pami_geometry_t* geometry,pami_xfer_type_t* colltype,pami_algorithm_t* algs0,pami_metadata_t* mdata0,size_t* num0,pami_algorithm_t* algs1,pami_metadata_t* mdata1,size_t* num1,pami_result_t* result));

////////////////////////////////////////////////////////////////////////////////
// Geometry Utility functions                                                 //
////////////////////////////////////////////////////////////////////////////////

///

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_dispatch.h                                              //
////////////////////////////////////////////////////////////////////////////////

ALIAS(AMCOLLECTIVE_DISPATCH_SET,amcollective_dispatch_set,(pami_context_t* context,pami_algorithm_t* algorithm,size_t* dispatch,pami_dispatch_callback_function fn,void* cookie,pami_collective_hint_t* options,pami_result_t* result));

////////////////////////////////////////////////////////////////////////////////
// Type function implementations                                              //
////////////////////////////////////////////////////////////////////////////////

ALIAS(TYPE_CREATE,type_create,(pami_type_t* type,pami_result_t* result));

ALIAS(TYPE_ADD_SIMPLE,type_add_simple,(pami_type_t* type,size_t* bytes,size_t* offset,size_t* count,size_t* stride,pami_result_t* result));

ALIAS(TYPE_ADD_TYPED,type_add_typed,(pami_type_t* type,pami_type_t* subtype,size_t* offset,size_t* count,size_t* stride,pami_result_t* result));

ALIAS(TYPE_COMPLETE,type_complete,(pami_type_t* type,size_t* atom_size,pami_result_t* result));

ALIAS(TYPE_DESTROY,type_destroy,(pami_type_t* type,pami_result_t* result));

ALIAS(TYPE_SERIALIZE,type_serialize,(pami_type_t* type,void** address,size_t* size,pami_result_t* result));

ALIAS(TYPE_DESERIALIZE,type_deserialize,(pami_type_t* type,void** address,size_t* size,pami_result_t* result));

ALIAS(TYPE_QYERY,type_query,(pami_type_t* type,pami_configuration_t config[],size_t* num_configs,pami_result_t* result));

ALIAS(TYPE_TRANSFORM_DATA,type_transform_data,(void** src_addr,pami_type_t* src_type,size_t* src_offset,void** dst_addr,pami_type_t* dst_type,size_t* dst_offset,size_t* size,pami_data_function data_fn,void* cookie,pami_result_t* result));

#ifdef USE_COMMTHREADS // from (bgq/) Client.h
/// \todo #warning pami_client_add_commthread_context() is non-standard API
ALIAS(CLIENT_ADD_COMMTHREAD_CONTEXT,client_add_commthread_context,(pami_client_t* client,pami_context_t* context,pami_result_t* result));
#endif // USE_COMMTHREADS


ALIAS(CLIENT_CREATE,client_create,(const char* name,pami_client_t* client,pami_configuration_t  configuration[],size_t* num_configs,pami_result_t* result));

ALIAS(CLIENT_DESTROY,client_destroy,(pami_client_t* client,pami_result_t* result));

ALIAS(CONTEXT_CREATEV,context_createv,(pami_client_t* client,pami_configuration_t*   configuration[],size_t* count,pami_context_t* context,size_t* ncontexts,pami_result_t* result));

ALIAS(CONTEXT_DESTROYV,context_destroyv,(pami_context_t* contexts,size_t* ncontexts,pami_result_t* result));

ALIAS(CLIENT_QUERY,client_query,(pami_client_t* client,pami_configuration_t  configuration[],size_t* num_configs,pami_result_t* result));

ALIAS(CLIENT_UPDATE,client_update,(pami_client_t* client,pami_configuration_t  configuration[],size_t* num_configs,pami_result_t* result));

ALIAS(DISPATCH_QUERY,dispatch_query,(pami_context_t* context,size_t* dispatch,pami_configuration_t  configuration[],size_t* num_configs,pami_result_t* result));

ALIAS(DISPATCH_UPDATE,dispatch_update,(pami_context_t* context,size_t* dispatch,pami_configuration_t  configuration[],size_t* num_configs,pami_result_t* result));

ALIAS(CONTEXT_QUERY,context_query,(pami_context_t* context,pami_configuration_t  configuration[],size_t* num_configs,pami_result_t* result));

ALIAS(CONTEXT_UPDATE,context_update,(pami_context_t* context,pami_configuration_t  configuration[],size_t* num_configs,pami_result_t* result));

ALIAS(DISPATCH_SET,dispatch_set,(pami_context_t* context,size_t* dispatch,pami_dispatch_callback_function fn,void* cookie,pami_send_hint_t* options,pami_result_t* result));

ALIAS(CONTEXT_LOCK,context_lock,(pami_context_t* context,pami_result_t* result));

ALIAS(CONTEXT_TRYLOCK,context_trylock,(pami_context_t* context,pami_result_t* result));

ALIAS(CONTEXT_UNLOCK,context_unlock,(pami_context_t* context,pami_result_t* result));

ALIAS(CONTEXT_ADVANCE,context_advance,(pami_context_t* context,size_t* maximum,pami_result_t* result));

ALIAS(CONTEXT_ADVANCEV,context_advancev,(pami_context_t context[],size_t* count,size_t* maximum,pami_result_t* result));

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_p2p.h                                                   //
////////////////////////////////////////////////////////////////////////////////

ALIAS(SEND,send,(pami_context_t* context,pami_send_t* parameters,pami_result_t* result));

ALIAS(SEND_IMMEDIATE,send_immediate,(pami_context_t* context,pami_send_immediate_t* parameters,pami_result_t* result));

ALIAS(SEND_TYPED,send_typed,(pami_context_t* context,pami_send_typed_t* parameters,pami_result_t* result));


ALIAS(PUT,put,(pami_context_t* context,pami_put_simple_t* parameters,pami_result_t* result));


ALIAS(PUT_TYPED,put_typed,(pami_context_t* context,pami_put_typed_t* parameters,pami_result_t* result));

ALIAS(GET,get,(pami_context_t* context,pami_get_simple_t* parameters,pami_result_t* result));


ALIAS(GET_TYPED,get_typed,(pami_context_t* context,pami_get_typed_t* parameters,pami_result_t* result));


ALIAS(RMW,rmw,(pami_context_t* context,pami_rmw_t* parameters,pami_result_t* result));

ALIAS(MEMREGION_CREATE,memregion_create,(pami_context_t* context,void** address,size_t* bytes_in,size_t* bytes_out,pami_memregion_t* memregion,pami_result_t* result));

ALIAS(MEMREGION_DESTROY,memregion_destroy,(pami_context_t* context,pami_memregion_t* memregion,pami_result_t* result));

ALIAS(RPUT,rput,(pami_context_t* context,pami_rput_simple_t* parameters,pami_result_t* result));


ALIAS(RPUT_TYPED,rput_typed,(pami_context_t* context,pami_rput_typed_t* parameters,pami_result_t* result));

ALIAS(RGET,rget,(pami_context_t* context,pami_rget_simple_t* parameters,pami_result_t* result));


ALIAS(RGET_TYPED,rget_typed,(pami_context_t* context,pami_rget_typed_t* parameters,pami_result_t* result));


ALIAS(PURGE,purge,(pami_context_t* context,pami_endpoint_t* dest,size_t* count,pami_result_t* result)); 


ALIAS(RESUME,resume,(pami_context_t* context,pami_endpoint_t* dest,size_t* count,pami_result_t* result));

ALIAS(DT_QUERY,dt_query,(pami_dt* dt,size_t* size,pami_result_t* result));

//#include "api/extension/Extension.h"
//#include "api/extension/registry.h"

ALIAS(EXTENSION_OPEN,extension_open,(pami_client_t* client,const char* name,pami_extension_t extension,pami_result_t* result));

ALIAS(EXTENSION_CLOSE,extension_close,(pami_extension_t extension,pami_result_t* result));

ALIAS(EXTENSION_FUNCTION,extension_symbol,(pami_extension_t extension,const char* fn,void* result));

ALIAS(ADDRESS,address,(void* addr_in, size_t* addr_out, pami_result_t* result));
