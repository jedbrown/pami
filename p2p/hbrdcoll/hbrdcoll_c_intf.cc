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

#include "Communicator.h"



#define MAXCOMM 16
#define MAXTPN 64
static HybridColl::Communicator * _hbrdcomm [MAXCOMM][MAXTPN];
static bool _initialized = false;

/* ************************************************************************ */
/*             initialize hybrid communicator                               */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_comm_init (void)
{
  static SMPColl::Communicator * smpcomm;
  static TSPColl::Communicator * tspcomm;

  /* ------------------------- */
  /* Check initialization flag */
  /* ------------------------- */

  __pgasrt_hbrd_barrier ();
  if (_initialized) return;
  __pgasrt_hbrd_barrier ();
  _initialized = true;
  for (int i=0; i<MAXCOMM; i++) _hbrdcomm[i][PGASRT_MYSMPTHREAD] = NULL;
  __pgasrt_hbrd_barrier ();
  
  /* ---------------------------------------- */
  /* allocate transport and smp communicators */
  /* ---------------------------------------- */

  if (PGASRT_MYSMPTHREAD==0)
    {
      tspcomm=(TSPColl::Communicator*)malloc(sizeof(TSPColl::Communicator));
      smpcomm=(SMPColl::Communicator*)malloc(sizeof(SMPColl::Communicator));
      if (!tspcomm || !smpcomm)
	__pgasrt_fatalerror (-1, "%s: malloc error", __FUNCTION__);
      new (tspcomm) TSPColl::Communicator;
      tspcomm->setup();
      new (smpcomm) SMPColl::Communicator;
    }

  /* ---------------------------------------- */
  /* allocate and assign hybrid communicator  */
  /* ---------------------------------------- */

  __pgasrt_hbrd_barrier ();

  HybridColl::Communicator * c;
  c = (HybridColl::Communicator *) malloc (sizeof(HybridColl::Communicator));  
  new (c) HybridColl::Communicator (PGASRT_MYTHREAD, PGASRT_THREADS, 
				    tspcomm, smpcomm);
  _hbrdcomm[0][PGASRT_MYSMPTHREAD] = c;

  __pgasrt_hbrd_barrier ();
}

/* ************************************************************************ */
/*               initialize a blocked communicator                          */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_comm_block   (int            newID,
						int            BF,
						int            ncomms)
{
  __pgasrt_hbrd_barrier ();


  /* ------------------------------------------- */
  /* use case 1: BF is divisible by threads/node */
  /* ------------------------------------------- */

  if (((BF / PGASRT_SMPTHREADS) * PGASRT_SMPTHREADS) == BF)
    {
      static SMPColl::Communicator * smpcomm;
      static TSPColl::Communicator * tspcomm;
      if (PGASRT_MYSMPTHREAD==0)
	{
	  int BF_tsp  = BF / PGASRT_SMPTHREADS;
	  int ncomms_tsp = ncomms;

	  tspcomm = 
	    (TSPColl::Communicator*)malloc(sizeof(TSPColl::BC_Comm));
	  smpcomm =
	    (SMPColl::Communicator*)malloc(sizeof(SMPColl::Communicator));
	  if (!tspcomm || !smpcomm)
	    __pgasrt_fatalerror (-1, "%s: malloc error", __FUNCTION__);

	  new (tspcomm) TSPColl::BC_Comm (BF_tsp, ncomms_tsp);
	  tspcomm->setup();
	  new (smpcomm) SMPColl::Communicator;
	}

      /* create the communicator */
      __pgasrt_hbrd_barrier ();
      HybridColl::Communicator * c = NULL;
      c = (HybridColl::Communicator *) malloc (sizeof(HybridColl::BC_Comm));
      if (!c) __pgasrt_fatalerror (-1, "%s: malloc error", __FUNCTION__);
      new (c) HybridColl::BC_Comm (BF, ncomms, tspcomm, smpcomm);
      _hbrdcomm[newID][PGASRT_MYSMPTHREAD] = c;
    }

  /* --------------------------------------------------------- */
  /* use case 2: BF==1 and ncomms is divisible by threads/node */
  /* --------------------------------------------------------- */
#if 0
  else if (BF==1 && ((ncomms/PGASRT_SMPTHREADS)*PGASRT_SMPTHREADS)==BF)
    {
      TSPColl::Communicator * tspcomm =
	(TSPColl::Communicator*)malloc(sizeof(TSPColl::BC_Comm));
      SMPColl::Communicator * smpcomm =
	(SMPColl::Communicator*)malloc(sizeof(SMPColl::Communicator));
      
      new (tspcomm) TSPColl::BC_Comm (BF_tsp, ncomms_tsp);
      new (smpcomm) SMPColl::Communicator;

      /* create the communicator */
      __pgasrt_hbrd_barrier ();
      HybridColl::Communicator * c = NULL;
      c = (HybridColl::Communicator *) malloc (sizeof(HybridColl::BC_Comm));
      if (!c) __pgasrt_fatalerror (-1, "%s: malloc error", __FUNCTION__);
      new (c) HybridColl::BC_Comm (BF, ncomms, tspcomm, smpcomm);
      _hbrdcomm[newID][PGASRT_MYSMPTHREAD] = c;
    } 
#endif

  else
    {
      __pgasrt_fatalerror (-1, "%s: blocking factor (currently %d)"
			   " must be divisible by threads/node (%d)",
			   __FUNCTION__, BF, PGASRT_SMPTHREADS);
    }

  __pgasrt_hbrd_barrier ();
}



