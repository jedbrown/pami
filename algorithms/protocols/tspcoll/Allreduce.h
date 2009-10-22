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
/**
 * \file algorithms/protocols/tspcoll/Allreduce.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_Allreduce_h__
#define __algorithms_protocols_tspcoll_Allreduce_h__

#include "algorithms/protocols/tspcoll/CollExchange.h"
#include "math/math_coremath.h"
#include <string.h>

namespace CCMI { namespace Adaptor { namespace Allreduce {
      extern void getReduceFunction(xmi_dt, xmi_op, unsigned,
				    unsigned&, coremath&);
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

    template<class T_Mcast>
    class Short: public CollExchange<T_Mcast>
    {
    public:
      static const int MAXBUF = 1000;
      void * operator new (size_t, void * addr) { return addr; }
      Short(XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int offset);
      void reset (const void        * s,
		  void              * d,
		  xmi_op             op,
		  xmi_dt             dt,
		  unsigned            nelems);
    protected:
      static void cb_switchbuf (CollExchange<T_Mcast> *, unsigned phase);
      static void cb_allreduce (CollExchange<T_Mcast> *, unsigned phase);
    protected:
      int           _nelems, _logMaxBF;
      void        * _dbuf;
      //void       (* _cb_allreduce) (const void *, void *, unsigned);
      coremath _cb_allreduce;
      char          _dummy;

    protected:
      typedef char PhaseBufType[MAXBUF] __attribute__((__aligned__(16)));
      PhaseBufType  _phasebuf[MAX_PHASES][2];
      int           _bufctr  [MAX_PHASES]; /* 0 or 1 */
    }; /* Short Allreduce */

    /* ******************************************************************* */
    /* long allreduce (extra data buffer, message xfer permit protocol)    */
    /* ******************************************************************* */
    template<class T_Mcast>
    class Long: public CollExchange<T_Mcast>
    {
    public:
      void * operator new (size_t, void * addr) { return addr; }
      Long (XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int offset);
      //      void reset (const void * s, void * d,
      //		  __pgasrt_ops_t op, __pgasrt_dtypes_t dt, unsigned nelems);
      void reset (const void * s, void * d,
		  xmi_op op, xmi_dt dt, unsigned nelems);

    protected:
      static void cb_allreduce (CollExchange<T_Mcast> *, unsigned phase);

    protected:
      int           _nelems, _logMaxBF;
      void        * _dbuf;
      //      void       (* _cb_allreduce) (const void *, void *, unsigned);
      coremath _cb_allreduce;
      char          _dummy;
      void        * _tmpbuf;
    }; /* Long Allreduce */
  }; /* Allreduce */
}; /* TSPColl */


