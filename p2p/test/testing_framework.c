/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

#include "pgasrt.h"

#define MAXBUFLEN (1024*1024)
#define MAXPROCS  1024

/* ************************************************************************* */
/* ************************************************************************* */

static double timer()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

/* ************************************************************************* */
/* ************************************************************************* */

void test_barrier  (int commID,
		    int rank,
		    int commsize,
		    int niters,
		    void * arg,
		    void (*BARRIER) (int),
		    int (*GET)(int commID, int rank, void *arg),
		    void (*SET)(int commID, int rank, void *arg, int val))
{
  int i, j;
  
  SET (commID, rank, arg, 0);
  BARRIER (commID);
  
  for (i=0; i<niters; i++)
    {
      BARRIER (commID);
      int z = GET (commID, rank, arg);
      SET (commID, rank, arg, z+1);
      for (j=0; j<commsize; j++)
	{
	  int y = GET (commID, rank, arg);
	  if (y != z && y != z+1)
	    __pgasrt_fatalerror (-1, "%d: Barrier %d fault:"
				 " counter[%d]=%d",
				 rank, z, j, y);
	}
    }

  double t0 = timer();
  for (i=0; i<niters; i++) { BARRIER (commID); }
  double t1 = timer();
  if (rank == 0)
    {
      printf ("barrier: OK, %g usecs/barrier\n", (t1-t0)/niters);
      fflush (stdout);
    }
  BARRIER (commID);
}

/* ************************************************************************* */
/*                testing framework for allgather operations                 */
/* ************************************************************************* */

void test_allgather (int    commID, 
		     int    rank, 
		     int    commsize, 
		     size_t maxlen, 
		     int    niters, 
		     void (*ALLGATHER)(int, const void *, void *, size_t), 
		     void (*BARRIER)(int))
{
  int i, j;
  size_t nbytes;
  unsigned char sbuf [MAXBUFLEN];
  unsigned char rbuf [MAXBUFLEN];

  if (maxlen >= MAXBUFLEN) maxlen = MAXBUFLEN;
  for (i=0; i<MAXBUFLEN; i++) sbuf[i] = (10*rank + i)&0xFF;
  memset (rbuf, 0, sizeof(rbuf));

  BARRIER(commID);
  for (nbytes=5 ; nbytes<(size_t)(maxlen/commsize) ; nbytes=nbytes*3/2)
    {
      ALLGATHER (commID, sbuf, rbuf, nbytes);

      for (i=0; i<commsize; i++)
        for (j=0; j<(int)nbytes; j++)
          {
            if (rbuf[i*nbytes + j] != ((10*i+j)&0xFF))
              __pgasrt_fatalerror (-1, "%d: Allgather mismatch: n=%d idx=%d "
				   "exp=%d actual=%d",
				   rank,
                                   nbytes, i*nbytes+j,
				   ((10*i+j)&0xFF),
				   rbuf[i*nbytes + j]);
          }

      if (niters > 0)
	{
	  double t0 = timer();
	  for (i = 0; i < niters; i++)
	    ALLGATHER (commID, sbuf, rbuf, nbytes);
	  double t1 = timer();
	  
	  if (rank == 0)
	    {
	      printf ("%10d %10g %12g\n",
		      nbytes, (t1-t0)/niters, nbytes * niters/(t1-t0));
	      fflush(stdout);
	    }
	}
    }

  if (rank==0) { printf ("Allgather OK\n"); fflush (stdout); }
  BARRIER(commID);
}


/* ************************************************************************* */
/*                      allgatherv framework                                 */
/* ************************************************************************* */

void test_allgatherv (int    commID, 
		     int    rank, 
		     int    commsize, 
		     size_t maxlen, 
		     int    niters, 
		     void (*ALLGATHERV)(int, const void *, void *, size_t *), 
		     void (*BARRIER)(int))
{
  int i, j;
  unsigned char sbuf [MAXBUFLEN];
  unsigned char rbuf [MAXBUFLEN];
  size_t nbytes, lengths [MAXPROCS];

  if (maxlen >= MAXBUFLEN) maxlen = MAXBUFLEN;
  assert (commsize < MAXPROCS);

  for (i=0; i<MAXBUFLEN; i++) sbuf[i] = (10*rank + i)&0xFF;
  memset (rbuf, 0, sizeof(rbuf));
  BARRIER(commID);

  for (nbytes=5 ; nbytes<(size_t)(maxlen/commsize) ; nbytes=nbytes*3/2)
    {
      for (i=0; i<commsize; i++) lengths[i] = nbytes + i;

      ALLGATHERV (commID, sbuf, rbuf, lengths);

      int idx=0;
      for (i=0; i<commsize; i++)
        for (j=0; j<(int)nbytes+i; j++)
          {
            if (rbuf[idx] != ((10*i+j)&0xFF))
              __pgasrt_fatalerror (-1, "%d: Mismatch in allgatherv "
                                   "off=%d exp=%d act=%d\n",
                                   PGASRT_MYNODE,
                                   idx,
                                   (10*i+j)&0xFF,
                                   rbuf[i*nbytes + j]);
	    idx++;
          }

      if (niters > 0)
	{
	  double t0 = timer();
	  for (i = 0; i < niters; i++)
	    ALLGATHERV (commID, sbuf, rbuf, lengths);
	  double t1 = timer();
	  
	  if (rank == 0)
	    {
	      printf ("%10d %10g %12g\n",
		      nbytes, (t1-t0)/niters, nbytes * niters/(t1-t0));
	      fflush(stdout);
	    }
	}
    }

  if (rank==0) { printf ("Allgatherv OK\n"); fflush (stdout); }
  BARRIER(commID);
}

