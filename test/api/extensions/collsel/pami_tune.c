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
 * \file api/extensions/collsel/pami_tune.c
 * \user parameter proces to the the PAMI collsel extension
 */

#include <stdio.h>
#include <pami.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>


typedef void* advisor_t;
typedef void* advisor_table_t;
typedef enum {
  dummy,
} advisor_attribute_name_t;

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
   pami_xfer_type_t  *collectives;        /** List of collectives to benchmark */
   size_t             num_collectives;    /** Size of collectives list */
   size_t            *geometry_sizes;     /** List of geometry sizes */
   size_t             num_geometry_sizes; /** Size of geometry list */
   size_t            *message_sizes;      /** List of message sizes */
   size_t             num_message_sizes;  /** Message sizes list size */
   int		      iter;
   int                verify;
   int                verbose;
} advisor_params_t;

typedef pami_result_t (*pami_extension_collsel_init) (pami_client_t, advisor_configuration_t [], size_t, \
					            pami_context_t [], size_t, advisor_t *);
typedef pami_result_t (*pami_extension_collsel_table_generate) (advisor_t, char *, advisor_params_t *, int);
typedef pami_result_t (*pami_extension_collsel_destroy) (advisor_t *);

int     xfer_count = PAMI_XFER_COUNT;
pami_xfer_type_t xfer_array[PAMI_XFER_COUNT];
const char * xfer_array_str[PAMI_XFER_COUNT];
#define MAX_OP_STR 14
#define MIN_OP_STR 4

void print_param(advisor_params_t *params);

void init_advisor_params(advisor_params_t *params)
{
  params->num_collectives = 0;
  params->num_geometry_sizes = 0;
  params->num_message_sizes = 0;
  params->collectives = NULL;
  params->geometry_sizes = NULL;
  params->message_sizes = NULL;
}

void free_advisor_params(advisor_params_t *params)
{
  free(params->collectives);
  free(params->geometry_sizes);
  free(params->message_sizes);
}

void init_xfer_tables()
{
  xfer_array_str[PAMI_XFER_BROADCAST]="bcast";
  xfer_array_str[PAMI_XFER_ALLREDUCE]="allreduce";
  xfer_array_str[PAMI_XFER_REDUCE]="reduce";
  xfer_array_str[PAMI_XFER_ALLGATHER]="allgather";
  xfer_array_str[PAMI_XFER_ALLGATHERV]="allgatherv";
  xfer_array_str[PAMI_XFER_ALLGATHERV_INT]="allgatherv_int";
  xfer_array_str[PAMI_XFER_SCATTER]="scatter";
  xfer_array_str[PAMI_XFER_SCATTERV]="scatterv";
  xfer_array_str[PAMI_XFER_SCATTERV_INT]="scatterv_int";
  xfer_array_str[PAMI_XFER_GATHER]="gather";
  xfer_array_str[PAMI_XFER_GATHERV]="gatherv";
  xfer_array_str[PAMI_XFER_GATHERV_INT]="gatherv_int";
  xfer_array_str[PAMI_XFER_BARRIER]="barrier";
  xfer_array_str[PAMI_XFER_ALLTOALL]="alltoall";
  xfer_array_str[PAMI_XFER_ALLTOALLV]="alltoallv";
  xfer_array_str[PAMI_XFER_ALLTOALLV_INT]="alltoallv_int";
  xfer_array_str[PAMI_XFER_SCAN]="scan";
  xfer_array_str[PAMI_XFER_REDUCE_SCATTER]="reduce_scatter";
  xfer_array_str[PAMI_XFER_AMBROADCAST]="ambcast";
  xfer_array_str[PAMI_XFER_AMSCATTER]="amscatter";
  xfer_array_str[PAMI_XFER_AMGATHER]="amgather";
  xfer_array_str[PAMI_XFER_AMREDUCE]="amreduce";
}



void process_collective(char *ovalue, advisor_params_t *params)
{
  size_t num_coll = params->num_collectives;
  int i;
  for(i=0; i<PAMI_XFER_COUNT; i++)
  {
    if(!strcmp(ovalue, xfer_array_str[i]))
    {
      pami_xfer_type_t *temp = NULL;
      if(num_coll == 0) 
      { 
        temp = (pami_xfer_type_t *)malloc(sizeof(pami_xfer_type_t));
      }
      else 
      {
        temp = (pami_xfer_type_t *)realloc(params->collectives, (num_coll+1)*sizeof(pami_xfer_type_t));
      }
      if(temp != NULL)
      {
        params->collectives = temp;
	params->collectives[num_coll] = i;
	params->num_collectives++;
      }
      else
      {
        free(params->collectives);
	params->num_collectives = 0;
      }
      break;
    }
  }
}

