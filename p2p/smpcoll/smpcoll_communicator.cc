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
/*                  communicator constructor                                */
/* ************************************************************************ */

SMPColl::Communicator::Communicator (int s) :
  _size(s), _barrier(s), _allreduce(s)
{
}

/* ************************************************************************ */
/*                       barrier collective                                 */
/* ************************************************************************ */

void SMPColl::Communicator::barrier(void)
{
  _barrier.execute (rank());
}

/* ************************************************************************ */
/*                       allgather collective                               */
/* ************************************************************************ */

void SMPColl::Communicator::
allgather (const void * sbuf, void * rbuf, size_t nbytes)
{
  _barrier.setaddr (rank(), (void *)sbuf);
  _barrier.execute (rank());
  char * r = (char *) rbuf;
  for (int i=0; i<size(); i++) /* copy buffers from everyone */
    memcpy (r + i * nbytes, _barrier.getaddr(i), nbytes);
  _barrier.execute (rank());
}

/* ************************************************************************ */
/*                         allgatherv                                       */
/* ************************************************************************ */

void SMPColl::Communicator::
allgatherv (const void * sbuf, void * rbuf, size_t * lengths)
{
  _barrier.setaddr (rank(), (void *)sbuf);
  _barrier.execute (rank());
  char * r = (char *) rbuf;
  for (int i=0; i<size(); i++)
    {
      memcpy (r, _barrier.getaddr(i), lengths[i]);
      r += lengths[i];
    }
  _barrier.execute (rank());
}

/* ************************************************************************ */
/*                              scatter                                     */
/* ************************************************************************ */

void SMPColl::Communicator::
scatter (int root, const void * sbuf, void * rbuf, size_t length)
{
  _barrier.setaddr (rank(), (void *)sbuf);
  _barrier.execute (rank());
  char * s = (char *)_barrier.getaddr(root) + rank() * length;
  memcpy (rbuf, s, length);
  _barrier.execute (rank());
}

/* ************************************************************************ */
/*                                scatterv                                  */
/* ************************************************************************ */

void SMPColl::Communicator::
scatterv (int root, const void * sbuf, void * rbuf, size_t * lengths)
{
  _barrier.setaddr (rank(), (void *)sbuf);
  _barrier.execute (rank());
  char * s = (char *)_barrier.getaddr(root);
  for (int i=0; i<rank(); i++) s += lengths[i];
  memcpy (rbuf, s, lengths[rank()]);
  _barrier.execute (rank());
}

/* ************************************************************************ */
/*                               gather                                     */
/* ************************************************************************ */

void SMPColl::Communicator::
gather (int root, const void * sbuf, void * rbuf, size_t length)
{
  _barrier.setaddr (rank(), (void *)rbuf);
  _barrier.execute (rank());
  char * d = (char *)_barrier.getaddr(root) + rank() * length;
  memcpy (d, sbuf, length);
  _barrier.execute (rank());
}

/* ************************************************************************ */
/*                              gatherv                                     */
/* ************************************************************************ */

void SMPColl::Communicator::
gatherv (int root, const void * sbuf, void * rbuf, size_t * lengths)
{
  _barrier.setaddr (rank(), (void *)rbuf);
  _barrier.execute (rank());
  char * d = (char *)_barrier.getaddr(root);
  for (int i=0; i<rank(); i++) d += lengths[i];
  memcpy (d, sbuf, lengths[rank()]);
  _barrier.execute (rank());
}

/* ************************************************************************ */
/*                              broadcast                                   */
/* ************************************************************************ */

void SMPColl::Communicator::
bcast (int root, const void * sbuf, void * rbuf, size_t length)
{
  _barrier.setaddr (rank(), (void *)sbuf);
  _barrier.execute (rank());
  void * realsbuf = _barrier.getaddr (root);
  if (realsbuf != rbuf) memcpy (rbuf, realsbuf, length);
  __pgasrt_smp_fence();
  _barrier.execute (rank());
}

/* ************************************************************************ */
/*                              allreduce                                   */
/* ************************************************************************ */

void SMPColl::Communicator::
allreduce  (const void *s, void *d,
	    __pgasrt_ops_t op, __pgasrt_dtypes_t dtype,
	    unsigned nelems)
{
  void (*cb)(const void *, void *, unsigned) = NULL;
  cb = TSPColl::Allreduce::getcallback (op, dtype);
  size_t dw = TSPColl::Allreduce::datawidthof (dtype);
  assert (cb != NULL);
  if (s != d) memcpy (d, s, nelems * dw);

  if (nelems < 32)
    {
      char myD [1024];
      __pgasrt_smp_fence();
      _barrier.setaddr (rank(), d);
      _barrier.execute (rank());
      memset (myD, 0, sizeof(myD));
      for (int i=0; i<size(); i++) cb (myD, _barrier.getaddr(i), nelems);
      _barrier.execute (rank());
      memcpy (d, myD, nelems * dw);
      __pgasrt_smp_fence();
    }
  else
    _allreduce.execute (rank(), d, op, dtype, nelems);
}

/* ************************************************************************ */
/* ************************************************************************ */

#define COURSEOF(node) ((node)/(_BF*_ncomms))
#define COMMOF(node) (((node)/_BF)%_ncomms)
#define VIRTOF(node) (((node)%_BF)+(_BF*COURSEOF(node)))


SMPColl::BC_Comm::BC_Comm (int BF, int ncomms, int mycomm):
  Communicator(PGASRT_SMPTHREADS/ncomms), _BF(BF), _ncomms(ncomms)
{
  _size         = PGASRT_SMPTHREADS/ncomms;
  _mycomm       = mycomm;
  if ((int)(_size * ncomms) != (int)PGASRT_SMPTHREADS)
    __pgasrt_fatalerror (-1, "bad arguments for blocked SMP communicator");
}

/* ************************************************************************ */
/* ************************************************************************ */

int SMPColl::BC_Comm::rank () const
{
  if (COMMOF(PGASRT_MYTHREAD)==_mycomm) return VIRTOF(PGASRT_MYSMPTHREAD);
  return -1;
}

/* ************************************************************************ */
/*    absolute rank corresponding to virtual rank in *my* communicator      */
/* ************************************************************************ */
/*  rank/BF == block corresponding to rank                                  */
/*  (rank/BF) * BF * ncomms = course                                        */
/* ************************************************************************ */

int SMPColl::BC_Comm::absrankof (int rank) const
{

  return
    (rank/_BF)*_BF*_ncomms +    /* current course of rank               */
    _mycomm * _BF +             /* block in course of *my* communicator */
    (rank%_BF);                 /* rank's phase in block                */
}

/* ************************************************************************ */
/*    virtual rank of a particular absolute rank                            */
/* ************************************************************************ */

int SMPColl::BC_Comm::virtrankof (int rank) const
{
  if (COMMOF(rank)==_mycomm) return VIRTOF(rank);
  return -1;
}