/* ************************************************************************* */
/*                         broadcast test                                    */
/* ************************************************************************* */

void test_bcast (int    commID, 
		 int    rank, 
		 int    commsize, 
		 size_t maxlen, 
		 int    niters, 
		 void (*BCAST)(int, int, const void *, void *, size_t), 
		 void (*BARRIER)(int))
{
  unsigned char buf [MAXBUFLEN];
  int i, j;
  size_t nbytes;

  if (maxlen >= MAXBUFLEN) maxlen = MAXBUFLEN;

  BARRIER(commID);

  int root = 0;
  for (nbytes = 5; nbytes < maxlen; nbytes = nbytes * 3 / 2)
  {
    BARRIER (commID);
    if (rank == root)
      for (i=0; i<(int)maxlen; i++) buf[i] = (10*rank + i)&0xFF;
    else
      memset (buf, 0, sizeof(buf));

    BCAST (commID, root, buf, buf, nbytes);
    BARRIER (commID);

#if 0
    printf ("%d: ", rank);
    for (j=0; j<(int)nbytes; j++) printf ("%2d ", buf[j]);
    printf ("\n");
#endif

    for (j=0; j<(int)nbytes; j++)
      {
	if (buf[j] != ((10*root+j)&0xFF))
	  __pgasrt_fatalerror (-1, "%d: Mismatch in bcast: "
			       "idx=%d/%d exp=%d act=%d",
			       rank, j, (int)nbytes, 
			       ((10*root+j)&0xFF),
			       buf[j]);
      }

    BARRIER (commID);
    if (niters > 0)
      {
	double t0 = timer();
	for (i = 0; i < niters; i++)
	  BCAST(commID, root, buf, buf, nbytes);
	double t1 = timer();
	if (rank == 0)
	  {
	    printf ("%10d %10g %12g\n",
		    nbytes, (t1-t0)/niters, nbytes*niters/(t1-t0));
	    fflush(stdout);
	  }
      }
    root = (root+1) % commsize;
  }

  if (rank==0) { printf ("Bcast OK\n"); fflush (stdout); }
  BARRIER(commID);
}

/* ************************************************************************* */
/*                          scatter test                                     */
/* ************************************************************************* */

void test_scatter (int    commID, 
		   int    rank, 
		   int    commsize, 
		   size_t maxlen, 
		   int    niters, 
		   void (*SCATTER)(int, int, const void *, void *, size_t), 
		   void (*BARRIER)(int))
{
  int i, j;
  size_t nbytes;
  unsigned char sbuf [MAXBUFLEN];
  unsigned char rbuf [MAXBUFLEN];

  if (maxlen >= MAXBUFLEN) maxlen = MAXBUFLEN;
  for (i=0; i<MAXBUFLEN; i++) sbuf[i] = (10*rank + i)&0xFF;
  memset (rbuf, 0, sizeof(rbuf));

  BARRIER(commID);

  int root = 0;
  for (nbytes = 5; nbytes < maxlen/commsize; nbytes = nbytes * 3 / 2)
  {
    SCATTER (commID, root, sbuf, rbuf, nbytes);
    BARRIER (commID);

    for (j=0; j<(int)nbytes; j++)
      {
	if (rbuf[j] != ((10*root + nbytes*rank + j)&0xFF))
	  __pgasrt_fatalerror (-1, "%d: Mismatch in scatter "
			       "idx=%d/%d exp=%d actual=%d\n",
			       rank, j, nbytes, 
			       ((10*root + nbytes*rank + j)&0xFF),
			       rbuf[j]);
      }

    if (niters > 0)
      {
	double t0 = timer();
	for (i = 0; i < niters; i++)
	  SCATTER(commID, root, sbuf, rbuf, nbytes);
	double t1 = timer();
	if (rank == 0)
	  {
	    printf ("%10d %10g %12g\n",
		    nbytes, (t1-t0)/niters, nbytes*niters/(t1-t0));
	    fflush(stdout);
	  }
      }
    root = (root+1) % commsize;
  }

  if (rank==0) { printf ("Scatter OK\n"); fflush (stdout); }
  BARRIER(commID);
}