void process_msg_sizes(int m_size, advisor_params_t *params, int *msg_full_flag)
{
  size_t num_msg = params->num_message_sizes;
  if(m_size == 0)
  {
    *msg_full_flag = 1;  
  }
  else if(m_size > 0)
  {
    size_t *temp;
    if( num_msg == 0)
    {
      temp = (size_t *) malloc(sizeof(size_t));
    }
    else 
    {
      temp = (size_t *)realloc(params->message_sizes, (num_msg+1)*sizeof(size_t));	
    }
    if(temp != NULL)
    {
      params->message_sizes = temp;
      params->message_sizes[num_msg] = m_size;
      params->num_message_sizes++;
    }
    else 
    {
      if(params->message_sizes) free(params->message_sizes);
      params->message_sizes = 0;
    }
  } 
}

void process_geo_sizes(int g_size, advisor_params_t *params, int *geo_full_flag)
{
  size_t num_geo = params->num_geometry_sizes; 
  if(g_size == 0)
  {
     *geo_full_flag = 1;  
  } 
  else if(g_size > 1)
  {
    size_t * temp;	
    if(num_geo == 0)
    {
      temp = (size_t *) malloc(sizeof(size_t));
    }
    else 
    {
      temp = (size_t *)realloc(params->geometry_sizes, (num_geo+1)*sizeof(size_t));	
    }
    if(temp != NULL)
    {
      params->geometry_sizes = temp;
      params->geometry_sizes[num_geo] = g_size;
      params->num_geometry_sizes++;
    }
    else 
    {
      free(params->geometry_sizes);
      params->geometry_sizes = 0;
    }
  } 
}

int process_file(const char *filename, advisor_params_t *params, int *msg_full_flag, int *geo_full_flag)
{
  FILE *fr = fopen(filename, "r");
  if(!fr)
  {
    fprintf(stderr, "Error. Can't open file %s\n", filename);
    return 1;
  }
  char line[10000];
 
  //process collectives
  if(fgets(line, 10000, fr)!=NULL)
  {
    char * pch;
    pch = strtok (line," ,\n");
    while(pch != NULL)
    {
      process_collective(pch, params); 
      pch = strtok(NULL, " ,\n");
    }
  }
  
  //process geo sizes
  if(fgets(line, 10000, fr)!=NULL)
  {
    char * pch;
    pch = strtok (line," ,\n");
    while(pch != NULL)
    {
      int g_size = atoi(pch); 
      process_geo_sizes(g_size, params, geo_full_flag); 
      pch = strtok(NULL, " ,\n");
    }
  }

  //process msg sizes
  if(fgets(line, 10000, fr)!=NULL)
  {
    char * pch;
    pch = strtok (line," ,\n");
    while(pch != NULL)
    {
      int m_size = atoi(pch); 
      process_msg_sizes(m_size, params, msg_full_flag); 
      pch = strtok(NULL, " ,\n");
    }
  }

  //process iteration
  if(fgets(line, 10000, fr)!=NULL)
  {
    sscanf(line, "%d", &params->iter);
  }

  //process verify
  if(fgets(line, 10000, fr)!=NULL)
  {
    sscanf(line, "%d", &params->verify);
  }

  //process verbose
  if(fgets(line, 10000, fr)!=NULL)
  {
    sscanf(line, "%d", &params->verbose);
  }
  fclose(fr);

  return 0;
}


int print_usage()
{
 printf("usage: pami_tune -o -m -g -i -f -v\n"); 
 return 0;
}

