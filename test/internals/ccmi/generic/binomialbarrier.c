/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/tests/generic/binomialbarrier.c
 * \brief ???
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "ccmi_collectives.h"
#include <math.h>
#include <assert.h>

#define MAXRANKS 1025


int mysleep(int seconds)
{
  long long sec = (seconds*700000000ULL)/7ULL; /* Fudge for broken sleep() */
  long long i;
  volatile long long j;
  for(i=0; i<sec; i++)
    {
    }
  j = i;

  return j;
}

PAMI_CollectiveProtocol_t              bar_reg __attribute__((__aligned__(32))), local_bar_reg __attribute__((__aligned__(32)));
CCMI_Barrier_Configuration_t           configuration;
PAMI_CollectiveRequest_t               request;
PAMI_Callback_t                        cb_done;
CCMI_Consistency                       consistency;
CCMI_Geometry_t                        geometry;
int                                    done;

CCMI_Geometry_t *getGeometry (int comm) {
  return &geometry;
}

void barrier()
{
  done = 0;
  CCMI_Barrier(&geometry,
               cb_done,
               consistency);

  while(!done)
    {
      CCMI_Generic_adaptor_advance();
    }
}

void done_callback(void* cd, PAMI_Error_t *err)
{
  done=1;

}

#define NITER 100

double BarrierTime(void)
{
  int i =0;
  double tbstart, tbfinish;

  tbstart = MPI_Wtime();
  for(i=0; i< NITER; i++)
    barrier();
  tbfinish = MPI_Wtime();

  return (tbfinish - tbstart)/NITER;
}



int main(int argc, char **argv)
{
  int rank, size;
  int i;
  double tf;

  unsigned    ranks[MAXRANKS];

  //fprintf (stderr, "Starting barrier test\n");

  MPI_Init (&argc, &argv);
  CCMI_Collective_initialize();

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  PAMI_assert(size < MAXRANKS);

  for (i = 0; i < size; i++)
    ranks[i] = i;

  configuration.protocol = CCMI_BINOMIAL_BARRIER_PROTOCOL;
  configuration.cb_geometry = getGeometry;

  CCMI_Result ccmiResult;
  if((ccmiResult = (CCMI_Result) CCMI_Barrier_register(&bar_reg, &configuration)) != PAMI_SUCCESS)
    fprintf(stderr,"CCMI_Barrier_register failed %d\n",ccmiResult);

#if 0
  configuration.protocol = CCMI_BINOMIAL_BARRIER_PROTOCOL;
  if((ccmiResult = (CCMI_Result) CCMI_Barrier_register(&local_bar_reg, &configuration)) != PAMI_SUCCESS)
    fprintf(stderr,"CCMI_Barrier_register failed %d\n",ccmiResult);
#endif

  PAMI_CollectiveProtocol_t             * bar_p = & bar_reg, * local_bar_p = & local_bar_reg;
  if((ccmiResult = (CCMI_Result) CCMI_Geometry_initialize (&geometry, 0, ranks, size,
                                                           &bar_p, 1,
                                                           &bar_p, 1,
                                                           &request, 0, 1)) != PAMI_SUCCESS)
    fprintf(stderr,"CCMI_Geometry_initialize failed %d\n",ccmiResult);

  cb_done.function  = done_callback;
  cb_done.clientdata= NULL;

  MPI_Barrier (MPI_COMM_WORLD);

  barrier();
  barrier();
  barrier();

  tf = BarrierTime();

  barrier();
  barrier();

  printf("rank = %d Average Barrier Time %lf us\n", rank,tf*1e6);

  MPI_Finalize ();

  return 0;
}
