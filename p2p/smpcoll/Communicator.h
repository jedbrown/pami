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

#ifndef __smpcoll_communicator_h__
#define __smpcoll_communicator_h__

#include "Barrier.h"
#include "Allreduce.h"
#include "../tspcoll/Communicator.h"
#include <assert.h>
#include <string.h>


namespace SMPColl
{
  /* **************************************************************** */
  /*           Communicator base class                                */
  /* **************************************************************** */

  class Communicator
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    Communicator (int s=PGASRT_SMPTHREADS);
    // virtual ~Communicator() { }

  public:
    virtual int  size       (void)     const { return _size; }
    virtual int  rank       (void)     const { return PGASRT_MYSMPTHREAD; }

    int          split      (int color, int rank, int * proclist);

    virtual int  absrankof  (int rank) const { return rank; }
    virtual int  virtrankof (int rank) const { return rank; }

  public:

    virtual void     barrier     (void);
    virtual void     allgather   (const void *s, void *d, size_t l);
    virtual void     allgatherv  (const void *s, void *d, size_t *l);
    virtual void     bcast       (int root, const void *s, void *d, size_t l);
    virtual void     allreduce   (const void        * s,
                                  void              * d,
                                  __pgasrt_ops_t      op,
                                  __pgasrt_dtypes_t   dtype,
                                  unsigned            nelems);

    virtual void scatter    (int root, const void *s, void *d, size_t l);
    virtual void scatterv   (int root, const void *s, void *d, size_t *l);
    virtual void gather     (int root, const void *s, void *d, size_t l);
    virtual void gatherv    (int root, const void *s, void *d, size_t *l);

  public:
    
    void    setaddr (int rank, void * addr) { _barrier.setaddr (rank, addr); }
    void *  getaddr (int rank) const { return _barrier.getaddr (rank); }


  protected:
    int     _size;
    Barrier _barrier;
    Allreduce _allreduce;
  };

  /* ******************************************************************* */
  /*                     blocked communicator                            */
  /* ******************************************************************* */

  class BC_Comm: public Communicator
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    BC_Comm (int BF, int ncomms, int mycomm);

    virtual int  rank       (void)     const;
    virtual int  absrankof  (int rank) const;
    virtual int  virtrankof (int rank) const;

  protected:
    int _BF, _ncomms, _mycomm;
  };


};

#endif
