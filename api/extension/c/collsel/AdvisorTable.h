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
#define COLLSEL_MAX_NUM_MSG_SZ 64 // Maximum number of message sizes
// check statement
#define CRC(STATE, MSG) { \
  if (STATE) { \
    printf("ERR: failed to %s\n", (MSG)); \
    exit(-1); \
  } \
} 

const char *TMP_OUTPUT_PREFIX="./collsel";
const size_t SSIZE_T=sizeof(size_t);
const size_t SDOUBLE=sizeof(double);

#include "CollselData.h"
#include "CollselXMLWriter.h"

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
      void sort_algo_list(sorted_list* algo_list, AlgoList currAlgoList, int length);

      Advisor &_advisor;
      advisor_params_t _params;
      pami_task_t      _task_id;
      size_t           _ntasks;
      bool             _free_geometry_sz;
      bool             _free_message_sz;
      CollselData      _collsel_data;
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

  int cmp_by_time(const void *a, const void *b) 
{ 
    sorted_list *ia = (sorted_list *)a;
    sorted_list *ib = (sorted_list *)b;
    return (int)(100.f*ia->times[2] - 100.f*ib->times[2]);
	/* float comparison: returns negative if b > a 
	and positive if a > b. We multiplied result by 100.0
	to preserve decimal fraction */ 
 
} 

  inline void AdvisorTable::sort_algo_list(sorted_list* algo_list, AlgoList currAlgoList, int length)
  {
     int tmp_sorted_list[COLLSEL_MAX_ALGO];
     memset(tmp_sorted_list, -1, sizeof(int)*COLLSEL_MAX_ALGO);
     qsort(algo_list, length, sizeof(sorted_list), cmp_by_time);
     int i;
     for(i = 0; i < length*2; i+=2)
     {
       tmp_sorted_list[i]    = algo_list[i].algo_id;
       currAlgoList[i]   = tmp_sorted_list[i] + '0';
       if(i+1 < (length*2 - 1))currAlgoList[i+1] = ',';
     }
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
        _params.geometry_sizes = (size_t *) malloc(_params.num_geometry_sizes * SSIZE_T);
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
        _params.geometry_sizes = (size_t *) malloc(_params.num_geometry_sizes * SSIZE_T);

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
      _params.message_sizes = (size_t *) malloc(_params.num_message_sizes*SSIZE_T);

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

    /* XML and CollselData related */
    GeometryShapeMap     &ppn_map       = _collsel_data.get_datastore();
    AlgoMap              *algo_map      = _collsel_data.get_algorithm_map();
    AlgoNameToIdMap      *algo_name_map = _collsel_data.get_algorithm_name_map(); /* Used to get correct ids for algorithm based on their names */
    AlgoMap              *tmp_algo_map  = NULL;
    GeometrySizeMap      *geo_map       = NULL;
    CollectivesMap       *coll_map      = NULL;
    MessageSizeMap       *msg_map       = NULL;
    AlgoList              currAlgoList  = NULL;
    AlgoList              prevAlgoList  = NULL;
    unsigned              algo_ids[PAMI_XFER_COUNT];
    memset(algo_ids, 0, sizeof(unsigned) * PAMI_XFER_COUNT);
    //algo_map[PAMI_XFER_BROADCAST].insert(Algo_Pair(0,0x1234567));


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

    // tmp output file name
    char   tmp_output_fname[128];
    FILE  *tFile;

    if(mode == 0){
      //read in the file
    }
    else if(mode == 1)
    {
      size_t psize, gsize, csize, msize, algo, algo_name_len, i, j, flen;
      size_t act_msg_size[COLLSEL_MAX_NUM_MSG_SZ];
      size_t act_num_msg_size = 0;
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

      // loop on each setting of procs_per_node
      for (psize = 0; psize < _params.num_procs_per_node; psize ++) {

        /* Inserting geo_map in ppn_map with key _params.procs_per_node[psize] */
        geo_map         = &ppn_map[_params.procs_per_node[psize]];
        // Loop on geometry size
        for(gsize = 0; gsize < _params.num_geometry_sizes; gsize ++) {
          PAMI_assertf(_params.geometry_sizes[gsize] <= _ntasks, 
              "Geometry size (%zu) exceeds the number of tasks (%zu)", 
              _params.geometry_sizes[gsize], _ntasks);

          size_t geo_size =  _params.geometry_sizes[gsize];
          /* Inserting coll_map in geo_map with key geo_size */
          coll_map        = &(*geo_map)[geo_size];

          // Create subgeometry
          size_t geo_ntasks = _ntasks;
          pami_task_t local_task_id = _task_id;
          pami_task_t root = 0;
          pami_geometry_t new_geometry = world_geometry;

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

          // Loop on each collective
          for(csize = 0; csize < _params.num_collectives; csize ++) {

            //query algo for barrier, collective and reduce
            pami_xfer_t coll[3];

            pami_xfer_type_t coll_xfer_type = _params.collectives[csize];
            /* Inserting msg_map in coll_map with key coll_xfer_type */
            msg_map = &(*coll_map)[coll_xfer_type];
            /* The algolists will be the sorted list */
            currAlgoList = prevAlgoList = NULL;
            /* we will set the names, pami_algorithm_t and ids (key) in the algomap */
            tmp_algo_map = &algo_map[coll_xfer_type];
            size_t msg_thresh = get_msg_thresh(byte_thresh, coll_xfer_type, geo_size);
            coll_mem_alloc(&coll[0], coll_xfer_type, msg_thresh, geo_size);
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

            if(!_task_id) {
              printf("# Collective: %-20s   Geometry Size: %-8zu \n",
                     xfer_type_str[_params.collectives[csize]], geo_size);
              printf("# -------------------------------------------------------\n");
            }

            size_t num_algo = col_num_algo[0] + col_num_algo[1];

            // Loop on message size
            for(msize = 0; msize < _params.num_message_sizes && _params.message_sizes[msize] <= msg_thresh; msize ++) {
              act_msg_size[msize] = MIN(_params.message_sizes[msize], msg_thresh);

              if(!_task_id) {
                printf("# Algorithm                           Message Size    Min (usec)        Max (usec)        Avg (usec)\n");
                printf("# ---------                           ------------    ----------        ----------        ----------\n");
              }

              sprintf(tmp_output_fname, "%s_%zu_%zu_%d_%zu",
                  TMP_OUTPUT_PREFIX,
                  _params.procs_per_node[psize],
                  _params.geometry_sizes[gsize],
                  _params.collectives[csize],
                  act_msg_size[msize]);

              tFile = fopen(tmp_output_fname, "r");

              if (tFile != NULL) {
                if (!_task_id) {
                  // when the tmp output file exists
                  size_t tmp_num_algo, tmp_algo, tmp_algo_name_len;
                  char tmp_algo_name[128];
                  double tmp_results[3];

                  flen = fread(&tmp_num_algo, 1, SSIZE_T, tFile);
                  CRC(flen != SSIZE_T, "read tmp_num_algo");

                  PAMI_assertf(tmp_num_algo == num_algo, 
                      "Algorithm number mismatched.\n");

                  for (i = 0; i < tmp_num_algo; i ++) {
                    flen = fread(&tmp_algo, 1, SSIZE_T, tFile); 
                    CRC(flen != SSIZE_T, "read tmp_algo");

                    flen = fread(&tmp_algo_name_len, 1, SSIZE_T, tFile); 
                    CRC(flen != SSIZE_T, "read tmp_algo_name_len");

                    flen = fread(tmp_algo_name, 1, tmp_algo_name_len, tFile);
                    CRC(flen != tmp_algo_name_len, "read tmp_algo_name");
                    if(algo_name_map[coll_xfer_type].find(tmp_algo_name) == algo_name_map[coll_xfer_type].end())
                    {
                      algo_name_map[coll_xfer_type][tmp_algo_name] = algo_ids[coll_xfer_type]++;
                    }
                    (*tmp_algo_map)[algo_name_map[coll_xfer_type].find(tmp_algo_name)->second].algorithm_name = tmp_algo_name;
                    algo_list[i].algo_name = tmp_algo_name;
                    algo_list[i].algo_id   = algo_name_map[coll_xfer_type].find(tmp_algo_name)->second;

                    for (j = 0; j < 3; j ++) {
                      flen = fread(&tmp_results[j], 1, SDOUBLE, tFile); 
                      CRC(flen != SDOUBLE, "read tmp_results");

                    }
                    memcpy(algo_list[i].times, tmp_results, 3*SDOUBLE);
                    /*
                    printf("READ of (%s): %zu of %zu record: algo(%zu) algo.name.len(%zu) algo.name(%s) results(%6f:%6f:%6f)\n",
                            tmp_output_fname,
                            i, tmp_num_algo, tmp_algo, tmp_algo_name_len,
                            tmp_algo_name,
                            tmp_results[0], 
                            tmp_results[1], 
                            tmp_results[2]);
                    */

                    // these tmp_ data can be put into the xml
                    // file now
                  }
                }

                fclose(tFile);

              } else {
                if(!_task_id) {
                  // when the tmp output file doesn't exist
                  tFile = fopen(tmp_output_fname, "w");
                  CRC(tFile == NULL, "create tmp output file");

                  // write the number of algorithms into the
                  // beginning of the tmp output file
                  flen = fwrite(&num_algo, 1, SSIZE_T, tFile);
                  CRC(flen != SSIZE_T, "write num_algo");
                }

                //loop on algorithms
                for(algo = 0; algo < num_algo; algo ++)
                {
                  size_t low, high;
                  char *algo_name =
                    algo<col_num_algo[0]?col_md[algo].name:q_col_md[algo-col_num_algo[0]].name;


                  metadata_result_t result = {0};
                  fill_coll(client, contexts[0], coll, coll_xfer_type, &low, &high,
                      algo, act_msg_size[msize], _task_id, geo_size, root,
                      col_num_algo, &result, col_algo, col_md, q_col_algo, q_col_md);

                  if(algo_name_map[coll_xfer_type].find(algo_name) == algo_name_map[coll_xfer_type].end())
                  {
                    algo_name_map[coll_xfer_type][algo_name] = algo_ids[coll_xfer_type]++;
                  }
                  /* tmp_algo_map when filled, it is used by XML writer to write key data for algorithms
                     the algo_list on the other hand is used to sort the data */
                  (*tmp_algo_map)[algo_name_map[coll_xfer_type].find(algo_name)->second].algorithm_name = algo_name;
                  (*tmp_algo_map)[algo_name_map[coll_xfer_type].find(algo_name)->second].algorithm      = coll[0].algorithm;
                  algo_list[algo].algo      = coll[0].algorithm;
                  algo_list[algo].algo_name = algo_name;
                  algo_list[algo].algo_id   = algo_name_map[coll_xfer_type].find(algo_name)->second;
				  
                  if(act_msg_size[msize] <low || act_msg_size[msize] > high)
                  {
                    if(!_task_id) printf(
                        "  %s skipped as message size %zu is not in range (%zu-%zu)\n", algo_name,
                        act_msg_size[msize], low, high);
                    algo_list[algo].times[0] = DOUBLE_DIG;
                    algo_list[algo].times[1] = DOUBLE_DIG;
                    algo_list[algo].times[2] = DOUBLE_DIG;
                    continue;
                  }
                  if(result.bitmask)
                  {
                    algo_list[algo].times[0] = DOUBLE_DIG;
                    algo_list[algo].times[1] = DOUBLE_DIG;
                    algo_list[algo].times[2] = DOUBLE_DIG;
                    continue;
                  }

                  //run benchmark 
                  bench_setup bench[1];
                  bench[0].xfer    = coll_xfer_type;
                  bench[0].bytes   = act_msg_size[msize];
                  bench[0].np      = geo_size;
                  bench[0].task_id = local_task_id;
                  bench[0].data_check = _params.verify;
                  bench[0].iters   = _params.iter;
                  bench[0].verbose = _params.verbose;
                  bench[0].isRoot  = (local_task_id == root)?1:0;
                  measure_collective(contexts[0], coll, bench);
                  memcpy(algo_list[algo].times, bench[0].times, 3*SDOUBLE);

                  if(!_task_id) {
                    printf("  %-35s %-15zu %-17f %-17f %-17f\n", algo_name,
                        act_msg_size[msize], algo_list[algo].times[0],
                        algo_list[algo].times[1], algo_list[algo].times[2]);

                    // write the output of the current algorithm into the tmp output file
                    flen = fwrite(&algo, 1, SSIZE_T, tFile);
                    CRC(flen != SSIZE_T, "write algo");

                    algo_name_len = strlen(algo_name);
                    flen = fwrite(&algo_name_len, 1, SSIZE_T, tFile);
                    CRC(flen != SSIZE_T, "write algo_name_len");

                    flen = fwrite(algo_name, 1, algo_name_len, tFile);
                    CRC(flen != algo_name_len, "write algo_name");

                    for (i = 0; i < 3; i ++) {
                      flen = fwrite(&(algo_list[algo].times[i]), 1, SDOUBLE, tFile);
                      CRC(flen != SDOUBLE, "write results");
                    }

                    // also write these result into the structure
                  }

                } // end of algorithm loop


                if(!_task_id) {
                  // close the tmp output file
                  fclose(tFile);
                }
              }
              /* We have currAlgoList and prevAlgoList to compare and set message min and max ranges in xml */
              if(currAlgoList == NULL)
                currAlgoList = (AlgoList)malloc((sizeof(char) * num_algo * 2)); 
              sort_algo_list(algo_list, currAlgoList, num_algo);
              if(prevAlgoList == NULL) /* This means this is the first time in the message size loop */
              {
                prevAlgoList = (AlgoList)malloc((sizeof(char) * num_algo * 2));
                strcpy((char *)prevAlgoList, (const char *)currAlgoList);
                (*msg_map)[act_msg_size[msize]] = currAlgoList;
              }
              else
              {
                if(strcmp((const char *)currAlgoList, (const char *)prevAlgoList) != 0)
                {
                  (*msg_map)[act_msg_size[msize-1]] = prevAlgoList;
                  (*msg_map)[act_msg_size[msize]]   = currAlgoList;
                  strcpy((char *)prevAlgoList, (const char *)currAlgoList);
                }
              }
            } // end of message size loop
            free(currAlgoList);
            currAlgoList = NULL;
            free(prevAlgoList);
            prevAlgoList = NULL;

            // store the actual number of message sizes
            act_num_msg_size = msize;

            //a bunch of free here
            release_coll(coll, coll_xfer_type);
            // Destroy the sub geometry
          } // end collectives loop
          if(new_geometry != world_geometry)
            destroy_geometry(client, contexts[0], &new_geometry);
          blocking_coll(contexts[0], &barrier, &barrier_poll_flag);
        } // end geometry size loop
      } // end procs_per_node loop

      if(!_task_id)
      {
        PAMI::XMLWriter<>  xml_creator;
        xml_creator.write_xml("test_one.xml", ppn_map, algo_map);
      }

      if(!_task_id) {
        // since all the interations passed with no issue
        // remove all the tmp output files
        for (psize = 0; psize < _params.num_procs_per_node; psize ++) {
          for(gsize = 0; gsize < _params.num_geometry_sizes; gsize ++) {
            for(csize = 0; csize < _params.num_collectives; csize ++) {
              for(msize = 0; msize < act_num_msg_size; msize ++) {
                sprintf(tmp_output_fname, "%s_%zu_%zu_%d_%zu",
                    TMP_OUTPUT_PREFIX,
                    _params.procs_per_node[psize],
                    _params.geometry_sizes[gsize],
                    _params.collectives[csize],
                    act_msg_size[msize]);
                /*
                printf("about to delete file (%s) with %zu:%zu:%zu\n",
                        tmp_output_fname,
                        psize, 
                        _params.num_procs_per_node,
                        _params.procs_per_node[psize]);
                */
                if (remove(tmp_output_fname) != 0) {
                  printf("Error deleting file %s\n",
                      tmp_output_fname);
                }
              }
            }
          }
        }
      }
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
