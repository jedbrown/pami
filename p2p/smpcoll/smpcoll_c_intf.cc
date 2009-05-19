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

#include <stdio.h>

#undef TRACE
//#define TRACE(x) fprintf x
#define TRACE(x)

#define MAXCOMM 16
#define MAXTPN 64
static SMPColl::Communicator * _smpcomm [MAXCOMM][MAXTPN];
static bool _initialized = false;

/* ************************************************************************ */
/*                    set up a new communicator                             */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_comm_init ()
{
  __pgasrt_smp_barrier ();

  /* ------------------------- */
  /* Check initialization flag */
  /* ------------------------- */

  if (PGASRT_MYSMPTHREAD==0)
    {
      if (_initialized)  __pgasrt_fatalerror (-1,"smpcoll initialized twice");
      _initialized = true;
      for (int i=0; i<MAXCOMM; i++)
	for (int j=0; j<MAXTPN; j++)
	  _smpcomm[i][j] = NULL;
      
      SMPColl::Communicator * c;
      c = (SMPColl::Communicator *) malloc (sizeof(SMPColl::Communicator));
      if (!c) __pgasrt_fatalerror (-1, "%s: malloc error", __FUNCTION__);
      new (c) SMPColl::Communicator;
      for (unsigned j=0; j<PGASRT_SMPTHREADS; j++) _smpcomm[0][j] = c;
    }

  __pgasrt_smp_barrier ();
}

/* ************************************************************************ */
/*                    communicator rank                                     */
/* ************************************************************************ */

extern "C" __pgasrt_thread_t __pgasrt_smpcoll_comm_rank (int commID)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  return (__pgasrt_thread_t) 
    _smpcomm[commID][PGASRT_MYSMPTHREAD]->rank();
}

/* ************************************************************************ */
/*                    rank of any ID in a communicator                      */
/* ************************************************************************ */

extern "C" __pgasrt_thread_t 
__pgasrt_smpcoll_comm_rankof (int commID, __pgasrt_thread_t rank)
{
  return (__pgasrt_thread_t) 
    _smpcomm[commID][PGASRT_MYSMPTHREAD]->absrankof(rank);
}

/* ************************************************************************ */
/*                   communicator size                                      */
/* ************************************************************************ */

extern "C" __pgasrt_thread_t __pgasrt_smpcoll_comm_size (int commID)
{
  return (__pgasrt_thread_t) 
    _smpcomm[commID][PGASRT_MYSMPTHREAD]->size();
}

/* ************************************************************************ */
/*                   split a communicator                                   */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_comm_split   (int            commID,
					       int            newID,
					       int            color,
					       int            rank)
{
  assert (0);
}

/* ************************************************************************ */
/*                   blocked subcommunicator                                */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_comm_block (int               newID,
					     int               BF,
					     int               ncomms)
{
  _smpcomm[0][PGASRT_MYSMPTHREAD]->barrier();
  int bpc = PGASRT_SMPTHREADS / (BF * ncomms);
  if ((int)(bpc * BF * ncomms) != (int)PGASRT_SMPTHREADS)
    __pgasrt_fatalerror (-1, "%s (%d,%d,%d): Invalid arguments: "
			 "TPN=%d is not a multiple of BF=%d * ncomms=%d",
			 __FUNCTION__, newID, BF, ncomms,
			 PGASRT_SMPTHREADS, BF, ncomms);
  if (PGASRT_MYSMPTHREAD==0)
    {
      SMPColl::Communicator * c = NULL;
      for (int i=0; i<ncomms; i++)
	{
	  c = (SMPColl::Communicator *) malloc (sizeof(TSPColl::BC_Comm));
	  if (!c) __pgasrt_fatalerror (-1, "%s: malloc error", __FUNCTION__);
	  new (c) SMPColl::BC_Comm (BF, ncomms, i);

	  for (int u=i*BF; u < (int)PGASRT_SMPTHREADS; u += ncomms * BF)
	    for (int v=0; v < (int)BF && u+v < (int)PGASRT_SMPTHREADS; v++)
	      _smpcomm[newID][u+v] = c;
	}
    }
  _smpcomm[0][PGASRT_MYSMPTHREAD]->barrier();
}

/* ************************************************************************ */
/*                   barrier implementation                                 */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_barrier (int commID)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _smpcomm[commID][PGASRT_MYSMPTHREAD]->
    barrier();
}

/* ************************************************************************ */
/*                   allgather implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_allgather (int commID,
					    const void * sbuf,
					    void * rbuf, 
					    size_t bufsize)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _smpcomm[commID][PGASRT_MYSMPTHREAD]->
    allgather (sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                   allgatherv implementation                              */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_allgatherv (int           commID,
					     const void  * sbuf,
					     void        * rbuf, 
					     size_t      * lengths)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _smpcomm[commID][PGASRT_MYSMPTHREAD]->
    allgatherv (sbuf, rbuf, lengths);
}

/* ************************************************************************ */
/*                   scatter   implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_scatter (int             commID,
					  int             root,
					  const void    * sbuf,
					  void          * rbuf, 
					  size_t          bufsize)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _smpcomm[commID][PGASRT_MYSMPTHREAD]->
    scatter (root, sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                   scatterv  implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_scatterv (int             commID,
					   int             root,
					   const void    * sbuf,
					   void          * rbuf, 
					   size_t        * lengths)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _smpcomm[commID][PGASRT_MYSMPTHREAD]->
    scatterv (root, sbuf, rbuf, lengths);
}

/* ************************************************************************ */
/*                   scatter   implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_gather (int              commID,
					 int              root,
					 const void     * sbuf,
					 void           * rbuf, 
					 size_t           bufsize)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _smpcomm[commID][PGASRT_MYSMPTHREAD]->
    gather (root, sbuf, rbuf, bufsize);
}

/* ************************************************************************ */
/*                   scatterv  implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_gatherv (int             commID,
					  int             root,
					  const void    * sbuf,
					  void          * rbuf, 
					  size_t        * lengths)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _smpcomm[commID][PGASRT_MYSMPTHREAD]->
    gatherv (root, sbuf, rbuf, lengths);
}

/* ************************************************************************ */
/*                   scatter   implementation                               */
/* ************************************************************************ */

extern "C" void __pgasrt_smpcoll_bcast (int            commID,
					int            root,
					const void   * sbuf,
					void         * rbuf, 
					size_t         bufsize)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _smpcomm[commID][PGASRT_MYSMPTHREAD]->
    bcast (root, sbuf, rbuf, bufsize);
}

extern "C" void __pgasrt_smpcoll_allreduce   (int               commID,
					      const void      * sbuf,
					      void            * rbuf,
					      __pgasrt_ops_t    op,
					      __pgasrt_dtypes_t dtype,
					      unsigned          nelems)
{
  assert (0 <= commID && commID < MAXCOMM);
  assert (_smpcomm[commID][PGASRT_MYSMPTHREAD] != NULL);
  _smpcomm[commID][PGASRT_MYSMPTHREAD]->
    allreduce (sbuf,rbuf,op,dtype,nelems);
}