/* ************************************************************************* */
/*                        scatterv test                                      */
/* ************************************************************************* */

void test_scatterv (int    commID, 
		    int    rank, 
		    int    commsize, 
		    size_t maxlen, 
		    int    niters, 
		    void (*SCATTERV)(int, int, const void *, void *, size_t*), 
		    void (*BARRIER)(int))
{
  int i, j;
  size_t nbytes;


  unsigned char sbuf [MAXBUFLEN+MAXPROCS];
  unsigned char rbuf [MAXBUFLEN];
  size_t lengths[MAXPROCS];

  if (maxlen >= MAXBUFLEN) maxlen = MAXBUFLEN;
  for (i=0; i<(int)sizeof(sbuf); i++) sbuf[i] = (10*rank + i)&0xFF;
  memset (rbuf, 0, sizeof(rbuf));

  BARRIER(commID);

  int root = 0;
  for (nbytes = 5; nbytes < maxlen/commsize; nbytes = nbytes * 3 / 2)
  {
    for (i=0; i<commsize; i++) lengths[i] = nbytes + i;

    SCATTERV (commID, root, sbuf, rbuf, lengths);
    BARRIER (commID);

    for (j=0; j<(int)lengths[rank]; j++)
      {
	int offset = 0; for (i=0;i<rank; i++) offset += lengths[i];
	if (rbuf[j] != ((10*root + offset + j)&0xFF))
	  __pgasrt_fatalerror (-1, "%d: Mismatch in scatterv "
			       "idx=%d/%d exp=%d actual=%d\n",
			       rank, j, nbytes, 
			       ((10*root + offset + j)&0xFF),
			       rbuf[j]);
      }

    if (niters > 0)
      {
	double t0 = timer();
	for (i = 0; i < niters; i++)
	  SCATTERV(commID, root, sbuf, rbuf, lengths);
	double t1 = timer();
	if (rank == 0)
	  {
	    printf ("%10d %10g %12g\n",
		    nbytes, (t1-t0)/niters, nbytes*niters/(t1-t0));
	    fflush(stdout);
	  }
      }
    root = (root+1) % commsize;
  }

  if (rank==0) { printf ("Scatterv OK\n"); fflush (stdout); }
  BARRIER(commID);
}

/* ************************************************************************* */
/*                          gather  test                                     */
/* ************************************************************************* */

void test_gather  (int    commID, 
		   int    rank, 
		   int    commsize, 
		   size_t maxlen, 
		   int    niters, 
		   void (*GATHER)(int, int, const void *, void *, size_t), 
		   void (*BARRIER)(int))
{
  int i, j;
  size_t nbytes;
  unsigned char sbuf [MAXBUFLEN];
  unsigned char rbuf [MAXBUFLEN];

  if (maxlen >= MAXBUFLEN) maxlen = MAXBUFLEN;
  for (i=0; i<MAXBUFLEN; i++) sbuf[i] = (10*rank + i)&0xFF;

  BARRIER(commID);

  int root = 0;
  for (nbytes = 5; nbytes < maxlen/commsize; nbytes = nbytes * 3 / 2)
  {
    memset (rbuf, 0, sizeof(rbuf));
    GATHER  (commID, root, sbuf, rbuf, nbytes);
    BARRIER (commID);
    if (rank == root)
      for (i=0; i<commsize; i++)
	for (j=0; j<(int)nbytes; j++)
	  {
	    if (rbuf[i*nbytes+j] != ((10*i + j)&0xFF))
	      __pgasrt_fatalerror (-1, "%d: Mismatch in gather "
				   " idx=%d, exp=%d act=%d\n",
				   rank, i*nbytes+j,
				   (10*i+j)&0xFF, rbuf[i*nbytes+j]);
	  }

    BARRIER (commID);
    if (niters > 0)
      {
	double t0 = timer();
	for (i = 0; i < niters; i++)
	  GATHER (commID, root, sbuf, rbuf, nbytes);
	double t1 = timer();
	if (rank == 0)
	  {
	    printf ("%10d %10g %12g\n",
		    nbytes, (t1-t0)/niters, nbytes*niters/(t1-t0));
	    fflush(stdout);
	  }
      }
    root = (root+1) % commsize;
  }

  if (rank==0) { printf ("Gather OK\n"); fflush (stdout); }
  BARRIER(commID);
}


/* ************************************************************************* */
/*                         allreduce test                                    */
/* ************************************************************************* */

/* ------------------------------ */
/*   test SUM of <type>           */
/* ------------------------------ */

