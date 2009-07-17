/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common.h
 * \brief common routines for ccmi tests
 */

#ifndef __SYS_TESTS_COMMON_H__
#define __SYS_TESTS_COMMON_H__
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "ccmi_collectives.h"
#include <math.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <getopt.h>

//---------------------------------------------------------------------------------
// Global values to control the test.
//---------------------------------------------------------------------------------
//#define STRICT_CHECKING 1

int warmup_repetitions = 20; // default warmup
int repetitions = 50; // default performance loop

// Limit our buffer size
#define MAX_COUNT (20*1024*1024)

// Optionally limits some of the output with some criteria
//#define TERSE_OUTPUT if(rank == 0 || rank == 1)
#ifndef TERSE_OUTPUT
 #define TERSE_OUTPUT 
#endif

//#define TRACE_TEST(x)  if(rank == 0 || rank == 1) fprintf x
#ifndef TRACE_TEST
  #define TRACE_TEST(x)
#endif

//#define TRACE_TEST_VERBOSE(x)  if(rank == 0 || rank == 1) fprintf x
#ifndef TRACE_TEST_VERBOSE
  #define TRACE_TEST_VERBOSE(x)
#endif
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

char *argv0; // filename

CCMI_CollectiveProtocol_t              barrier_reg __attribute__((__aligned__(32))), local_barrier_reg __attribute__((__aligned__(32)));
CCMI_Barrier_Configuration_t           configuration;
CCMI_CollectiveRequest_t               request;
CM_Callback_t                        common_done;
CCMI_Consistency                       consistency;
CCMI_Geometry_t                        geometry;

volatile int                           done;

unsigned rank, size, count, bytes;
unsigned * srcbuf, * allocSrcBuf;
unsigned * dstbuf, * allocDstBuf;
unsigned * ranks;

CM_Dt type = CM_UNSIGNED_INT;
CM_Op op   = CM_SUM;

CCMI_Geometry_t *getGeometry (int comm)
{
  TRACE_TEST_VERBOSE((stderr,"%s:%s\n", argv0,__PRETTY_FUNCTION__));
  return &geometry;
}


void done_callback(void* cd, CM_Error_t *e)
{
  TRACE_TEST_VERBOSE((stderr,"%s:%s\n", argv0,__PRETTY_FUNCTION__));
  done=1;
}

void no_op(char* string)
{
  TRACE_TEST((stderr, string));
  if(rank == 0) fprintf(stderr, "NO OP\n");
  exit(0);
}

void usage(int ret)
{
  if(rank == 0) fprintf(stderr,
          "\nUsage: %s [--help] [--count N] [--repetitions N] [--double|--float|--integer] [--max|--sum|--prod]\n"
          "Where:\n"
          "\t--help           This help string\n"
          "\t--count N        Count for bandwidth testing (default %d)\n"
          "\t--repetitions N  Loop repetitions (default %d)\n"
          "\t--limit N        Limit the partition size to N (mostly for reduce root tests) (defaults to np)\n"
          "Datatype - one of:\n"
          "\t--integer (default)\n"
          "\t--double  (not implemented)\n"
          "\t--float   (not implemented)\n"
          "Operation - one of:\n"
          "\t--sum  (default)\n"
          "\t--prod (not implemented)\n"
          "\t--max  (not implemented)\n"
          ,
          argv0, count, repetitions);
  exit(ret);
}