//#define DEBUG_ALLREDUCE 1
#undef TRACE
#ifdef DEBUG_ALLREDUCE
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* ************************************************************************* */
/*                      start a short allreduce                              */
/* ************************************************************************* */
template <class T_Mcast>
TSPColl::Allreduce::Short<T_Mcast>::
Short (XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int offset) :
       CollExchange<T_Mcast> (comm, tag, instID, offset, false)
{
  _dbuf   = NULL;
  _nelems = 0;
  for (_logMaxBF = 0; (1<<(_logMaxBF+1)) <= this->_comm->size(); _logMaxBF++) ;
  int maxBF  = 1<<_logMaxBF;
  int nonBF  = this->_comm->size() - maxBF;
  int phase  = 0;
  int rank   = comm->virtrank();
  /* -------------------------------------------- */
  /* phase 0: gather buffers from ranks > n2prev  */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      unsigned rdest = comm->absrankof (rank - maxBF);
      this->_dest    [phase] = (rank >= maxBF) ? rdest : -1;
      this->_sbuf    [phase] = NULL;    /* unknown */
      this->_rbuf    [phase] = (rank < nonBF)  ? _phasebuf[phase][0] : NULL;
      this->_cb_recv1[phase] = (rank < nonBF)  ? cb_switchbuf : NULL;
      this->_cb_recv2[phase] = (rank < nonBF)  ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;       /* unknown */
      this->_bufctr  [phase] = 0;
      phase ++;
    }

  /* -------------------------------------------- */
  /* butterfly phases                             */
  /* -------------------------------------------- */

  for (int i=0; i<_logMaxBF; i++)
    {
      unsigned rdest   = comm->absrankof (rank ^ (1<<i));
      this->_dest    [phase] = (rank < maxBF) ? rdest : -1;
      this->_sbuf    [phase] = NULL;     /* unknown */
      this->_rbuf    [phase] = (rank < maxBF) ? _phasebuf[phase][0] : NULL;
      this->_cb_recv1[phase] = (rank < maxBF) ? cb_switchbuf : NULL;
      this->_cb_recv2[phase] = (rank < maxBF) ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;        /* unknown */
      this->_bufctr  [phase] = 0;
      phase ++;
    }

  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      unsigned rdest   = comm->absrankof (rank + maxBF);
      this->_dest    [phase] = (rank < nonBF)  ? rdest : -1;
      this->_sbuf    [phase] = NULL;     /* unknown */
      this->_rbuf    [phase] = NULL;     /* unknown */
      this->_cb_recv1[phase] = NULL;
      this->_cb_recv2[phase] = NULL;
      this->_sbufln  [phase] = 0;        /* unknown */
      this->_bufctr  [phase] = 0;
      phase ++;
    }

  this->_numphases    = phase;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

/* ************************************************************************* */
/*                     allreduce executor                                    */
/* ************************************************************************* */
template <class T_Mcast>
void TSPColl::Allreduce::Short<T_Mcast>::
cb_allreduce (CollExchange<T_Mcast> *coll, unsigned phase)
{
  TSPColl::Allreduce::Short<T_Mcast> * ar = (TSPColl::Allreduce::Short<T_Mcast> *) coll;
  int c = (ar->_counter+1) & 1;
  void * inputs[] = {ar->_dbuf, ar->_phasebuf[phase][c]};
  //  ar->_cb_allreduce (ar->_dbuf, ar->_phasebuf[phase][c], ar->_nelems);
  ar->_cb_allreduce (ar->_dbuf, inputs, 2, ar->_nelems);


}

template <class T_Mcast>
void TSPColl::Allreduce::Short<T_Mcast>::
cb_switchbuf (CollExchange<T_Mcast> * coll, unsigned phase)
{
  TSPColl::Allreduce::Short<T_Mcast> * ar = (TSPColl::Allreduce::Short<T_Mcast> *) coll;
  int c = (++(ar->_bufctr[phase])) & 1;
  ar->_rbuf[phase] = ar->_phasebuf[phase][c];
}

