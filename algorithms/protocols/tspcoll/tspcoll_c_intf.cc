/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2009, 2010.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */
/**
 * \file algorithms/protocols/tspcoll/tspcoll_c_intf.cc
 * \brief ???
 */

#include "interface/Communicator.h"

#include <stdio.h>
#include <assert.h>

#undef TRACE
//#define TRACE(x) fprintf x
#define TRACE(x)

#define MAXCOLL 16
static TSPColl::Communicator * _tspcoll [MAXCOLL];
static bool _initialized = false;

/* ************************************************************************ */
/*                    set up a new communicator                             */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_comm_init (void)
{
  __pgasrt_tsp_barrier ();

  /* ------------------------- */
  /* Check initialization flag */
  /* ------------------------- */

  if (_initialized)__pgasrt_fatalerror (-1,"tspcoll initialized twice");
  _initialized = true;
  for (int i=0; i<MAXCOLL; i++) _tspcoll[i] = NULL;

  /* ------------------------- */
  /* create comm-world         */
  /* ------------------------- */

  _tspcoll[0]=(TSPColl::Communicator *) malloc (sizeof(TSPColl::Communicator));
  if (!_tspcoll[0]) __pgasrt_fatalerror (-1, "%s: malloc error", __FUNCTION__);
  new (_tspcoll[0]) TSPColl::Communicator ();
  _tspcoll[0]->setup();
  __pgasrt_tsp_barrier ();
}

/* ************************************************************************ */
/*                    communicator rank                                     */
/* ************************************************************************ */

extern "C" __pgasrt_thread_t __pgasrt_tspcoll_comm_rank (int commID)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  return (__pgasrt_thread_t) _tspcoll[commID]->rank();
}

/* ************************************************************************ */
/*                    rank of any ID in a communicator                      */
/* ************************************************************************ */

extern "C" __pgasrt_thread_t
__pgasrt_tspcoll_comm_rankof (int commID, __pgasrt_thread_t rank)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  return (__pgasrt_thread_t) _tspcoll[commID]->absrankof(rank);
}

/* ************************************************************************ */
/*                   communicator size                                      */
/* ************************************************************************ */

extern "C" __pgasrt_thread_t __pgasrt_tspcoll_comm_size (int commID)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  return (__pgasrt_thread_t) _tspcoll[commID]->size();
}

/* ************************************************************************ */
/*                   split a communicator                                   */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_comm_split   (int            commID,
                                               int            newID,
                                               int            color,
                                               int            rank)
{
  /* ---------------------------------------------------- */
  /* create a list of all processes in split communicator */
  /* ---------------------------------------------------- */
  int oldsize = _tspcoll[commID]->size();
  int * plist = (int *) malloc(sizeof(int)*oldsize);
  if (!plist) __pgasrt_fatalerror (-1, "comm_split: allocation error");
  int nsize = _tspcoll[commID]->split (color, rank, plist);

  //#if 0
  int mynewrank = -1;
  for (int i=0; i<nsize; i++) if (plist[i]==PGASRT_MYNODE) mynewrank=i;
  if (mynewrank<0) __pgasrt_fatalerror(-1, "comm_setup: Invalid process list");
  //#endif

  /* ---------------------------------------------------- */
  /* create and initialize an enumerated communicator     */
  /* ---------------------------------------------------- */

  TSPColl::Communicator * c = NULL;
  c = (TSPColl::Communicator *) malloc (sizeof(TSPColl::EnumComm));
  if (!c) __pgasrt_fatalerror (-1, "%s: malloc error", __FUNCTION__);
  new (c) TSPColl::EnumComm (rank, nsize, plist);
  _tspcoll[newID] = c;
  c->setup();

  /* ---------------------------------------------------- */
  /* free the process list and pass thru barrier          */
  /* ---------------------------------------------------- */
  free (plist);
  _tspcoll[commID]->barrier();
}

/* ************************************************************************ */
/*                    create a blocked communicator                         */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_comm_block   (int            newID,
                                               int            BF,
                                               int            ncomms)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[0]->barrier();

  TSPColl::Communicator * c = NULL;
  c = (TSPColl::Communicator *) malloc (sizeof(TSPColl::BC_Comm));
  if (!c) __pgasrt_fatalerror (-1, "%s: malloc error", __FUNCTION__);
  new (c) TSPColl::BC_Comm (BF, ncomms);
  _tspcoll[newID] = c;
  c->setup();

  _tspcoll[0]->barrier();
}


/* ************************************************************************ */
/*                   barrier implementation                                 */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_barrier (int commID)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[commID]->barrier();
}

/* ************************************************************************ */
/*                   allgather implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_allgather (int commID,
                                            const void * sbuf,
                                            void * rbuf,
                                            size_t bufsize)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[commID]->allgather (sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                   allgatherv implementation                              */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_allgatherv (int commID,
                                            const void * sbuf,
                                            void * rbuf,
                                             size_t * lengths)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[commID]->allgatherv (sbuf, rbuf, lengths);
}

/* ************************************************************************ */
/*                   scatter   implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_scatter (int commID,
                                          int root,
                                          const void * sbuf,
                                          void * rbuf,
                                          size_t bufsize)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[commID]->scatter (root, sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                   scatterv  implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_scatterv (int commID,
                                           int root,
                                           const void * sbuf,
                                           void * rbuf,
                                           size_t * lengths)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[commID]->scatterv (root, sbuf, rbuf, lengths);
}

/* ************************************************************************ */
/*                   scatter   implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_bcast (int commID,
                                        int root,
                                        const void * sbuf,
                                        void * rbuf,
                                        size_t bufsize)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[commID]->bcast (root, sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_allreduce   (int               commID,
                                              const void      * sbuf,
                                              void            * rbuf,
                                              __pgasrt_ops_t    op,
                                              __pgasrt_dtypes_t dtype,
                                              unsigned          nelems)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[commID]->allreduce (sbuf,rbuf,op,dtype,nelems);
}

/* ************************************************************************ */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_gather (int commID,
                                         int root,
                                         const void * sbuf,
                                         void * rbuf,
                                         size_t bufsize)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[commID]->gather (root, sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                   gatherv  implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_tspcoll_gatherv (int commID,
                                          int root,
                                          const void * sbuf,
                                          void * rbuf,
                                          size_t * lengths)
{
  if (!_initialized)__pgasrt_fatalerror (-1,"tspcoll not initialized");
  assert (_tspcoll != NULL);
  _tspcoll[commID]->gatherv (root, sbuf, rbuf, lengths);
}
