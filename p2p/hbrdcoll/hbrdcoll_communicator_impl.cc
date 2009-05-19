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

#include "./Communicator.h"
#include "../tspcoll/Allreduce.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************ */
/*                        hybrid barrier                                    */
/* ************************************************************************ */

void HybridColl::Communicator::barrier ()
{
  _smpcomm->barrier();
  if (_smpcomm->rank() == 0) _tspcomm->barrier();
  _smpcomm->barrier();
}

/* ************************************************************************ */
/*                   hybrid allgather                                       */
/* ************************************************************************ */

void HybridColl::Communicator::allgather (const void *s, void *d, size_t l)
{
  int localroot = 0;

  /* --------------------------------------------------------------- */
  /* GATHER up local contributions from my node                      */
  /* --------------------------------------------------------------- */

  _smpcomm->gather (localroot, s, (char *)d + rank () * l, l);

  /* --------------------------------------------------------------- */
  /* if I am the local leader, participate in a transport allgatherv */
  /* --------------------------------------------------------------- */

  if (_smpcomm->rank() == 0 && _tspcomm->size() > 1) 
    {
      /* ------------------------------------------------- */
      /* figure out how many bytes each node contributes:  */
      /* (l * threads per node in this comm) on each node. */
      /* ------------------------------------------------- */

      size_t * lengths=(size_t *) malloc (_tspcomm->size() * sizeof (size_t));
      if (lengths == NULL)
	__pgasrt_fatalerror (-1, "Hybrid allgather: Allocation error");
      for (int i=0; i<_tspcomm->size(); i++)       lengths[i] = 0;
      for (int i=0; i<size(); i++)                 lengths[to_tsp(i)] += l;

      /* --------------- */
      /* run allgatherv  */
      /* --------------- */

      _tspcomm->allgatherv ((const void *)((char *)d + rank() * l), 
			    (void *)d, 
			    lengths);
      free (lengths);
    }

  /* --------------------------------------------------------------- */
  /* in-node leader broadcasts allgather results.                    */
  /* --------------------------------------------------------------- */

  _smpcomm->bcast (localroot, d, d, size() * l);
}

/* ************************************************************************ */
/*                   hybrid allgatherv                                      */
/* ************************************************************************ */

void HybridColl::Communicator::allgatherv (const void *s, void *d, size_t *l)
{
  int localroot = 0;
  int ldr_hbrd_rank = rank() / PGASRT_SMPTHREADS * PGASRT_SMPTHREADS;

  /* --------------------------------------------------------------- */
  /* GATHER up local contributions from my node                      */
  /* --------------------------------------------------------------- */
  
  char * p = (char *)d;
  for (int i=0; i<ldr_hbrd_rank; i++) p += l[i];
  _smpcomm->gatherv (localroot, s, (void *)p, l + ldr_hbrd_rank);

  /* --------------------------------------------------------------- */
  /* if I am the local leader, participate in a transport allgatherv */
  /* --------------------------------------------------------------- */

  if (_smpcomm->rank() == 0 && _tspcomm->size() > 1) 
    {
      /* ------------------------------------------------- */
      /* figure out how many bytes each node contributes:  */
      /* (l * threads per node in this comm) on each node. */
      /* ------------------------------------------------- */

      size_t * l2=(size_t *) malloc (_tspcomm->size() * sizeof (size_t));
      if (l2 == NULL)
	__pgasrt_fatalerror (-1, "Hybrid allgatherv: Allocation error");
      for (int i=0; i<_tspcomm->size(); i++)  l2[i] = 0;
      for (int i=0; i<size(); i++)            l2[to_tsp(i)] += l[i];

      /* --------------- */
      /* run allgatherv  */
      /* --------------- */

      _tspcomm->allgatherv ((const void *)p, (void *)d, l2);
      free (l2);
    }

  /* --------------------------------------------------------------- */
  /* in-node leader broadcasts allgather results.                    */
  /* --------------------------------------------------------------- */
  size_t totallength = 0;
  for (int i=0; i<size(); i++) totallength += l[i];
  _smpcomm->bcast (localroot, d, d, totallength);
}

/* ************************************************************************ */
/*                       hybrid scatter                                     */
/* ************************************************************************ */

void HybridColl::Communicator::
scatter (int root, const void * sbuf, void * rbuf, size_t length)
{
  // _smpcomm->scatter (localroot, s, (char *)d + rank () * l, l);
  assert (0);
}

/* ************************************************************************ */
/* ************************************************************************ */

void HybridColl::Communicator::
scatterv (int root, const void * sbuf, void * rbuf, size_t * lengths)
{
  assert (0);
}

/* ************************************************************************ */
/* ************************************************************************ */

void HybridColl::Communicator::
gather (int root, const void * sbuf, void * rbuf, size_t length)
{
  assert (0);
}

/* ************************************************************************ */
/* ************************************************************************ */

void HybridColl::Communicator::
gatherv (int root, const void * sbuf, void * rbuf, size_t * lengths)
{
  assert (0);
}

/* ************************************************************************ */
/*                       broadcast                                          */
/* ************************************************************************ */

void HybridColl::Communicator::
bcast (int root, const void * sbuf, void * rbuf, size_t nbytes)
{
  /* ----------------------------------- */
  /* transmit buffer to smpcoll rank = 0 */
  /* ----------------------------------- */
      
  if (this->rank() == root) { _smpcomm->setaddr (0, (void *)sbuf); }
  _smpcomm->barrier ();
  void * realsbuf = _smpcomm->getaddr (0);
  _smpcomm->barrier ();  

  if (_tspcomm->size() > 1)
    {
      /* ---------------------------------- */
      /* do a transport broadcast if needed */
      /* ---------------------------------- */

      if (_smpcomm->rank() == 0)
	_tspcomm->bcast (to_tsp(root), realsbuf, rbuf, nbytes);
      
      /* ---------------------------------- */
      /* re-broadcast locally               */  
      /* ---------------------------------- */

      _smpcomm->bcast (0, rbuf, rbuf, nbytes);
    }
  else
    {
      _smpcomm->bcast (0, realsbuf, rbuf, nbytes);
    }
}


/* ************************************************************************ */
/* ************************************************************************ */

void HybridColl::Communicator::
allreduce  (const void *s, void *d,
	    __pgasrt_ops_t op, __pgasrt_dtypes_t dtype,
	    unsigned nelems)
{
  /* local allreduce in node */
  _smpcomm->allreduce (s, d, op, dtype, nelems);

  /* allreduce across nodes */
  if (_tspcomm->size() > 1) 
    {
      if (_smpcomm->rank() == 0)
	{
	  _tspcomm->allreduce (d, d, op, dtype, nelems);
	}
      
      /* local broadcast of result */
      size_t dw = TSPColl::Allreduce::datawidthof (dtype);
      _smpcomm->bcast (0, d, d, nelems * dw);
    }
}
