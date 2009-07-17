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

#ifndef __tspcoll_genericcomm_h__
#define __tspcoll_genericcomm_h__

#include "interface/Communicator.h"
#include "algorithms/protocols/tspcoll/Barrier.h"
#include "algorithms/protocols/tspcoll/Allgather.h"
#include "algorithms/protocols/tspcoll/Allgatherv.h"
#include "algorithms/protocols/tspcoll/Scatter.h"
#include "algorithms/protocols/tspcoll/BinomBcast.h"
#include "algorithms/protocols/tspcoll/ScBcast.h"
#include "algorithms/protocols/tspcoll/Allreduce.h"
#include "algorithms/protocols/tspcoll/NBColl.h"


namespace TSPColl
{
  
  class NBColl; /* any non-blocking collective */
    
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
}
#endif