/* ************************************************************************ */
/*                    communicator rank                                     */
/* ************************************************************************ */

extern "C" __pgasrt_thread_t __pgasrt_hbrdcoll_comm_rank (int commID)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  return (__pgasrt_thread_t)
    _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->rank();
}

/* ************************************************************************ */
/*                   communicator size                                      */
/* ************************************************************************ */

extern "C" __pgasrt_thread_t __pgasrt_hbrdcoll_comm_size (int commID)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  return (__pgasrt_thread_t)
    _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->size();
}

/* ************************************************************************ */
/*                    rank of any ID in a communicator                      */
/* ************************************************************************ */

extern "C" __pgasrt_thread_t
__pgasrt_hbrdcoll_comm_rankof (int commID, __pgasrt_thread_t rank)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  return (__pgasrt_thread_t)
    _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->absrankof(rank);
}


extern "C" __pgasrt_thread_t
__pgasrt_hbrdcoll_comm_to_tsp (int commID, __pgasrt_thread_t rank)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  return (__pgasrt_thread_t)
    _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->to_tsp(rank);
}


/* ************************************************************************ */
/*                   barrier implementation                                 */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_barrier (int commID)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->barrier();
}

/* ************************************************************************ */
/*                   allgather implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_allgather (int commID,
                                            const void * sbuf,
                                            void * rbuf, 
                                            size_t bufsize)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->
    allgather (sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                   allgatherv implementation                              */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_allgatherv (int commID,
					    const void * sbuf,
					    void * rbuf, 
					     size_t * lengths)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->
    allgatherv (sbuf, rbuf, lengths);
}

/* ************************************************************************ */
/*                   scatter   implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_scatter (int commID,
					  int root,
					  const void * sbuf,
					  void * rbuf, 
					  size_t bufsize)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->
    scatter (root, sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                   scatterv  implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_scatterv (int commID,
					   int root,
					   const void * sbuf,
					   void * rbuf, 
					   size_t * lengths)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->
    scatterv (root, sbuf, rbuf, lengths);
}

/* ************************************************************************ */
/*                   scatter   implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_gather (int commID,
					 int root,
					 const void * sbuf,
					 void * rbuf, 
					 size_t bufsize)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->
    gather (root, sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                   scatterv  implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_gatherv (int commID,
					  int root,
					  const void * sbuf,
					  void * rbuf, 
					  size_t * lengths)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->
    gatherv (root, sbuf, rbuf, lengths);
}

/* ************************************************************************ */
/*                   scatter   implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_bcast (int commID,
					int root,
					const void * sbuf,
					void * rbuf, 
					size_t bufsize)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->
    bcast (root, sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                    hybrid collective allreduce                           */
/* ************************************************************************ */

extern "C" void __pgasrt_hbrdcoll_allreduce   (int               commID,
					      const void      * sbuf,
					      void            * rbuf,
					      __pgasrt_ops_t    op,
					      __pgasrt_dtypes_t dtype,
					      unsigned          nelems)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_hbrdcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _hbrdcomm[commID][PGASRT_MYSMPTHREAD]->
    allreduce (sbuf,rbuf,op,dtype,nelems);
}

/* ************************************************************************ */
/*                   split a communicator                                   */
/* ************************************************************************ */

