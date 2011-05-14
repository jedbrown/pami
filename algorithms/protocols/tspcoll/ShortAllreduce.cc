#include "algorithms/protocols/tspcoll/Allreduce.h"
#include "algorithms/protocols/tspcoll/Team.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#ifndef __APPLE__
#include <malloc.h>
#endif

//#define DEBUG_ALLREDUCE 1
#undef TRACE
#ifdef DEBUG_ALLREDUCE
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* ************************************************************************* */
/*                       start a short allreduce                              */
/* ************************************************************************* */

xlpgas::Allreduce::Short::
Short (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
  CollExchange (ctxt, comm, kind, tag, offset)
{
  _dbuf = NULL;
  _nelems = 0;
  for (_logMaxBF = 0; (1<<(_logMaxBF+1)) <= _comm->size(); _logMaxBF++) ;
  int maxBF  = 1<<_logMaxBF;
  int nonBF  = _comm->size() - maxBF;
  int phase  = 0;

  /* -------------------------------------------- */
  /* phase 0: gather buffers from ordinals > n2prev  */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      //xlpgas_endpoint_t rdest = comm->endpoint (comm->ordinal() - maxBF);
      //_dest    [phase] = (comm->ordinal() >= maxBF) ? rdest : -1;
      _dest    [phase] = comm->endpoint (comm->ordinal() - maxBF);
      _sbuf    [phase] = NULL;    /* unknown */
      _rbuf    [phase] = (comm->ordinal() < nonBF)  ? _phasebuf[phase][0] : NULL;
      _cb_rcvhdr[phase] = (comm->ordinal() < nonBF)  ? cb_switchbuf : NULL;
      _postrcv [phase] = (comm->ordinal() < nonBF)  ? cb_allreduce : NULL;
      _sbufln  [phase] = 0;       /* unknown */
      _bufctr  [phase] = 0;
      phase ++;
    }

  /* -------------------------------------------- */
  /* butterfly phases                             */
  /* -------------------------------------------- */

  for (int i=0; i<_logMaxBF; i++)
    {
      //unsigned rdest   = comm->endpoint (comm->ordinal() ^ (1<<i));
      //_dest    [phase] = (comm->ordinal() < maxBF) ? rdest : -1;
      _dest [phase] = comm->endpoint (comm->ordinal() ^ (1<<i));
      _sbuf    [phase] = NULL;     /* unknown */
      _rbuf    [phase] = (comm->ordinal() < maxBF) ? _phasebuf[phase][0] : NULL;
      _cb_rcvhdr[phase] = (comm->ordinal() < maxBF) ? cb_switchbuf : NULL;
      _postrcv [phase] = (comm->ordinal() < maxBF) ? cb_allreduce : NULL;
      _sbufln  [phase] = 0;        /* unknown */
      _bufctr  [phase] = 0;
      phase ++;
    }

  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      ///unsigned rdest   = comm->endpoint (comm->ordinal() + maxBF);
      //_dest    [phase] = (comm->ordinal() < nonBF)  ? rdest : -1;
      _dest    [phase] = comm->endpoint (comm->ordinal() + maxBF);
      _sbuf    [phase] = NULL;     /* unknown */
      _rbuf    [phase] = NULL;     /* unknown */
      _cb_rcvhdr[phase] = NULL;
      _postrcv [phase] = NULL;
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

void xlpgas::Allreduce::Short::
cb_allreduce (CollExchange *coll, unsigned phase)
{
  xlpgas::Allreduce::Short * ar = (xlpgas::Allreduce::Short *) coll;
  int c = (ar->_counter+1) & 1;
  /*int *s = (int*)(ar->_phasebuf[phase][c]);
  int *d = (int*)(ar->_dbuf);
  for(int i=0;i<ar->_nelems;++i){
    printf("L%d Add %d %d\n",XLPGAS_MYNODE, s[i],d[i]);
  }
  */
  ar->_cb_allreduce (ar->_dbuf, ar->_phasebuf[phase][c], ar->_nelems, *(ar->_uf));
}

xlpgas_local_addr_t xlpgas::Allreduce::Short::
cb_switchbuf (CollExchange * coll, unsigned phase, unsigned counter)
{
  xlpgas::Allreduce::Short * ar = (xlpgas::Allreduce::Short *) coll;
  int c = (counter+1) & 1;
  return (xlpgas_local_addr_t) (ar->_phasebuf[phase][c]);
}

/* ************************************************************************* */
/*                      start a short allreduce operation                     */
/* ************************************************************************* */
void xlpgas::Allreduce::Short::reset (const void         * sbuf,
				     void               * dbuf,
				     xlpgas_ops_t       op,
				     xlpgas_dtypes_t    dt,
				     unsigned           nelems,
				     user_func_t* uf)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);

  xlpgas::CollExchange::reset();//to lock

  _uf = uf; //user function pointer
  /* --------------------------------------------------- */
  /*         copy source to destination if necessary     */
  /* --------------------------------------------------- */
  /*
    printf("L%d: short allreduce [teamid=%d rank=%d sizeworld=%d nelems=%d]\n",
	 XLPGAS_MYNODE,
	 _comm->commID(),
	 _comm->ordinal(),
	 _comm->size(),
	 nelems
	 );
  */
  _dbuf   = dbuf;
  _nelems = nelems;
  size_t datawidth = datawidthof (dt);
  if (sbuf != dbuf) memcpy (dbuf, sbuf, nelems * datawidth);

  /* --------------------------------------------------- */
  /*  set source and destination buffers and node ids    */
  /* We deal with non-power-of-2 nodes                   */
  /* --------------------------------------------------- */

  int maxBF  = 1<<_logMaxBF;
  int nonBF  = _comm->size() - maxBF;
  int phase  = 0;

  if (nonBF > 0)   /* phase 0: gather buffers from ordinals > n2prev */
    {
      _sbuf    [phase] = (_comm->ordinal() >= maxBF) ? dbuf  : NULL;
      _sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  /* -------------------------------------------- */
  /* butterfly phases                             */
  /* -------------------------------------------- */

  for (int i=0; i<_logMaxBF; i++)   /* middle phases: butterfly pattern */
    {
      _sbuf    [phase] = (_comm->ordinal() < maxBF) ? dbuf  : NULL;
      _sbufln  [phase] = nelems * datawidth;
      phase ++;
    }


  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)   /*  last phase: collect results */
    {
      _sbuf    [phase] = (_comm->ordinal() < nonBF)  ? dbuf  : NULL;
      _rbuf    [phase] = (_comm->ordinal() >= maxBF) ? dbuf  : NULL;
      _sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  assert (phase == _numphases);
  _cb_allreduce = getcallback (op, dt);
}