/* ************************************************************************* */
/*                     start an allreduce operation                          */
/* ************************************************************************* */
template <class T_Mcast>
void TSPColl::Allreduce::Short<T_Mcast>::reset (const void         * sbuf,
				       void               * dbuf,
				       xmi_op              op,
				       xmi_dt              dt,
				       unsigned             nelems)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);

  /* --------------------------------------------------- */
  /*         copy source to destination if necessary     */
  /* --------------------------------------------------- */

  _dbuf   = dbuf;
  _nelems = nelems;
  unsigned datawidth;
  //  size_t datawidth = datawidthof (dt);
  CCMI::Adaptor::Allreduce::getReduceFunction(dt, op, nelems, datawidth,_cb_allreduce);
  assert (nelems * datawidth < sizeof(PhaseBufType));
  if (sbuf != dbuf) memcpy (dbuf, sbuf, nelems * datawidth);

  int maxBF = 1<<_logMaxBF; /* largest power of 2 that fits into comm */
  int nonBF = this->_comm->size() - maxBF; /* comm->size() - largest power of 2 */
  int rank  = this->_comm->virtrank();
  /* -------------------------------------------- */
  /* phase 0: gather buffers from ranks > n2prev  */
  /* -------------------------------------------- */

  int phase=0;
  if (nonBF > 0)   /* phase 0: gather buffers from ranks > n2prev */
    {
      this->_sbuf    [phase] = (rank >= maxBF) ? dbuf  : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  /* -------------------------------------------- */
  /* butterfly phases                             */
  /* -------------------------------------------- */

  for (int i=0; i<_logMaxBF; i++)   /* middle phases: butterfly pattern */
    {
      this->_sbuf    [phase] = (rank < maxBF) ? dbuf  : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }


  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)   /*  last phase: collect results */
    {
      this->_sbuf    [phase] = (rank < nonBF)  ? dbuf  : NULL;
      this->_rbuf    [phase] = (rank >= maxBF) ? dbuf  : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  assert (phase == this->_numphases);

#ifdef DEBUG_ALLREDUCE
  printf ("%d: ", rank);
  for (int i=0; i<this->_numphases; i++)
    printf ("[%d %s] ", this->_dest[i], this->_rbuf[i]? "Y" : "N");
  printf ("\n");
#endif

  //  _cb_allreduce = getcallback (op, dt);
  TSPColl::CollExchange<T_Mcast>::reset();
}


//#define DEBUG_ALLREDUCE 1
#undef TRACE
#ifdef DEBUG_ALLREDUCE
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* ************************************************************************* */
/*                       start a long allreduce                              */
/* ************************************************************************* */
template <class T_Mcast>
TSPColl::Allreduce::Long<T_Mcast>::
Long (XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int offset) :
  CollExchange<T_Mcast> (comm, tag, instID, offset, false)
{
  _tmpbuf = NULL;
  _dbuf = NULL;
  _nelems = 0;
  for (_logMaxBF = 0; (1<<(_logMaxBF+1)) <= this->_comm->size(); _logMaxBF++) ;
  int maxBF  = 1<<_logMaxBF;
  int nonBF  = this->_comm->size() - maxBF;
  int phase  = 0;
  int rank   = this->_comm->virtrank();
  /* -------------------------------------------- */
  /* phase 0: gather buffers from ranks > n2prev  */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      /* send permission chits to potential senders */

      unsigned rdest = this->_comm->absrankof (rank + maxBF);
      this->_dest    [phase] = (rank <  nonBF) ? rdest : -1;
      this->_sbuf    [phase] = (rank <  nonBF) ? &_dummy : NULL;
      this->_rbuf    [phase] = (rank >= maxBF) ? &_dummy : NULL;
      this->_cb_recv2[phase] = NULL;
      this->_sbufln  [phase] = 1;
      phase ++;

      /* send data */

      rdest = this->_comm->absrankof (rank - maxBF);
      this->_dest    [phase] = (rank >= maxBF) ? rdest : -1;
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_cb_recv2[phase] = (rank < nonBF)  ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;    /* data length not available */
      phase ++;
    }

  /* -------------------------------------------- */
  /* butterfly phase                              */
  /* -------------------------------------------- */

  for (int i=0; i<_logMaxBF; i++)
    {
      /* send permission chits to senders */

      unsigned rdest   = this->_comm->absrankof (rank ^ (1<<i));
      this->_dest    [phase] = (rank < maxBF) ? rdest : -1;
      this->_sbuf    [phase] = (rank < maxBF) ? &_dummy : NULL;
      this->_rbuf    [phase] = (rank < maxBF) ? &_dummy : NULL;
      this->_cb_recv2[phase] = NULL;
      this->_sbufln  [phase] = 1;
      phase ++;

      /* send data */

      this->_dest    [phase] = (rank < maxBF) ? rdest : -1;
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_cb_recv2[phase] = (rank < maxBF) ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;    /* data length not available */
      phase ++;
    }

  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      /* send permission slips */

      unsigned rdest = this->_comm->absrankof (rank - maxBF);
      this->_dest    [phase] = (rank >= maxBF) ? rdest : -1;
      this->_sbuf    [phase] = (rank >= maxBF) ? &_dummy : NULL;
      this->_rbuf    [phase] = (rank < nonBF)  ? &_dummy : NULL;
      this->_cb_recv2[phase] = NULL;
      this->_sbufln  [phase] = 1;
      phase ++;

      /* send data */

      rdest   = this->_comm->absrankof (rank + maxBF);
      this->_dest    [phase] = (rank < nonBF)  ? rdest : -1;
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_cb_recv2[phase] = NULL;
      this->_sbufln  [phase] = 0; /* data length not available */
      phase ++;
    }

  this->_numphases    = phase;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