void setup(int argc, char **argv, unsigned override_default_max_count = 0)
{
  TRACE_TEST_VERBOSE((stderr,"%s:%s\n", argv0,__PRETTY_FUNCTION__));
  consistency = CCMI_MATCH_CONSISTENCY;

  MPI_Init (&argc, &argv);

  CCMI_Collective_initialize();

  MPI_Comm_size(MPI_COMM_WORLD, (int*)&size);
  MPI_Comm_rank(MPI_COMM_WORLD, (int*)&rank);

  ranks = (unsigned *)malloc(size * sizeof(unsigned));
  assert ( ranks );

  static struct option lopts[] = {
    {"help",        no_argument,      NULL, 'h'},

    {"count",       required_argument,NULL, 'c'},
    {"repetitions", required_argument,NULL, 'r'},

    {"double",      no_argument,      NULL, 'd'},
    {"float",       no_argument,      NULL, 'f'},
    {"integer",     no_argument,      NULL, 'i'},

    {"max",         no_argument,      NULL, 'm'},
    {"prod",        no_argument,      NULL, 'p'},
    {"sum",         no_argument,      NULL, 's'},

    {NULL, 0, NULL, 0},
  };

//  extern int optind;
  extern char *optarg;
  int x;
  argv0 = argv[0];

  count = override_default_max_count? override_default_max_count:MAX_COUNT; // default count to max

  while((x = getopt_long(argc, argv,"dfchimps", lopts, NULL)) != EOF)
  {
    switch(x)
    {
    case 'c': // count
      count  = atoi(optarg);
      // Exceeding override_default_max_count is dangerous.  
      // Presumably the testcase knew something if it used this override (like short protocol limits)
      if((override_default_max_count) && (count > override_default_max_count))
        if(rank == 0) printf("%s: WARNING specified count(%d) exceeds override max count(%d)\n", argv0, count, override_default_max_count);
      break;
    case 'r': // repetitions
      repetitions  = atoi(optarg);
      break;
    case 'd':
      type = CM_DOUBLE;
      break;
    case 'i':
      type = CM_UNSIGNED_INT;
      break;
    case 'f':
      type = CM_FLOAT;
      break;
    case 'm':
      op = CM_MAX;
      break;
    case 'p':
      op = CM_PROD;
      break;
    case 's':
      op = CM_SUM;
      break;
    case 'h':
      if(rank == 0)usage(1);
      break;
    default:
      if(rank == 0)fprintf(stderr," Unknown %c\n",x);
      if(rank == 0)usage(1);
      break;
    }
  }

  char* tmp = strrchr(argv0, '/'); 
  if(tmp) argv0 = tmp+1;

  for(int i = 0; i < (int) size; i++) ranks[i] = i;

}


void initialize_common(CCMI_Barrier_Protocol barrier_protocol,
                CCMI_Barrier_Protocol lbarrier_protocol)
{

  TRACE_TEST_VERBOSE((stderr,"%s:%s\n", argv0,__PRETTY_FUNCTION__));

  if(size < 3) no_op("Generally unsupported partition size (too small)\n");

  configuration.protocol = barrier_protocol;
  configuration.cb_geometry = getGeometry;

  CCMI_Result ccmiResult;
  if((ccmiResult = (CCMI_Result) CCMI_Barrier_register(&barrier_reg, &configuration)) != CM_SUCCESS)
    if(rank == 0) fprintf(stderr,"CCMI_Barrier_register failed %d\n",ccmiResult);

  configuration.protocol = lbarrier_protocol;
  if((ccmiResult = (CCMI_Result) CCMI_Barrier_register(&local_barrier_reg, &configuration)) != CM_SUCCESS)
    if(rank == 0) fprintf(stderr,"CCMI_Barrier_register failed %d\n",ccmiResult);

  CCMI_CollectiveProtocol_t  * bar_p = & barrier_reg, * local_bar_p = & local_barrier_reg;
  if((ccmiResult = (CCMI_Result) CCMI_Geometry_initialize (&geometry, 0, ranks, size,
                                                           &bar_p, 1, 
                                                           &local_bar_p, 1, 
                                                           &request, 0, 1)) != CM_SUCCESS)
    if(rank == 0) fprintf(stderr,"CCMI_Geometry_initialize failed %d\n",ccmiResult);

  return;
}

void allocate_buffers()
{
  TRACE_TEST_VERBOSE((stderr,"%s:%s\n", argv0,__PRETTY_FUNCTION__));

  bytes = count * sizeof(unsigned);

  // Padded so we can put padding around data and check for out of bounds overflows
  allocSrcBuf = (unsigned *)malloc(bytes + 5*sizeof(unsigned)); 
  allocDstBuf = (unsigned *)malloc(bytes + 5*sizeof(unsigned)); 
  assert ( allocSrcBuf && allocDstBuf );
  srcbuf = allocSrcBuf + 4;
  dstbuf = allocDstBuf + 4;
  allocSrcBuf[0] = allocSrcBuf[1] = allocSrcBuf[2] = allocSrcBuf[3] = 
    allocDstBuf[0] = allocDstBuf[1] = allocDstBuf[2] = allocDstBuf[3] = -1;

  TRACE_TEST((stderr, "%s:  Number of Ranks:%d, Repetitions:%d, BwCount:%d\n", argv0,
              size,
              repetitions,
              count));


  return;
}

