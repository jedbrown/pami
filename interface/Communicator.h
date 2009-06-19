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

#include "collectives/util/ccmi_debug.h"
#include "collectives/interface/MultiSendOld.h"

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
    Communicator (int r, int s);
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

    virtual NBColl * ibarrier    (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     barrier     (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iallgather  (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  const void *s, void *d, size_t l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     allgather   (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  const void *s, void *d, size_t l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iallgatherv (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  const void *s, void *d, size_t *l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     allgatherv  (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  const void *s, void *d, size_t *l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * ibcast      (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  int root, const void *s, void *d, size_t l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     bcast       (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  int root, const void *s, void *d, size_t l,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iallreduce  (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  const void        * s,
				  void              * d, 
				  CCMI_Op             op,
				  CCMI_Dt             dtype, 
				  unsigned            nelems,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     allreduce   (CCMI::MultiSend::MulticastInterface *mcast_iface,
				  const void        * s, 
				  void              * d, 
				  CCMI_Op             op,
				  CCMI_Dt             dtype, 
				  unsigned            nelems,
				  void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iscatter   (CCMI::MultiSend::MulticastInterface *info_barrier,
				 CCMI::MultiSend::MulticastInterface *info_scatter,
				 int root, const void *s, void *d, size_t l,
				 void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     scatter    (CCMI::MultiSend::MulticastInterface *info_barrier,
				 CCMI::MultiSend::MulticastInterface *info_scatter,
				 int root, const void *s, void *d, size_t l,
				 void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual NBColl * iscatterv  (CCMI::MultiSend::MulticastInterface *info_barrier,
				 CCMI::MultiSend::MulticastInterface *info_scatter,
				 int root, const void *s, void *d, size_t *l,
				 void (*cb_complete)(void *)=NULL, void *arg=NULL);
    virtual void     scatterv   (CCMI::MultiSend::MulticastInterface *info_barrier,
				 CCMI::MultiSend::MulticastInterface *info_scatter,
				 int root, const void *s, void *d, size_t *l,
				 void (*cb_complete)(void *)=NULL, void *arg=NULL);

    virtual void     gather     (CCMI::MultiSend::MulticastInterface *mcast_iface,
				 int root, const void *s, void *d, size_t l);
    virtual void     gatherv    (CCMI::MultiSend::MulticastInterface *mcast_iface,
				 int root, const void *s, void *d, size_t *l);

    virtual void     nbwait     (NBColl *);
    
  protected:  
    NBColl * _barrier;
    NBColl * _allgather;
    NBColl * _allgatherv;
    NBColl * _bcast, *_bcast2;
    NBColl * _sar, * _lar;
    NBColl * _sct, * _sctv;
    int _rank, _size;
  };
}

#endif
