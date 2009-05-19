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

#include "./Allreduce.h"
#include "./Communicator.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

TSPColl::Allreduce::Short::
Short (Communicator * comm, NBTag tag, int instID, int offset) :
  CollExchange (comm, tag, instID, offset, false)
{
  _dbuf   = NULL;
  _nelems = 0;
  for (_logMaxBF = 0; (1<<(_logMaxBF+1)) <= _comm->size(); _logMaxBF++) ;
  int maxBF  = 1<<_logMaxBF;
  int nonBF  = _comm->size() - maxBF;
  int phase=0;

  /* -------------------------------------------- */
  /* phase 0: gather buffers from ranks > n2prev  */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      unsigned rdest = comm->absrankof (comm->rank() - maxBF);
      _dest    [phase] = (comm->rank() >= maxBF) ? rdest : -1;
      _sbuf    [phase] = NULL;    /* unknown */
      _rbuf    [phase] = (comm->rank() < nonBF)  ? _phasebuf[phase][0] : NULL;
      _cb_recv1[phase] = (comm->rank() < nonBF)  ? cb_switchbuf : NULL;
      _cb_recv2[phase] = (comm->rank() < nonBF)  ? cb_allreduce : NULL;
      _sbufln  [phase] = 0;       /* unknown */
      _bufctr  [phase] = 0;
      phase ++;
    }
  
  /* -------------------------------------------- */
  /* butterfly phases                             */
  /* -------------------------------------------- */

  for (int i=0; i<_logMaxBF; i++)
    {
      unsigned rdest   = comm->absrankof (comm->rank() ^ (1<<i));
      _dest    [phase] = (comm->rank() < maxBF) ? rdest : -1;
      _sbuf    [phase] = NULL;     /* unknown */
      _rbuf    [phase] = (comm->rank() < maxBF) ? _phasebuf[phase][0] : NULL;
      _cb_recv1[phase] = (comm->rank() < maxBF) ? cb_switchbuf : NULL;
      _cb_recv2[phase] = (comm->rank() < maxBF) ? cb_allreduce : NULL;
      _sbufln  [phase] = 0;        /* unknown */
      _bufctr  [phase] = 0;
      phase ++;
    }
  
  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      unsigned rdest   = comm->absrankof (comm->rank() + maxBF);
      _dest    [phase] = (comm->rank() < nonBF)  ? rdest : -1;
      _sbuf    [phase] = NULL;     /* unknown */
      _rbuf    [phase] = NULL;     /* unknown */
      _cb_recv1[phase] = NULL;
      _cb_recv2[phase] = NULL;
      _sbufln  [phase] = 0;        /* unknown */
      _bufctr  [phase] = 0;
      phase ++;
    }

  _numphases    = phase;
  _phase        = _numphases;
  _sendcomplete = _numphases;
}

/* ************************************************************************* */
/*                     allreduce executor                                    */
/* ************************************************************************* */

void TSPColl::Allreduce::Short::
cb_allreduce (CollExchange *coll, unsigned phase)
{
  TSPColl::Allreduce::Short * ar = (TSPColl::Allreduce::Short *) coll;
  int c = (ar->_counter+1) & 1;
  ar->_cb_allreduce (ar->_dbuf, ar->_phasebuf[phase][c], ar->_nelems);
}

void TSPColl::Allreduce::Short::
cb_switchbuf (CollExchange * coll, unsigned phase)
{
  TSPColl::Allreduce::Short * ar = (TSPColl::Allreduce::Short *) coll;
  int c = (++(ar->_bufctr[phase])) & 1;
  ar->_rbuf[phase] = ar->_phasebuf[phase][c];
}

/* ************************************************************************* */
/*                     start an allreduce operation                          */
/* ************************************************************************* */

void TSPColl::Allreduce::Short::reset (const void         * sbuf,
				       void               * dbuf,
				       __pgasrt_ops_t       op,
				       __pgasrt_dtypes_t    dt,
				       unsigned             nelems)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);

  /* --------------------------------------------------- */
  /*         copy source to destination if necessary     */
  /* --------------------------------------------------- */

  _dbuf   = dbuf;
  _nelems = nelems;
  size_t datawidth = datawidthof (dt);
  assert (nelems * datawidth < sizeof(PhaseBufType));
  if (sbuf != dbuf) memcpy (dbuf, sbuf, nelems * datawidth);

  int maxBF = 1<<_logMaxBF; /* largest power of 2 that fits into comm */
  int nonBF = _comm->size() - maxBF; /* comm->size() - largest power of 2 */

  /* -------------------------------------------- */
  /* phase 0: gather buffers from ranks > n2prev  */
  /* -------------------------------------------- */

  int phase=0;
  if (nonBF > 0)   /* phase 0: gather buffers from ranks > n2prev */
    {
      _sbuf    [phase] = (_comm->rank() >= maxBF) ? dbuf  : NULL;
      _sbufln  [phase] = nelems * datawidth;
      phase ++;
    }
  
  /* -------------------------------------------- */
  /* butterfly phases                             */
  /* -------------------------------------------- */

  for (int i=0; i<_logMaxBF; i++)   /* middle phases: butterfly pattern */
    {
      _sbuf    [phase] = (_comm->rank() < maxBF) ? dbuf  : NULL;
      _sbufln  [phase] = nelems * datawidth;
      phase ++;
    }
  

  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)   /*  last phase: collect results */
    {
      _sbuf    [phase] = (_comm->rank() < nonBF)  ? dbuf  : NULL;
      _rbuf    [phase] = (_comm->rank() >= maxBF) ? dbuf  : NULL;
      _sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  assert (phase == _numphases);

#ifdef DEBUG_ALLREDUCE
  printf ("%d: ", comm->rank());
  for (int i=0; i<_numphases; i++)
    printf ("[%d %s] ", _dest[i], _rbuf[i]? "Y" : "N");
  printf ("\n");
#endif

  _cb_allreduce = getcallback (op, dt);
  TSPColl::CollExchange::reset();
}