void print_performance(double latency_time, double bandwidth_time)
{
  TRACE_TEST_VERBOSE((stderr,"%s:%s\n", argv0,__PRETTY_FUNCTION__));
  if(latency_time == 0.0)
  {
    if(rank == 0) printf("%32.32s \t\tNP\t       \t\tBandWidth\tbytes\n", argv0);
    if(rank == 0) printf("%32.32s \t\t        \tbytes/cycle\n", argv0);
    if(rank == 0) printf("%32.32s \t\t%d\t      \t\t%5.3f\t\t%d\n", argv0,
                         size,
                         //latency_time/repetitions, 
                         ((double)bytes * repetitions)/bandwidth_time,
                         bytes );
  }
  else if(bandwidth_time == 0.0)
  {
    if(rank == 0) printf("%32.32s \t\tNP\tLatency\t\t         \t     \n", argv0);
    if(rank == 0) printf("%32.32s \t\t(cycles)\t           \n", argv0);
    if(rank == 0) printf("%32.32s \t\t%d\t%g\t\t     \t\t     \n", argv0,
                         size,
                         latency_time/repetitions
                         //((double)bytes * repetitions)/bandwidth_time,
                         //bytes 
                         );
  }
  else
  {
    if(rank == 0) printf("%32.32s \t\tNP\tLatency\t\tBandWidth\tbytes\n", argv0);
    if(rank == 0) printf("%32.32s \t\t\t(us)\t\tMB/s\n", argv0);
    if(rank == 0) printf("%32.32s \t\t%d\t%5.3g\t\t%5.3g\t\t%d\n", argv0,
                         size,
                         latency_time*1e6/repetitions, 
                         ((double)bytes * repetitions * 1e-6)/bandwidth_time,
                         bytes );
  }
}

void cleanup(unsigned rc = 0)
{
  TRACE_TEST_VERBOSE((stderr,"%s:%s\n", argv0,__PRETTY_FUNCTION__));

  CCMI_Geometry_free(&geometry);

  MPI_Finalize ();

  free(allocSrcBuf);
  free(allocDstBuf);
  free(ranks);
  if(rc)
  {
    printf("FAIL\n");
    exit(1);
  }
  else if(rank == 0)
  {
      printf("SUCCESS\n");
  }
  return;
}

void setBuffers(unsigned *srcbuf, unsigned *dstbuf, unsigned pcount)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, dst %p, count %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, dstbuf, pcount));
  unsigned j;
  for(j = 0; j < pcount; j++)
  {
    srcbuf[j] = j+1;
    dstbuf[j] = (unsigned)-1;
  }
  srcbuf[pcount] = dstbuf[pcount] = -1; // padded with -1 to check overflow
}

int checkBuffers(unsigned *srcbuf, unsigned *dstbuf, unsigned pcount)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, dst %p, count %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, dstbuf, pcount));
  int ret = 0;
  unsigned j;

  for(j = 0; j < pcount; j++)
  {
    unsigned v = (j+1) * size;
    if(dstbuf[j] != v)
    {
      TERSE_OUTPUT fprintf(stderr,"(E) checkBuffers dstbuf[%d]:%d should be %d\n",j,dstbuf[j],v);
      ret++;
    }
    if(srcbuf[j] != j+1)
    {
      TERSE_OUTPUT fprintf(stderr,"(E) checkBuffers srcbuf[%d]:%d should be %d\n",j,srcbuf[j],j+1);
      ret++;
    }
    if(ret)
    {
      break;
    }
  }
  // The buffers should have -1 padding on either side
  if(allocSrcBuf[3] != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkBuffers allocSrcBuf[3]:%d should be -1\n",allocSrcBuf[3]); ret++;
  }
  if(allocDstBuf[3] != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkBuffers allocDstBuf[3]:%d should be -1\n",allocDstBuf[3]); ret++;
  }
  if(srcbuf[pcount]  != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkBuffers srcbuf[count=%d] :%d should be -1\n",pcount, srcbuf[pcount] ); ret++;
  }
  if(dstbuf[pcount]  != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkBuffers dstbuf[count=%d] :%d should be -1\n",pcount, dstbuf[pcount] ); ret++;
  }

  return ret;
}