#define TESTSUM(typename,type)						\
  void test_allreduce_sum_##typename (int commID, int rank,		\
				      int commsize, size_t maxlen,	\
				      int niters,			\
				      void (*AR)(int,			\
						 const void *,		\
						 void *,		\
						 __pgasrt_ops_t,	\
						 __pgasrt_dtypes_t,	\
						 unsigned),		\
				      void (*BARRIER)(int))		\
  {									\
    type sbuf[MAXBUFLEN/sizeof(type)], rbuf[MAXBUFLEN/sizeof(type)];	\
    for (int i=0; i<(int)(MAXBUFLEN/sizeof(type)); i++)			\
      sbuf[i] = (type) (i + rank);					\
    if (maxlen > (int)(MAXBUFLEN/sizeof(type)))				\
      maxlen=MAXBUFLEN/sizeof(type);					\
    for (int nelems = 1; nelems<(int)maxlen; nelems=(1+nelems)*3/2)	\
      {									\
	memset (rbuf, maxlen*sizeof(type), 0);				\
	AR(commID,sbuf,rbuf,PGASRT_OP_ADD,PGASRT_DT_##typename,nelems); \
	for (int i=0; i<nelems; i++)					\
	  {								\
	    type exp = (type)(commsize*i+commsize*(commsize-1)/2);	\
	    if (rbuf[i] != exp)						\
	      __pgasrt_fatalerror(-1, "%d: AR (%s)x%d mismatch "	\
				  "idx=%d exp=%g act=%g\n",		\
				  rank, #type, nelems, i,		\
				  (double)exp, (double)rbuf[i]);	\
	  }								\
									\
	if (niters > 0)	{						\
	  double t0 = timer();						\
	  for (int i = 0; i < niters; i++)				\
	    AR(commID,sbuf,rbuf,PGASRT_OP_ADD,PGASRT_DT_##typename,nelems); \
	  double t1 = timer();						\
	  if (rank == 0) {						\
	    printf ("%10d %10g %12g\n",					\
		    nelems, (t1-t0)/niters, nelems*niters/(t1-t0));	\
	    fflush(stdout);						\
	  }								\
	}								\
      }									\
    if (rank==0) printf ("Allreduce SUM type=%s OK\n", #type);		\
    BARRIER(commID);                                                    \
  }

TESTSUM(dbl,  double)
TESTSUM(flt,  float)
TESTSUM(int,  int)
TESTSUM(word, unsigned)
TESTSUM(llg,  long long)
TESTSUM(dwrd, unsigned long long)



void test_allreduce (int    commID,
		     int    rank,
		     int    commsize,
		     size_t maxlen,
		     int    niters,
		     void (*AR)(int, const void *, void *, 
				__pgasrt_ops_t, __pgasrt_dtypes_t, unsigned),
                    void (*BARRIER)(int))
{
  test_allreduce_sum_dbl (commID, rank, commsize, maxlen, niters, AR, BARRIER);
  test_allreduce_sum_flt (commID, rank, commsize, maxlen, niters, AR, BARRIER);
  test_allreduce_sum_int (commID, rank, commsize, maxlen, niters, AR, BARRIER);
  test_allreduce_sum_word(commID, rank, commsize, maxlen, niters, AR, BARRIER);
  test_allreduce_sum_llg (commID, rank, commsize, maxlen, niters, AR, BARRIER);
  test_allreduce_sum_dwrd(commID, rank, commsize, maxlen, niters, AR, BARRIER);

  /* ------------------------------ */
  /*   test MAX with DOUBLE_INT     */
  /* ------------------------------ */
  
  {
    struct { double v; int idx; } sbuf = { (double)rank, rank} , rbuf;
    AR (commID, &sbuf, &rbuf, PGASRT_OP_MAX, PGASRT_DT_dblint, 1);
    
    if (rbuf.v != commsize-1 || rbuf.idx != commsize-1)
      __pgasrt_fatalerror(-1, "Allreduce MAX failure 1");
    BARRIER(commID);
    
    sbuf.idx = commsize-1-rank;
    AR (commID, &sbuf, &rbuf, PGASRT_OP_MAX, PGASRT_DT_dblint, 1);
    if (rbuf.v != commsize-1 || rbuf.idx != 0)
      __pgasrt_fatalerror(-1, "Allreduce MAX failure 2");
    BARRIER(commID);
    
    sbuf.idx = rank; sbuf.v = 1.0;
    AR (commID, &sbuf, &rbuf, PGASRT_OP_MAX, PGASRT_DT_dblint, 1);
    
    if (rbuf.v != 1.0 || rbuf.idx != commsize-1)
      __pgasrt_fatalerror(-1, "Allreduce MAX failure 3");
    BARRIER(commID);
    
    if (rank==0) printf ("Allreduce MAX type=dblint is OK\n");
  }
  BARRIER (commID);
}
