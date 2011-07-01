/**
 * \file algorithms/protocols/tspcoll/LongAllreduce.cc
 * \brief ???
 */
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
/*                       Long allreduce constructor                          */
/* ************************************************************************* */
template <class T_NI>
xlpgas::Allreduce::Long<T_NI>::
Long (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset)
{
  this->_tmpbuf = NULL;
  this->_tmpbuflen = 0;
  this->_dbuf = NULL;
  this->_nelems = 0;
  for (this->_logMaxBF = 0; (1<<(this->_logMaxBF+1)) <= (int)this->_comm->size(); this->_logMaxBF++) ;
  int maxBF  = 1<<this->_logMaxBF;
  int nonBF  = this->_comm->size() - maxBF;
  int phase  = 0;

  /* -------------------------------------------- */
  /* phase 0: gather buffers from ranks > n2prev  */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      /* send permission chits to potential senders */

      this->_dest    [phase] = this->_comm->endpoint (this->_comm->ordinal() + maxBF);
      this->_sbuf    [phase] = ((int)this->_comm->ordinal() <  nonBF) ? &this->_dummy : NULL;
      this->_rbuf    [phase] = ((int)this->_comm->ordinal() >= maxBF) ? &this->_dummy : NULL;
      this->_postrcv [phase] = NULL;
      this->_sbufln  [phase] = 1;

      //printf("%d: in phase %d will send to  %d \n", XLPGAS_MYNODE, phase, _dest[phase].node);
      phase ++;

      /* send data */

      this->_dest    [phase] = this->_comm->endpoint (this->_comm->ordinal() - maxBF);
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_postrcv [phase] = ((int)this->_comm->ordinal() < nonBF)  ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;    /* data length not available */

      //sprintf("%d: in phase %d will send to  %d \n", XLPGAS_MYNODE, phase, _dest[phase].node);
      phase ++;
    }

  /* -------------------------------------------- */
  /* butterfly phase                              */
  /* -------------------------------------------- */

  for (int i=0; i<this->_logMaxBF; i++)
    {
      /* send permission chits to senders */

      this->_dest    [phase] = this->_comm->endpoint (this->_comm->ordinal() ^ (1<<i));
      this->_sbuf    [phase] = ((int)this->_comm->ordinal() < maxBF) ? &this->_dummy : NULL;
      this->_rbuf    [phase] = ((int)this->_comm->ordinal() < maxBF) ? &this->_dummy : NULL;
      this->_postrcv [phase] = NULL;
      this->_sbufln  [phase] = 1;

      //printf("%d: in phase %d will send to  %d \n", XLPGAS_MYNODE, phase, _dest[phase].node);
      phase ++;

      /* send data */

      this->_dest    [phase] = this->_comm->endpoint (this->_comm->ordinal() ^ (1<<i));
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_postrcv [phase] = ((int)this->_comm->ordinal() < maxBF) ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;    /* data length not available */

      //printf("%d: in phase %d will send to  %d \n", XLPGAS_MYNODE, phase, _dest[phase].node);
      phase ++;
    }

  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      /* send permission slips */

      this->_dest    [phase] = this->_comm->endpoint (this->_comm->ordinal() - maxBF);
      this->_sbuf    [phase] = ((int)this->_comm->ordinal() >= maxBF) ? &this->_dummy : NULL;
      this->_rbuf    [phase] = ((int)this->_comm->ordinal() < nonBF)  ? &this->_dummy : NULL;
      this->_postrcv [phase] = NULL;
      this->_sbufln  [phase] = 1;
      //printf("%d: in phase %d will send to  %d \n", XLPGAS_MYNODE, phase, _dest[phase].node);
      phase ++;

      /* send data */

      this->_dest    [phase] = this->_comm->endpoint ((int)this->_comm->ordinal() + maxBF);
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_postrcv [phase] = NULL;
      this->_sbufln  [phase] = 0; /* data length not available */
      //printf("%d: in phase %d will send to  %d \n", XLPGAS_MYNODE, phase, _dest[phase].node);
      phase ++;
    }

  this->_numphases    = phase;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