int process_arg(int argc, char *argv[], advisor_params_t *params)
{ 
   int m_size, g_size;
  
   int geo_full_flag = 0;
   int msg_full_flag = 0;

   char ovalue[MAX_OP_STR];
   char filename[100];
   int c;

   opterr = 0;
   init_xfer_tables();
   params->verify = 0;
   while ((c = getopt (argc, argv, "o:m:g:f:i:v::c::")) != -1)
   {
     switch (c)
     {
       case 'o':
	 if((strlen(optarg)> MAX_OP_STR) || (strlen(optarg)< MIN_OP_STR))
	   break;
	 strcpy(ovalue, optarg);
	 process_collective(ovalue, params);
         break;
       case 'm':
	 m_size = atoi(optarg);
	 process_msg_sizes(m_size, params, &msg_full_flag);
         break;
       case 'g':
	 g_size = atoi(optarg);
	 process_geo_sizes(g_size, params, &geo_full_flag);
	 break;
       case 'f':
         strcpy(filename, optarg);
	 process_file(filename, params, &msg_full_flag, &geo_full_flag);
	 break;
       case 'i':
	 params->iter = atoi(optarg);
	 if(params->iter <= 0)
	   params->iter = 0;
	 break;
       case 'c':
	 params->verify = 1;
	 break;
       case 'v':
	 params->verbose = 1;
	 break;
       case '?':
	 print_usage();
	 break;
       default:
	 printf("default %s\n", optarg);
	 break;
     }
   }

   //full test, clean up single tests
   if(geo_full_flag && msg_full_flag)
   {
     if(params->geometry_sizes)free(params->geometry_sizes);
     if(params->message_sizes)free(params->message_sizes);
     params->geometry_sizes = (size_t *)malloc(sizeof(size_t));
     params->geometry_sizes[0] = 0;
     params->num_geometry_sizes = 1;
     params->message_sizes= (size_t *)malloc(sizeof(size_t));
     params->message_sizes[0] = 0;
     params->num_message_sizes = 1;
   } 
   //wrong paramters
   else if((!geo_full_flag && msg_full_flag) || (geo_full_flag && !msg_full_flag))
   {
     init_advisor_params(params);
     return 1;
   }
   else 
   {
     //wrong parameters
     if(!params->collectives || !params->geometry_sizes || !params->message_sizes)
     {
       init_advisor_params(params);
       return 1;
     }
   }
 
   return 0;
}

void print_param(advisor_params_t *params)
{
  size_t i;
  printf("number of collective %zu %p\n", params->num_collectives, params->collectives); 
  for(i=0; i<params->num_collectives; i++){
    printf("%d ", params->collectives[i]);
  }
  printf("\n number of geometry sizes %zu %p\n", params->num_geometry_sizes, params->geometry_sizes);
  for(i=0; i<params->num_geometry_sizes; i++){
    printf("%zu ", params->geometry_sizes[i]);
  }
  printf("\n number of message sizes %zu %p\n", params->num_message_sizes, params->message_sizes);
  for(i=0; i<params->num_message_sizes; i++){
    printf("%zu ", params->message_sizes[i]);
  }
   

}

int main(int argc, char ** argv)
{
  pami_client_t client;
  pami_context_t context;
  pami_result_t status = PAMI_ERROR;

  status = PAMI_Client_create("TEST", &client, NULL, 0);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", status);
    return 1;
  }

  status = PAMI_Context_createv(client, NULL, 0, &context, 1);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", status);
    return 1;
  }

  pami_extension_t extension;
  PAMI_Extension_open (client, "EXT_collsel", &extension);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. The \"EXT_collsel\" extension is not turned on. result = %d\n", status);
    return 1;
  }

  advisor_t advisor;
  advisor_configuration_t configuration[1];
  pami_extension_collsel_init pamix_collsel_init = 
    (pami_extension_collsel_init) PAMI_Extension_symbol (extension, "Collsel_init_fn");
  pamix_collsel_init (client, configuration, 1, &context, 1, &advisor);

  advisor_params_t params;
  init_advisor_params(&params);
  int result = process_arg(argc, argv, &params);
  if(result)
  {
    fprintf (stderr, "Error. Wrong EXT_collsel arguments\n");
    print_usage();
    return 1;
  }

  //do a bcast on 16 np 1000 bytes
  //params.collectives = (pami_xfer_type_t *)malloc(sizeof(pami_xfer_type_t));
  //params.collectives[0] = PAMI_XFER_BROADCAST;
  //params.num_collectives = 1;
  //params.geometry_sizes = (size_t *)malloc(sizeof(size_t));
  //params.geometry_sizes[0] = 10;
  //params.num_geometry_sizes = 1;
  //params.message_sizes = (size_t *)malloc(sizeof(size_t));
  //params.message_sizes[0] = 1000;
  //params.num_message_sizes = 1;
  pami_extension_collsel_table_generate pamix_collsel_table_generate = 
    (pami_extension_collsel_table_generate) PAMI_Extension_symbol (extension, "Collsel_table_generate_fn");
  status = pamix_collsel_table_generate (advisor, NULL, &params, 1);

  pami_extension_collsel_destroy pamix_collsel_destroy =
    (pami_extension_collsel_destroy) PAMI_Extension_symbol (extension, "Collsel_destroy_fn");
  status = pamix_collsel_destroy (advisor);


  status = PAMI_Extension_close (extension);
  if(status != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. The \"EXT_torus_network\" extension could not be closed. result = %d\n", status);
    return 1;
  }


  status = PAMI_Context_destroyv(&context, 1);
  if(status != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy pami context. result = %d\n", status);
    return 1;
  }

  status = PAMI_Client_destroy(&client);
  if(status != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", status);
    return 1;
  }

  return 0;
}