int checkUntouchedBuffers(unsigned *srcbuf, unsigned *dstbuf, unsigned pcount)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, dst %p, count %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, dstbuf, pcount));
  int ret = 0;
  unsigned j;

  for(j = 0; j < pcount; j++)
  {
#ifdef STRICT_CHECKING
    if(dstbuf[j] != (unsigned) -1)
    {
      TERSE_OUTPUT fprintf(stderr,"(E) checkUntouchedBuffers dstbuf[%d]:%d should be %d\n",j,dstbuf[j],-1);
      ret++;
    }
#endif
    if(srcbuf[j] != j+1)
    {
      TERSE_OUTPUT fprintf(stderr,"(E) checkUntouchedBuffers srcbuf[%d]:%d should be %d\n",j,srcbuf[j],j+1);
      ret++;
    }
    if(ret)
    {
      break;
    }
  }
  // The buffers should have -1 padding on either side
  if(allocSrcBuf[3] != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkUntouchedBuffers allocSrcBuf[3]:%d should be -1\n",allocSrcBuf[3]); ret++;
  }
  if(allocDstBuf[3] != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkUntouchedBuffers allocDstBuf[3]:%d should be -1\n",allocDstBuf[3]); ret++;
  }
  if(srcbuf[pcount]  != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkUntouchedBuffers srcbuf[count=%d] :%d should be -1\n",pcount, srcbuf[pcount] ); ret++;
  }
  if(dstbuf[pcount]  != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkUntouchedBuffers dstbuf[count=%d] :%d should be -1\n",pcount, dstbuf[pcount] ); ret++;
  }

  return ret;
}
int checkInPlaceBuffer(unsigned *dstbuf, unsigned pcount)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, dst %p, count %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, dstbuf, pcount));
  int ret = 0;
  unsigned j;

  for(j = 0; j < pcount; j++)
  {
    unsigned v = (j+1) * size;
    if(dstbuf[j] != v)
    {
      TERSE_OUTPUT fprintf(stderr,"(E) checkInPlaceBuffer dstbuf[%d]:%d should be %d\n",j,dstbuf[j],v);
      ret++;
    }
    if(ret)
    {
      break;
    }
  }
  // The buffers should have -1 padding on either side
  if(allocDstBuf[3] != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkInPlaceBuffer allocDstBuf[3]:%d should be -1\n",allocDstBuf[3]); ret++;
  }
  if(dstbuf[pcount]  != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkInPlaceBuffer dstbuf[count=%d] :%d should be -1\n",pcount, dstbuf[pcount] ); ret++;
  }

  return ret;
}

int checkUntouchedInPlaceBuffer(unsigned *srcbuf, unsigned pcount)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, dst %p, count %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, dstbuf, pcount));
  int ret = 0;
  unsigned j;

  for(j = 0; j < pcount; j++)
  {
    if(srcbuf[j] != j+1)
    {
      TERSE_OUTPUT fprintf(stderr,"(E) checkUntouchedInPlaceBuffer srcbuf[%d]:%d should be %d\n",j,srcbuf[j],j+1);
      ret++;
    }
    if(ret)
    {
      break;
    }
  }
  // The buffers should have -1 padding on either side
  if(allocSrcBuf[3] != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkUntouchedInPlaceBuffer allocSrcBuf[3]:%d should be -1\n",allocSrcBuf[3]); ret++;
  }
  if(srcbuf[pcount]  != (unsigned) -1)
  {
    TERSE_OUTPUT fprintf(stderr,"(E) checkUntouchedInPlaceBuffer srcbuf[count=%d] :%d should be -1\n",pcount, srcbuf[pcount] ); ret++;
  }

  return ret;
}
#endif
