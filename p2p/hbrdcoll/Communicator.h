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

#ifndef __hbrdcoll_communicator_h__
#define __hbrdcoll_communicator_h__

#include "../smpcoll/Communicator.h"
#include "../tspcoll/Communicator.h"

/* ************************************************************************ */
/*           Hybrid communicator base class                                 */
/* ************************************************************************ */

namespace HybridColl
{
  class Communicator
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    Communicator (int r, int s, 
		  TSPColl::Communicator *,
		  SMPColl::Communicator *);

  public:
    /* -------------------- */
    /* communicator inquiry */
    /* -------------------- */

    virtual int  size       (void)     const { return _size; }
    virtual int  rank       (void)     const { return _rank; }

    /* ------------------------------ */
    /* communicator rank translations */
    /* ------------------------------ */

    virtual int  absrankof  (int rank) const { return rank; }
    virtual int  virtrankof (int rank) const { return rank; }
    virtual int  to_tsp     (int rank) const { return rank/PGASRT_SMPTHREADS; }

    /* collectives */

    virtual void barrier    (void);
    virtual void allgather  (const void *s, void *d, size_t l);
    virtual void allgatherv (const void *s, void *d, size_t *l);
    virtual void scatter    (int root, const void *s, void *d, size_t l);
    virtual void scatterv   (int root, const void *s, void *d, size_t *l);
    virtual void gather     (int root, const void *s, void *d, size_t l);
    virtual void gatherv    (int root, const void *s, void *d, size_t *l);
    virtual void bcast      (int root, const void *s, void *d, size_t l);
    virtual void allreduce  (const void *s, void *d, 
			     __pgasrt_ops_t op, __pgasrt_dtypes_t dtype, 
			     unsigned nelems);

  protected:
    int                     _rank, _size;
    TSPColl::Communicator * _tspcomm;
    SMPColl::Communicator * _smpcomm;
  };

  /* ********************************************************************* */
  /*                   blocked communicator                                */
  /* ********************************************************************* */

  class BC_Comm: public Communicator
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    BC_Comm (int BF, int ncomms,
	     TSPColl::Communicator *,
	     SMPColl::Communicator *);
  
    virtual int  absrankof  (int rank) const;
    virtual int  virtrankof (int rank) const;
    virtual int  to_tsp     (int rank) const;

  protected:
    int _BF, _ncomms, _mycomm;
  };
}

#endif
