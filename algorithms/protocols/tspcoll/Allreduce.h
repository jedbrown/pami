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

#ifndef __tspcoll_Allreduce_h__
#define __tspcoll_Allreduce_h__

#include "./CollExchange.h"
#include "interface/Communicator.h"

namespace CCMI { namespace Adaptor { namespace Allreduce {
      extern void getReduceFunction(CCMI_Dt, CCMI_Op, unsigned, 
				    unsigned&, CCMI_ReduceFunc&);
    }}};
namespace TSPColl
{
  namespace Allreduce
  {

    /* ******************************************************************* */
    /* ******************************************************************* */
    typedef void (*cb_Allreduce_t) (const void *, void *, unsigned);
    //    cb_Allreduce_t  getcallback (__pgasrt_ops_t, __pgasrt_dtypes_t);
    //    size_t          datawidthof (__pgasrt_dtypes_t);
    
    /* ******************************************************************* */
    /*      short allreduce (up to 1000 bytes of exchanged data)           */
    /* ******************************************************************* */

    class Short: public CollExchange
    {
    public:
      static const int MAXBUF = 1000;
      void * operator new (size_t, void * addr) { return addr; }
      Short (Communicator * comm, NBTag tag, int instID, int offset);
      void reset (const void        * s, 
		  void              * d,
		  CCMI_Op             op,
		  CCMI_Dt             dt,
		  unsigned            nelems);
    protected:
      static void cb_switchbuf (CollExchange *, unsigned phase);
      static void cb_allreduce (CollExchange *, unsigned phase);
    protected:
      int           _nelems, _logMaxBF;
      void        * _dbuf;
      //void       (* _cb_allreduce) (const void *, void *, unsigned);
      CCMI_ReduceFunc _cb_allreduce;
      char          _dummy;
      
    protected:
      typedef char PhaseBufType[MAXBUF] __attribute__((__aligned__(16)));
      PhaseBufType  _phasebuf[MAX_PHASES][2];   
      int           _bufctr  [MAX_PHASES]; /* 0 or 1 */
    }; /* Short Allreduce */

    /* ******************************************************************* */
    /* long allreduce (extra data buffer, message xfer permit protocol)    */
    /* ******************************************************************* */

    class Long: public CollExchange
    {
    public:
      void * operator new (size_t, void * addr) { return addr; }
      Long (Communicator * comm, NBTag tag, int instID, int offset);
      //      void reset (const void * s, void * d,
      //		  __pgasrt_ops_t op, __pgasrt_dtypes_t dt, unsigned nelems);
      void reset (const void * s, void * d,
		  CCMI_Op op, CCMI_Dt dt, unsigned nelems);

    protected:
      static void cb_allreduce (CollExchange *, unsigned phase);
      
    protected:
      int           _nelems, _logMaxBF;
      void        * _dbuf;
      //      void       (* _cb_allreduce) (const void *, void *, unsigned);
      CCMI_ReduceFunc _cb_allreduce;
      char          _dummy;
      void        * _tmpbuf;
    }; /* Long Allreduce */
  }; /* Allreduce */
}; /* TSPColl */

#endif /* __tspcoll_Allreduce_h__ */