/* ************************************************************************* */
/*                     allreduce executor                                    */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::Allreduce::Long<T_NI>::
cb_allreduce (CollExchange<T_NI> *coll, unsigned phase)
{
  xlpgas::Allreduce::Long<T_NI> * ar = (xlpgas::Allreduce::Long<T_NI> *) coll;

  //  int * dbuf   = (int *) ar->_dbuf;
  //  int * pbuf   = (int *) ar->_tmpbuf;
  //printf("%d: Add %d to already existent%d in phase %d\n", XLPGAS_MYNODE, pbuf[0], dbuf[0], phase);
  void * inputs[] = {ar->_dbuf, ar->_tmpbuf};
  ar->_cb_allreduce (ar->_dbuf, inputs, 2, ar->_nelems);
}

/* ************************************************************************* */
/*                      start a long allreduce operation                     */
/* ************************************************************************* */
#define MAXOF(a,b) (((a)>(b))?(a):(b))
template <class T_NI>
void xlpgas::Allreduce::Long<T_NI>::reset (const void         * sbuf,
				     void               * dbuf,
				     xlpgas_ops_t       op,
				     xlpgas_dtypes_t    dt,
				     unsigned           nelems,
				     user_func_t* uf)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);

  xlpgas::CollExchange<T_NI>::reset();//to lock

  _uf = uf; //user function pointer
  /* --------------------------------------------------- */
  /*         copy source to destination if necessary     */
  /* --------------------------------------------------- */
  /*
    printf("L%d: long allreduce [teamid=%d rank=%d sizeworld=%d nelems=%d]\n",
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

  /* need more memory in temp buffer? */
  if (_tmpbuflen < nelems * datawidth)
    {
      if (_tmpbuf) {
	__global.heap_mm->free (_tmpbuf);
	_tmpbuf = NULL;
	//printf("L%d: AR __global.heap_mm->free null \n",XLPGAS_MYNODE);
      }
      assert (nelems * datawidth > 0);

      /*
#if TRANSPORT == bgp
      //int alignment = MAXOF(sizeof(void*), datawidth);
      int alignment = sizeof(void*);
      int rc = __global.heap_mm->memalign (&_tmpbuf, alignment, nelems*datawidth);
      //printf("L%d: AR bgp alment=%d sz=%d %d \n",XLPGAS_MYNODE, alignment,nelems*datawidth,rc);
#else
      */
      _tmpbuf = __global.heap_mm->malloc (nelems * datawidth);
      int rc = 0;

      //#endif


      if (rc || !_tmpbuf)
	xlpgas_fatalerror (-1, "Allreduce: memory allocation error, rc=%d", rc);
      _tmpbuflen = nelems * datawidth;
    }

  /* --------------------------------------------------- */
  /*  set source and destination buffers and node ids    */
  /* We deal with non-power-of-2 nodes                   */
  /* --------------------------------------------------- */

  int maxBF  = 1<<_logMaxBF;
  int nonBF  = this->_comm->size() - maxBF;
  int phase  = 0;

  if (nonBF > 0)   /* phase 0: gather buffers from ranks > n2prev */
    {
      phase ++;
      this->_sbuf    [phase] = ((int)this->_comm->ordinal() >= maxBF) ? this->_dbuf : NULL;
      this->_rbuf    [phase] = ((int)this->_comm->ordinal() < nonBF)  ? this->_tmpbuf : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  for (int i=0; i<this->_logMaxBF; i++)   /* middle phases: butterfly pattern */
    {
      phase ++;
      this->_sbuf    [phase] = ((int)this->_comm->ordinal() < maxBF) ? this->_dbuf : NULL;
      this->_rbuf    [phase] = ((int)this->_comm->ordinal() < maxBF) ? this->_tmpbuf : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  if (nonBF > 0)   /*  last phase: collect results */
    {
      phase ++;
      this->_sbuf    [phase] = ((int)this->_comm->ordinal() < nonBF)  ? this->_dbuf  : NULL;
      this->_rbuf    [phase] = ((int)this->_comm->ordinal() >= maxBF) ? this->_dbuf  : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  assert (phase == this->_numphases);
  this->_cb_allreduce = getcallback (op, dt);
}
