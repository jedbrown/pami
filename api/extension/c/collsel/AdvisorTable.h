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
 * \file api/extension/c/collsel/AdvisorTable.h
 */
#ifndef __api_extension_c_collsel_AdvisorTable_h__
#define __api_extension_c_collsel_AdvisorTable_h__

#include "api/extension/c/collsel/CollselExtension.h"
#include "api/extension/c/collsel/Benchmark.h"
#define DOUBLE_DIG (9.999999999999999e99)
#define COLLSEL_MAX_ALGO 40 // Maximum number of algorithms per collective

namespace PAMI{

class AdvisorTable
{
public:
  AdvisorTable(Advisor &advisor);
  pami_result_t generate(char             *filename,
                         advisor_params_t *params,
                         int               mode);
  pami_result_t load(char* filename);
  pami_result_t unload();
  ~AdvisorTable();
private:
  void init(advisor_params_t *params);

  Advisor &_advisor;
  advisor_params_t _params;
  pami_task_t      _task_id;
  size_t           _ntasks;
  bool             _free_geometry_sz;
  bool             _free_message_sz;
};


inline AdvisorTable::AdvisorTable(Advisor &advisor):
  _advisor(advisor),
  _task_id(0),
  _ntasks(0),
  _free_geometry_sz(false),
  _free_message_sz(false)
{
}

inline AdvisorTable::~AdvisorTable()
{
  if(_free_geometry_sz)
    free(_params.geometry_sizes);
  if(_free_message_sz)
    free(_params.message_sizes);
}

inline void AdvisorTable::init(advisor_params_t *params)
{
  _params = *params;

  // User did not provide geometry sizes, use power of two geometries and adjacent values ( 2^k + 1, 2^k, 2^k - 1) 
  if(!_params.num_geometry_sizes)
  {
    // Geometry list = 2,3,4,5,7,8,9,15,16,17,31,32,33..... 
    if(_ntasks < 7)
    {
      _params.num_geometry_sizes = _ntasks - 1;
      _params.geometry_sizes = (size_t *) malloc(_params.num_geometry_sizes * sizeof(size_t));
      for(unsigned i = 0; i < _params.num_geometry_sizes; ++i)
        _params.geometry_sizes[i] = _ntasks - i;
    }
    else
    {
      bool is_pow2 = !(_ntasks & (_ntasks - 1));
      // Find largest power of 2 smaller than or equal to num tasks
      unsigned i = ipow2(ilog2(_ntasks));
      _params.num_geometry_sizes = ((ilog2(_ntasks) - 1) * 3);
      if(!is_pow2)
      {
        _params.num_geometry_sizes++;
        if(_ntasks != i+1)
          _params.num_geometry_sizes++;
      }
      _params.geometry_sizes = (size_t *) malloc(_params.num_geometry_sizes * sizeof(size_t));

      unsigned j = 0;
      if(!is_pow2)
      {
        _params.geometry_sizes[j++] = _ntasks;
        // If num tasks is 1 greater than power of 2, avoid duplicate 
        if(_ntasks != i+1)
          _params.geometry_sizes[j++] = i + 1;
      }
      _params.geometry_sizes[j++] = i;
      _params.geometry_sizes[j++] = i - 1;

      i >>=1;
      unsigned k = i + 1;
      for(; k > 2 && j < (_params.num_geometry_sizes - 1); ++j)
      {
         _params.geometry_sizes[j] = k--;
         if(k-i == (unsigned) -2)
         {
           i >>=1;
           k = i + 1;
         }
      }
      _params.geometry_sizes[_params.num_geometry_sizes - 1] = 2;
    }
    _free_geometry_sz = true;
  }

  if(!_params.num_message_sizes)
  {
    // 1 byte -> 4194304 bytes
    _params.num_message_sizes = 23;
    _params.message_sizes = (size_t *) malloc(_params.num_message_sizes*sizeof(size_t));

    for (unsigned i=1, j=0; i < (1U<<23); i <<=1)
      _params.message_sizes[j++] = i;

    _free_message_sz = true;
  }
}

inline pami_result_t AdvisorTable::generate(char             *filename,
                                            advisor_params_t *params,
                                            int               mode)
{
  //if(!filename || !params)
    //return PAMI_INVAL;

  pami_configuration_t config;
  pami_client_t client = _advisor._client;
  pami_context_t *contexts = _advisor._contexts;
  config.name = PAMI_CLIENT_TASK_ID;
  pami_result_t result = PAMI_Client_query(client, &config, 1);
  PAMI_assertf(result == PAMI_SUCCESS, "Failed to query client task id");
  _task_id = config.value.intval;

  config.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query(client, &config, 1);
  PAMI_assertf(result == PAMI_SUCCESS, "Failed to query number of tasks");
  _ntasks = config.value.intval;

  pami_geometry_t world_geometry;
  result = PAMI_Geometry_world(client, &world_geometry);
  PAMI_assertf(result == PAMI_SUCCESS, "Failed to get geometry world");

  init(params);
  init_tables();
  init_cutoff_tables();

  /* Setup operation and datatype tables*/
  //gValidTable = alloc2DContig(op_count, dt_count);
  //setup_op_dt(gValidTable,sDt,sOp);

  size_t byte_thresh = (1*1024*1024);

  if(mode == 0){
    //read in the file
  }
  else if(mode == 1)
  {
    size_t i, j, k, l;
    int geometry_id = 1;
    // Setup barrier on world geometry
    pami_xfer_t barrier;
    volatile unsigned barrier_poll_flag = 0;
    query_geometry_algorithm_aw(client, contexts[0], world_geometry, PAMI_XFER_BARRIER, &barrier);
    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & barrier_poll_flag;
    // Allocate buffers to store algorithms and metadata
    pami_algorithm_t  col_algo[COLLSEL_MAX_ALGO];
    pami_metadata_t   col_md[COLLSEL_MAX_ALGO];
    pami_algorithm_t  q_col_algo[COLLSEL_MAX_ALGO];
    pami_metadata_t   q_col_md[COLLSEL_MAX_ALGO];
    sorted_list       algo_list[COLLSEL_MAX_ALGO];
    // Loop on each collective
    for(i=0; i<_params.num_collectives; i++)
    {
      // Loop on geometry size
      for(j=0; j<_params.num_geometry_sizes; j++)
      {
        size_t geo_size = _params.geometry_sizes[j];

        // Create subgeometry
        size_t geo_ntasks = _ntasks;
        pami_task_t local_task_id = _task_id;
        pami_task_t root = 0;
        pami_geometry_t new_geometry = world_geometry;
        if( geo_size < _ntasks)
        {
          //printf("%d:Create geometry with size %zu and id %d\n", _task_id, geo_size, geometry_id);
          result = create_geometry(&geo_ntasks,
                                   &local_task_id,
                                    _task_id,
                                   &root,
                                    geo_size,
                                    geometry_id,
                                   &new_geometry,
                                    client,
                                    PAMI_GEOMETRY_NULL,
                                    contexts[0]);

          //printf("%d:Created geometry<%p> with size %zu and id %d\n", _task_id, new_geometry, geo_size, geometry_id);
          ++geometry_id;
          if(new_geometry == PAMI_GEOMETRY_NULL)
            continue;
        }

        //query algo for barrier, collective and reduce
        pami_xfer_t coll[3];

        pami_xfer_type_t coll_xfer_type = _params.collectives[i];
        size_t msg_thresh = get_msg_thresh(byte_thresh, coll_xfer_type, _ntasks);
        coll_mem_alloc(&coll[0], coll_xfer_type, msg_thresh, _ntasks);
        size_t col_num_algo[2];

        // Query the 
        int rc = query_geometry_algorithms_num(new_geometry,
                                               coll_xfer_type,
                                               col_num_algo);
        rc |= query_geometry_algorithms(client,
                                        contexts[0],
                                        new_geometry,
                                        coll_xfer_type,
                                        col_num_algo,
                                        col_algo,
                                        col_md,
                                        q_col_algo,
                                        q_col_md);

        // Query an always works barrier algorithm
        rc |= query_geometry_algorithm_aw(client, contexts[0], new_geometry, PAMI_XFER_BARRIER,  &(coll[1]));
        // Query an always works reduce algorithm
        rc |= query_geometry_algorithm_aw(client, contexts[0], new_geometry, PAMI_XFER_REDUCE, &(coll[2]));

        if(!_task_id) printf("# Collective: %-20s   Geometry Size: %-8zu \n",
                 xfer_type_str[_params.collectives[i]], geo_size);
        if(!_task_id) printf("# -------------------------------------------------------\n");

        size_t num_algo = col_num_algo[0] + col_num_algo[1];

        // Loop on message size
        for(k=0; k<_params.num_message_sizes && _params.message_sizes[k] <= msg_thresh; k++)
        {
          size_t msg_size = MIN(_params.message_sizes[k], msg_thresh);

          if(!_task_id) printf("# Algorithm                           Message Size    Min (usec)        Max (usec)        Avg (usec)\n");
          if(!_task_id) printf("# ---------                           ------------    ----------        ----------        ----------\n");

          //loop on algorithms
          for(l=0; l<num_algo; l++)
          {
            size_t low, high;
            char *algo_name = l<col_num_algo[0]?col_md[l].name:q_col_md[l-col_num_algo[0]].name;
            metadata_result_t result = {0};
            fill_coll(client, contexts[0], coll, coll_xfer_type, &low, &high, l, msg_size, _task_id, _ntasks, root,
                      col_num_algo, &result, col_algo, col_md, q_col_algo, q_col_md);
            algo_list[l].algo = coll[0].algorithm;

            if(msg_size <low || msg_size > high)
            {
              if(!_task_id) printf("  %s skipped as message size %zu is not in range (%zu-%zu)\n", algo_name, msg_size, low, high);
              algo_list[l].times[0] = DOUBLE_DIG;
              algo_list[l].times[1] = DOUBLE_DIG;
              algo_list[l].times[2] = DOUBLE_DIG;
              continue;
            }
            if(result.bitmask)
            {
              algo_list[l].times[0] = DOUBLE_DIG;
              algo_list[l].times[1] = DOUBLE_DIG;
              algo_list[l].times[2] = DOUBLE_DIG;
              continue;
            }

            //run benchmark 
            bench_setup bench[1];
            bench[0].xfer    = coll_xfer_type;
            bench[0].bytes   = msg_size;
            bench[0].np      = geo_size;
            bench[0].task_id = local_task_id;
            bench[0].data_check = _params.verify;
            bench[0].iters   = _params.iter;
            bench[0].verbose = _params.verbose;
            bench[0].isRoot  = (local_task_id == root)?1:0;
            measure_collective(contexts[0], coll, bench);
            memcpy(algo_list[l].times, bench[0].times, 3*sizeof(double));
            if(!_task_id)
              printf("  %-35s %-15zu %-17f %-17f %-17f\n", algo_name, msg_size, algo_list[l].times[0], algo_list[l].times[1], algo_list[l].times[2]);
          }
          //sort algo_list and write to file
        }
        //a bunch of free here
        release_coll(coll, coll_xfer_type);
        // Destroy the sub geometry
        if(new_geometry != world_geometry)
          destroy_geometry(client, contexts[0], &new_geometry);
      }//end geometry size loop
      blocking_coll(contexts[0], &barrier, &barrier_poll_flag);
    }//end collective loop
  }

  return PAMI_SUCCESS;
}

inline pami_result_t AdvisorTable::load(char* filename)
{
  return PAMI_SUCCESS;
}

inline pami_result_t AdvisorTable::unload()
{
  return PAMI_SUCCESS;
}

}

#endif // __api_extension_c_collsel_Extension_h__
