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

unsigned gVerbose = 1;    /* Global verbose flag, some tests set with TEST_VERBOSE=n */

#include<assert.h>
#include <pami.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  assert(array);
  array[0]     = (size_t *)calloc(nrows * ncols, sizeof(size_t));
  assert(array[0]);

  for (i = 1; i < nrows; i++)
      array[i]   = array[0] + i * ncols;

  return array;
}

void init_tables()
{
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
  init_tables();
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


#ifndef FULL_TEST
  #define FULL_TEST 0  /* default to 'short' test - only validated op/dt's,
                          define to 1 to run all dt/op's or use TEST_OP=ALL */
#endif
#ifndef COUNT
  #define COUNT      65536
#endif
#ifndef OFFSET
  #define OFFSET     0    
#endif
#ifndef NITERLAT
  #define NITERLAT   100    
#endif
#ifndef NITERBW
#define NITERBW    MIN(20, gNiterlat/100+1)
#endif
#ifndef CUTOFF
  #define CUTOFF     65536
#endif
          
#ifndef MAX
#define MAX(x, y)	(((x) > (y))?(x):(y))
#endif

unsigned gNumRoots       = -1; /* use num_tasks */
unsigned gFull_test      = FULL_TEST;
unsigned gMax_datatype_sz= 16; /* Assumed max datatype size */
unsigned gMax_byte_count = COUNT;
unsigned gMin_byte_count = 1;
unsigned gBuffer_offset  = OFFSET;
unsigned gNiterlat       = NITERLAT;
size_t   gNum_contexts   = 1;
size_t** gValidTable     = NULL;
unsigned gSelector       = 1;
char*    gSelected       ;
unsigned gParentless     = 1;
int      gOptimize       = -1; /* default, no de/optimize */

void setup_op_dt(size_t ** validTable,char* sDt, char* sOp);

void setup_env()
{


  /* \note Test environment variable" TEST_VERBOSE=N     */
  char* sVerbose = getenv("TEST_VERBOSE");

  if(sVerbose) gVerbose=atoi(sVerbose); /* set the global defined in coll_util.h */

  /* \note Test environment variable" TEST_PROTOCOL={-}substring.       */
  /* substring is used to select, or de-select (with -) test protocols */
  gSelected = getenv("TEST_PROTOCOL");

  if (!gSelected) gSelected = (char*)"";
  else if (gSelected[0] == '-')
  {
    gSelector = 0 ;
    ++gSelected;
  }

  /* \note Test environment variable" TEST_DT=pami datatype string or 'ALL' or 'SHORT' */
  char* sDt = getenv("TEST_DT");

  /* \note Test environment variable" TEST_OP=pami operation string or 'ALL' or 'SHORT'*/
  char* sOp = getenv("TEST_OP");

  /* Override FULL_TEST with 'ALL' or 'SHORT' */
  if (sDt || sOp) gFull_test = 0;
  if ((sDt && !strcmp(sDt, "ALL")) || (sOp && !strcmp(sOp, "ALL"))) gFull_test = 1;
  if ((sDt && !strcmp(sDt, "SHORT")) || (sOp && !strcmp(sOp, "SHORT"))) 
  {
    sDt = sOp = NULL; /* no selection will select default/minimal */
    gFull_test = 0;
  }

  /* \note Test environment variable" TEST_NUM_ROOTS=N max roots to test  */
  char* sNRoots = getenv("TEST_NUM_ROOTS");

  /* Override Number of Roots */
  if (sNRoots) gNumRoots = atoi(sNRoots);

  /* \note Test environment variable" TEST_BYTES=N max byte count     */
  char* sCount = getenv("TEST_BYTES");

  /* Override max byte COUNT */
  if (sCount) gMax_byte_count = atoi(sCount);

  /* \note Test environment variable" TEST_BYTES_ONLY=N only byte count     */
  char* sCountOnly = getenv("TEST_BYTES_ONLY");

  /* Override byte COUNT with single count */
  if (sCountOnly) gMin_byte_count = gMax_byte_count = atoi(sCountOnly);

  /* \note Test environment variable" TEST_OFFSET=N buffer offset/alignment*/
  char* sOffset = getenv("TEST_OFFSET");

  /* Override OFFSET */
  if (sOffset) gBuffer_offset = atoi(sOffset);

  /* \note Test environment variable" TEST_ITER=N iterations      */
  char* sIter = getenv("TEST_ITER");

  /* Override NITERLAT */
  if (sIter) gNiterlat = atoi(sIter);

  /* \note Test environment variable" TEST_NUM_CONTEXTS=N, defaults to 1.*/
  char* snum_contexts = getenv("TEST_NUM_CONTEXTS");

  if (snum_contexts) gNum_contexts = atoi(snum_contexts);

  /* Setup operation and datatype tables*/
  gValidTable = alloc2DContig(op_count, dt_count);
  setup_op_dt(gValidTable,sDt,sOp);

  /* \note Test environment variable" TEST_PARENTLESS=0 or 1, defaults to 1.
     0 - world_geometry is the parent
     1 - parentless                                                      */
  char* sParentless = getenv("TEST_PARENTLESS");

  if (sParentless) gParentless = atoi(sParentless);

  /* \note Test environment variable" TEST_OPTIMIZE=-1, 0, 1; defaults to -1.
     -1 = default optimization only
     0 - default, then deoptimize 
     1 - default, deoptimize, optimize                                    */
  char* sOptimize   = getenv("TEST_OPTIMIZE");

  if (sOptimize) gOptimize = atoi(sOptimize);


}

void setup_op_dt(size_t ** validTable,char* sDt, char* sOp)
{

  /* \todo We *could* change gMax_datatype_sz based on dt selected */

  int i,j;
  unsigned force = 0; /* don't force the dt/op selected */

  init_tables();

  if (gFull_test)
  {
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
        validTable[i][j] = 1;
      
  }
  else if (sDt && sOp)
  {
    force = 1; /* force the dt/op*/
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
          if (!strcmp(sDt, dt_array_str[j]) &&
              !strcmp(sOp, op_array_str[i]))
            validTable[i][j] = 1;
          else
            validTable[i][j] = 0;
  } 
  else if (sOp)
  {
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
        if (!strcmp(sOp, op_array_str[i]))
          validTable[i][j] = 1;
        else
          validTable[i][j] = 0;
  }
  else if (sDt)
  {
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
        if (!strcmp(sDt, dt_array_str[j]))
          validTable[i][j] = 1;
        else
          validTable[i][j] = 0;
  }
  else  /* minimal/default test */
  {
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
        validTable[i][j] = 0;
  
      validTable[OP_SUM][DT_SIGNED_INT] = 1;
      validTable[OP_MAX][DT_SIGNED_INT] = 1;
      validTable[OP_MIN][DT_SIGNED_INT] = 1;
      validTable[OP_SUM][DT_UNSIGNED_INT] = 1;
      validTable[OP_MAX][DT_UNSIGNED_INT] = 1;
      validTable[OP_MIN][DT_UNSIGNED_INT] = 1;
      validTable[OP_SUM][DT_DOUBLE] = 1;
      validTable[OP_MAX][DT_DOUBLE] = 1;
      validTable[OP_MIN][DT_DOUBLE] = 1;
  
  }
  if(!force) /* not forcing the op/dt*/
  {
    /*--------------------------------------*/
    /* Disable unsupported ops on complex   */
    /* Only sum, prod                       */
    for (i = 0, j = DT_SINGLE_COMPLEX; i < OP_COUNT; i++)if(i!=OP_SUM && i!=OP_PROD) validTable[i][j] = 0;
    for (i = 0, j = DT_DOUBLE_COMPLEX; i < OP_COUNT; i++)if(i!=OP_SUM && i!=OP_PROD) validTable[i][j] = 0; 
      
    /*--------------------------------------*/
    /* Disable NULL and byte operations     */
    for (i = 0, j = DT_NULL; i < OP_COUNT; i++) validTable[i][j] = 0;
    for (i = 0, j = DT_BYTE; i < OP_COUNT; i++) validTable[i][j] = 0;
    for (j = 0, i = OP_COPY; j < DT_COUNT; j++) validTable[i][j] = 0;
    for (j = 0, i = OP_NOOP; j < DT_COUNT; j++) validTable[i][j] = 0;
    
    /*--------------------------------------*/
    /* Disable non-LOC ops on LOC dt's      */
    for (i = 0, j = DT_LOC_2INT      ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_SHORT_INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_FLOAT_INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_DOUBLE_INT; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_2FLOAT    ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_2DOUBLE   ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
      
    /*--------------------------------------*/
    /* Disable LOC ops on non-LOC dt's      */
    for (j = 0, i = OP_MAXLOC; j < DT_LOC_2INT; j++) validTable[i][j] = 0;
    for (j = 0, i = OP_MINLOC; j < DT_LOC_2INT; j++) validTable[i][j] = 0;
      
    /*---------------------------------------*/
    /* Disable unsupported ops on logical dt */
    /* Only land, lor, lxor, band, bor, bxor */
    for (i = 0,         j = DT_LOGICAL; i < OP_LAND ; i++) validTable[i][j] = 0;
    for (i = OP_BXOR+1, j = DT_LOGICAL; i < OP_COUNT; i++) validTable[i][j] = 0;
    
    /*---------------------------------------*/
    /* Disable unsupported ops on long double*/
    /* Only max,min,sum,prod                 */
    for (i = OP_PROD+1, j = DT_LONG_DOUBLE; i < OP_COUNT; i++) validTable[i][j] = 0;
  }
}

void get_split_method(size_t *num_tasks,            /* input number of tasks/output number of task in subcomm*/
                      pami_task_t task_id,          /* input task id*/
                      int *rangecount,              /* output rangecount for geometry create */
                      pami_geometry_range_t *range, /* output range for geometry create */
                      pami_task_t *local_task_id,   /* output local task id in subcomm*/
                      size_t set[2],                /* output split set*/
                      int *id,                      /* output comm id*/
                      pami_task_t *root,            /* output root/task 0 in subcomm*/
                      int non_root[2])              /* output 2 non-root tasks (first and last) for barrier tests */
{
  size_t                 half        = *num_tasks / 2;
  char *method = getenv("TEST_SPLIT_METHOD");
    
  /* Default or TEST_SPLIT_METHOD=0 : divide in half */
  if ((!method || !strcmp(method, "0")))
  {
    if (task_id < half)
    {
      range[0].lo = 0;
      range[0].hi = half - 1;
      set[0]   = 1;
      set[1]   = 0;
      *id       = 1;
      *root     = 0;
      *num_tasks = half;
      *local_task_id = task_id;
      non_root[0]  = *root +1;      /* first non-root rank in the subcomm  */
      non_root[1]  = half-1;        /* last rank in the subcomm  */
    }
    else
    {
      range[0].lo = half;
      range[0].hi = *num_tasks - 1;
      set[0]   = 0;
      set[1]   = 1;
      *id       = 2;
      *root     = half;
      *local_task_id = task_id - *root;
      non_root[0] = *root +1;      /* first non-root rank in the subcomm  */
      non_root[1] = *num_tasks-1;  /* last rank in the subcomm  */
      *num_tasks = *num_tasks - half;
    }
  
    *rangecount = 1;
  }
  /* TEST_SPLIT_METHOD=-1 : alternate ranks  */
  else if ((method && !strcmp(method, "-1")))
  {
    unsigned i = 0;
    int iter = 0;;

    if ((task_id % 2) == 0)
    {
      for (i = 0; i < *num_tasks; i++)
      {
        if ((i % 2) == 0)
        {
          range[iter].lo = i;
          range[iter].hi = i;
          iter++;
        }
     }
  
      set[0]   = 1;
      set[1]   = 0;
      *id       = 1;
      *root     = 0;
      *rangecount = iter;
      non_root[0] = range[1].lo;      /* first non-root rank in the subcomm  */
      non_root[1] = range[iter-1].lo; /* last rank in the subcomm  */
    }
    else
    {
      for (i = 0; i < *num_tasks; i++)
      {
        if ((i % 2) != 0)
        {
          range[iter].lo = i;
          range[iter].hi = i;
          iter++;
        }
      }
      
      set[0]   = 0;
      set[1]   = 1;
      *id       = 2;
      *root     = 1;
      *rangecount = iter;
      non_root[0] = range[1].lo;      /* first non-root rank in the subcomm  */
      non_root[1] = range[iter-1].lo; /* last rank in the subcomm  */
    }
  
    *num_tasks = iter;
    *local_task_id = task_id/2;
  }
  /* TEST_SPLIT_METHOD=-2 : divide in half and reverse order the ranks */
  else if ((!method || !strcmp(method, "-2")))
  {
    int iter = 0;;
    if (task_id < half)
    {
      signed i = 0;
      for (i = half - 1; i >=0; i--)
      {
        range[iter].lo = i;
        range[iter].hi = i;
        if(task_id == (unsigned)i)
          *local_task_id = iter;
        iter++;
      }

      set[0]   = 1;
      set[1]   = 0;
      *id       = 1;
      *root     = half-1;
      *rangecount = iter;
      non_root[0] = range[1].lo;      /* first non-root rank in the subcomm  */
      non_root[1] = range[iter-1].lo; /* last rank in the subcomm  */
    }
    else
    {
      unsigned i = 0;
      for (i = *num_tasks - 1; i >=half; i--)
      {
        range[iter].lo = i;
        range[iter].hi = i;
        if(task_id == i)
          *local_task_id = iter;
        iter++;
      }
  
      set[0]   = 0;
      set[1]   = 1;
      *id       = 2;
      *root     = *num_tasks - 1;
      *rangecount = iter;
      non_root[0] = range[1].lo;      /* first non-root rank in the subcomm  */
      non_root[1] = range[iter-1].lo; /* last rank in the subcomm  */
    }
    *num_tasks = iter;
  }
  /* TEST_SPLIT_METHOD=N : Split the first "N" processes into a communicator */
  else
  {
    half = atoi(method);
    if(*num_tasks <= half)
    {
      fprintf(stderr, "assert(*num_tasks > half)");
      assert(*num_tasks > half);
    }
    if (task_id < half)
    {
      range[0].lo = 0;
      range[0].hi = half - 1;
      set[0]   = 1;
      set[1]   = 0;
      *id       = 1;
      *root     = 0;
      *num_tasks = half;
      *local_task_id = task_id;
      non_root[0] = *root +1;      /* first non-root rank in the subcomm  */
      non_root[1] = half-1;        /* last rank in the subcomm  */
    }
    else
    {
      range[0].lo = half;
      range[0].hi = *num_tasks - 1;
      set[0]   = 0;
      set[1]   = 1;
      *id       = 2;
      *root     = half;
      *local_task_id = task_id - *root;
      non_root[0] = *root +1;       /* first non-root rank in the subcomm  */
      non_root[1] = *num_tasks-1;   /* last rank in the subcomm  */
      *num_tasks = *num_tasks - half;
    }
  
    *rangecount = 1;
  }
  /*fprintf(stderr,"set %zu/%zu, root %u, num_tasks %zu, local_task_id %u, non_root[0] %d, non_root[1] %d, id %d\n",
          set[0],set[1],*root, *num_tasks, *local_task_id, non_root[0], non_root[1], *id);*/
  
}
void get_next_root(size_t num_tasks,             /* input number of tasks*/
                   pami_task_t *root)            /* input/output current/next root*/
{
  size_t                 half        = num_tasks / 2;
  char *method = getenv("TEST_SPLIT_METHOD");
    
  /* Default or TEST_SPLIT_METHOD=0 : divide in half */
  if ((!method || !strcmp(method, "0")))
  {
    if (*root < half)
    {
      *root = *root + 1;
      if(*root >= half) *root = 0;
    }
    else
    {
      *root = *root + 1;
      if(*root >= num_tasks) *root = half;
    }
  }
  /* TEST_SPLIT_METHOD=-1 : alternate ranks  */
  else if ((method && !strcmp(method, "-1")))
  {
    if ((*root % 2) == 0)
    {
      *root = *root + 2;
      if(*root >= num_tasks) *root = 0;
    }
    else
    {
      *root = *root + 2;
      if(*root >= num_tasks) *root = 1;
    }
  }
  /* TEST_SPLIT_METHOD=N : Split the first "N" processes into a communicator */
  else
  {
    half = atoi(method);
    if (*root < half)
    {
      *root = *root + 1;
      if(*root >= half) *root = 0;
    }
    else
    {
      *root = *root + 1;
      if(*root >= num_tasks) *root = half;
    }
  }
  /*  fprintf(stderr,"root %u, num_tasks %zu\n",
          *root, *num_tasks);
  */
}

#endif /* __test_api_init_h__ */