/* ************************************************************************* */
/*                     allreduce executor                                    */
/* ************************************************************************* */
template <class T_Mcast>
void TSPColl::Allreduce::Long<T_Mcast>::
cb_allreduce (CollExchange<T_Mcast> *coll, unsigned phase)
{
  TSPColl::Allreduce::Long<T_Mcast> * ar = (TSPColl::Allreduce::Long<T_Mcast> *) coll;
  void * inputs[] = {ar->_dbuf, ar->_tmpbuf};
  //  ar->_cb_allreduce (ar->_dbuf, ar->_tmpbuf, ar->_nelems);
  ar->_cb_allreduce (ar->_dbuf, inputs, 2, ar->_nelems);
}

/* ************************************************************************* */
/*                      start a long allreduce operation                     */
/* ************************************************************************* */
template <class T_Mcast>
void TSPColl::Allreduce::Long<T_Mcast>::reset (const void         * sbuf,
				      void               * dbuf,
				      xmi_op              op,
				      xmi_dt              dt,
				      unsigned             nelems)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);

  /* --------------------------------------------------- */
  /*         copy source to destination if necessary     */
  /* --------------------------------------------------- */

  _dbuf   = dbuf;
  _nelems = nelems;
  //  size_t datawidth = datawidthof (dt);
  unsigned datawidth;
  CCMI::Adaptor::Allreduce::getReduceFunction(dt, op, nelems, datawidth,_cb_allreduce);
  if (sbuf != dbuf) memcpy (dbuf, sbuf, nelems * datawidth);
  if (_tmpbuf) free (_tmpbuf); _tmpbuf = malloc (nelems * datawidth);
  if (!_tmpbuf) CCMI_FATALERROR (-1, "Allreduce: memory allocation error");

  /* --------------------------------------------------- */
  /*  set source and destination buffers and node ids    */
  /* We deal with non-power-of-2 nodes                   */
  /* --------------------------------------------------- */

  int maxBF  = 1<<_logMaxBF;
  int nonBF  = this->_comm->size() - maxBF;
  int phase  = 0;
  int rank   = this->_comm->virtrank();

  if (nonBF > 0)   /* phase 0: gather buffers from ranks > n2prev */
    {
      phase ++;
      this->_sbuf    [phase] = (rank >= maxBF) ? _dbuf : NULL;
      this->_rbuf    [phase] = (rank < nonBF)  ? _tmpbuf : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  for (int i=0; i<_logMaxBF; i++)   /* middle phases: butterfly pattern */
    {
      phase ++;
      this->_sbuf    [phase] = (rank < maxBF) ? _dbuf : NULL;
      this->_rbuf    [phase] = (rank < maxBF) ? _tmpbuf : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  if (nonBF > 0)   /*  last phase: collect results */
    {
      phase ++;
      this->_sbuf    [phase] = (rank < nonBF)  ? _dbuf  : NULL;
      this->_rbuf    [phase] = (rank >= maxBF) ? _dbuf  : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  assert (phase == this->_numphases);
  //  _cb_allreduce = getcallback (op, dt);
  TSPColl::CollExchange<T_Mcast>::reset();
}




#endif /* __tspcoll_Allreduce_h__ */
