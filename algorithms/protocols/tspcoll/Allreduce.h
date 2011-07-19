/**
 * \file algorithms/protocols/tspcoll/Allreduce.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_Allreduce_h__
#define __algorithms_protocols_tspcoll_Allreduce_h__

#include "algorithms/protocols/tspcoll/CollExchange.h"

namespace xlpgas
{

  namespace Allreduce
    {
      /* ******************************************************************* */
      /*               allreduce execution functions                         */
      /* ******************************************************************* */

      typedef coremath cb_Allreduce_t;
      inline cb_Allreduce_t  getcallback (xlpgas_ops_t, xlpgas_dtypes_t);
      inline size_t          datawidthof (xlpgas_dtypes_t);

      /* ******************************************************************* */
      /* ******************************************************************* */
      template <class T_NI>
      class Long: public CollExchange<T_NI>
	{
	public:
	  void * operator new (size_t, void * addr) { return addr; }
	  Long (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset);
          ~Long()
            {
              if(_tmpbuf)
                __global.heap_mm->free(_tmpbuf);
            }
	  void reset (const void * s, void * d,
		      xlpgas_ops_t op, xlpgas_dtypes_t dt, unsigned nelems, user_func_t* uf);

	protected:
	  static void cb_allreduce (CollExchange<T_NI> *, unsigned phase);

	protected:
	  int           _nelems, _logMaxBF;
	  void        * _dbuf;
          coremath      _cb_allreduce;
	  char          _dummy;
	  void        * _tmpbuf;
	  size_t        _tmpbuflen;
	  user_func_t* _uf;
	}; /* Long Allreduce */

    template <class T_NI>
    class Short: public CollExchange<T_NI>
    {
    public:
      static const int MAXBUF = 512;
      void * operator new (size_t, void * addr) { return addr; }
      Short (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset);
      void reset (const void        * s,
		  void              * d,
		  xlpgas_ops_t         op,
		  xlpgas_dtypes_t     dt,
		  unsigned            nelems,
		  user_func_t*        uf);

    protected:
      static xlpgas_local_addr_t cb_switchbuf (CollExchange<T_NI> *, unsigned phase, unsigned counter);
      static void cb_allreduce (CollExchange<T_NI> *, unsigned phase);

    protected:
      int           _nelems, _logMaxBF;
      void        * _dbuf;
      coremath      _cb_allreduce;
      char          _dummy;
      user_func_t*  _uf;

    protected:
      typedef char PhaseBufType[MAXBUF] __attribute__((__aligned__(16)));
      PhaseBufType  _phasebuf[CollExchange<T_NI>::MAX_PHASES][2];
      int           _bufctr  [CollExchange<T_NI>::MAX_PHASES]; /* 0 or 1 */
    }; /* Short Allreduce */

  }//end Allreduce

} /* Xlpgas */

#include "algorithms/protocols/tspcoll/LongAllreduce.cc"
#include "algorithms/protocols/tspcoll/ShortAllreduce.cc"
#include "algorithms/protocols/tspcoll/Allreduce.cc"

#endif /* __xlpgas_Allreduce_h__ */
