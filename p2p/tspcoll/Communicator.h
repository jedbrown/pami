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

#ifndef __tspcoll_basecomm_h__
#define __tspcoll_basecomm_h__

#include "../common/include/pgasrt.h"

namespace TSPColl
{

  class NBColl; /* any non-blocking collective */

  /* ******************************************************************* */
  /*           Communicator base class                                   */
  /* ******************************************************************* */

  class Communicator
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    Communicator (int r=PGASRT_MYNODE, int s=PGASRT_NODES);
    // virtual ~Communicator () { }
    virtual void setup () ;

  public:

    /* -------------------- */
    /* communicator inquiry */
    /* -------------------- */

    virtual int  size       (void)     const { return _size; }
    virtual int  rank       (void)     const { return _rank; }

    /* ---------------------- */
    /* communicator utilities */
    /* ---------------------- */

    int          split      (int color, int rank, int * proclist);

    /* ------------------------------------------ */
    /* communicator to absolute rank translations */
    /* ------------------------------------------ */

    virtual int  absrankof  (int rank) const { return rank; }
    virtual int  virtrankof (int rank) const { return rank; }

    /* -------------------- */
    /* collectives          */
    /* -------------------- */

    virtual NBColl * ibarrier    (void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     barrier     (void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iallgather  (const void *s, void *d, size_t l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     allgather   (const void *s, void *d, size_t l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iallgatherv (const void *s, void *d, size_t *l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     allgatherv  (const void *s, void *d, size_t *l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * ibcast      (int root, const void *s, void *d, size_t l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     bcast       (int root, const void *s, void *d, size_t l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iallreduce  (const void        * s,
				  void              * d, 
				  __pgasrt_ops_t      op,
				  __pgasrt_dtypes_t   dtype, 
				  unsigned            nelems,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     allreduce   (const void        * s, 
				  void              * d, 
				  __pgasrt_ops_t      op,
				  __pgasrt_dtypes_t   dtype, 
				  unsigned            nelems,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iscatter   (int root, const void *s, void *d, size_t l,
				 void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     scatter    (int root, const void *s, void *d, size_t l,
				 void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iscatterv  (int root, const void *s, void *d, size_t *l,
				 void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     scatterv   (int root, const void *s, void *d, size_t *l,
				 void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual void     gather     (int root, const void *s, void *d, size_t l);
    virtual void     gatherv    (int root, const void *s, void *d, size_t *l);

    virtual void     nbwait     (NBColl *);
    
  protected:  
    int _rank, _size;

    NBColl * _barrier;
    NBColl * _allgather;
    NBColl * _allgatherv;
    NBColl * _bcast, *_bcast2;
    NBColl * _sar, * _lar;
    NBColl * _sct, * _sctv;
  };
    
  /* ******************************************************************* */
  /*                      enumerated communicator                        */
  /* ******************************************************************* */

  class EnumComm: public Communicator
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    EnumComm (int rank, int size, int proclist[]);
    virtual int  absrankof (int rank) const { return _proclist[rank]; }
    
  protected:  
    int * _proclist; /* list of absolute (node) ranks */
  };

  /* ******************************************************************* */
  /*                      ranged communicator                            */
  /* ******************************************************************* */

  class Range
  {
  public:
      size_t _lo;
      size_t _hi;
  };

  class RangedComm: public Communicator
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    RangedComm (int rank, int numranges, Range rangelist[]);
    virtual int  absrankof (int rank) const;
    virtual int  virtrankof (int rank) const;
  protected:
    int     _numranges;
    Range * _rangelist; /* list of absolute (node) ranks */
  };



  /* ******************************************************************* */
  /*                     blocked communicator                            */
  /* ******************************************************************* */

  class BC_Comm: public Communicator
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    BC_Comm (int BF, int ncomms);

    virtual int  absrankof  (int rank) const;
    virtual int  virtrankof (int rank) const;

  protected:
    int _BF, _ncomms, _mycomm;
  };
}

#endif
