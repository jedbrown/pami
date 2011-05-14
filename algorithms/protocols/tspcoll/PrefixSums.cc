#include "algorithms/protocols/tspcoll/PrefixSums.h"
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

#undef TRACE
#ifdef DEBUG_COLL
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* ************************************************************************* */
/*                       Start PrefixSums                                    */
/* ************************************************************************* */
template <class T_NI>
xlpgas::PrefixSums<T_NI>::
PrefixSums (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset)
{
  this->_tmpbuf = NULL;
  this->_tmpbuflen = 0;
  this->_dbuf = NULL;
  this->_nelems = 0;
  for (this->_logMaxBF = 0; (1<<(this->_logMaxBF+1)) <= (int)this->_comm->size(); this->_logMaxBF++) ;
  int maxBF  = 1<<this->_logMaxBF;
  if(maxBF < (int) this->_comm->size()) this->_logMaxBF++;
  int phase  = 0;

  /* -------------------------------------------- */
  /* prefix sums                                  */
  /* -------------------------------------------- */

  for (int i=0; i<this->_logMaxBF; i++)
    {
      /* send permission chits to senders */
      int left = this->_comm->ordinal() - (1<<i);
      this->_dest    [phase] = this->_comm->endpoint (left);
      this->_sbuf    [phase] = (left >= 0) ? &this->_dummy : NULL;
      int right = this->_comm->ordinal() + (1<<i);
      this->_rbuf    [phase] = (right < (int)this->_comm->size()) ? &this->_dummy : NULL;
      this->_postrcv [phase] = NULL;
      this->_sbufln  [phase] = 1;

      //printf("%d: ### in phase %d will send permission to  %d \n", XLPGAS_MYNODE, phase, _dest[phase].node);
      phase ++;

      /* send data */
      this->_dest    [phase] = ( right < (int)this->_comm->size() ) ? this->_comm->endpoint(right) : this->_comm->endpoint(0);
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_postrcv [phase] = (left >= 0) ? cb_prefixsums : NULL;
      this->_sbufln  [phase] = 0;    /* data length not available */

      //printf("%d: ### in phase %d will send data to  %d \n", XLPGAS_MYNODE, phase, _dest[phase].node);
      phase ++;
    }

  this->_numphases    = phase;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

template <class T_NI>
void xlpgas::PrefixSums<T_NI>::
cb_prefixsums (CollExchange<T_NI> *coll, unsigned phase)
{
  xlpgas::PrefixSums<T_NI> * ar = (xlpgas::PrefixSums<T_NI> *) coll;
  void * inputs[] = {ar->_dbuf, ar->_tmpbuf};
  ar->_cb_prefixsums (ar->_dbuf, inputs, 2, ar->_nelems);
}


/* ************************************************************************* */
/*                      start prefixSums operation                         */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::PrefixSums<T_NI>::reset (const void         * sbuf,
				void               * dbuf,
				xlpgas_ops_t         op,
				xlpgas_dtypes_t      dt,
				unsigned             nelems)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);
  xlpgas::CollExchange<T_NI>::reset();
  this->_dt = dt;
  this->_dbuf   = dbuf;
  this->_nelems = nelems;
  size_t datawidth = xlpgas::Allreduce::datawidthof (dt);
  if (sbuf != dbuf) memcpy (dbuf, sbuf, nelems * datawidth);

  /* need more memory in temp buffer? */
  if (this->_tmpbuflen < nelems * datawidth)
    {
      if (this->_tmpbuf) {
	__global.heap_mm->free (this->_tmpbuf);
	this->_tmpbuf = NULL;
      }
      assert (nelems * datawidth > 0);

#if TRANSPORT == bgp
      //int alignment = MAXOF(sizeof(void*), datawidth);
      int alignment = sizeof(void*);
      int rc = __global.heap_mm->memalign (&this->_tmpbuf, alignment, nelems*datawidth);
      //printf("L%d: AR bgp alment=%d sz=%d %d \n",XLPGAS_MYNODE, alignment,nelems*datawidth,rc)
#else
      this->_tmpbuf = __global.heap_mm->malloc (nelems * datawidth);
      int rc = 0;
#endif
      if (rc || !this->_tmpbuf)
	xlpgas_fatalerror (-1, "PrefixSums: memory allocation error, rc=%d", rc);
      this->_tmpbuflen = nelems * datawidth;
    }

  /* --------------------------------------------------- */
  /*  set source and destination buffers and node ids    */
  /* We deal with non-power-of-2 nodes                   */
  /* --------------------------------------------------- */
  int phase  = 0;

  for (int i=0; i<this->_logMaxBF; i++)   /* prefix sums pattern */
    {
      phase ++;
      int tgt = this->_comm->ordinal() + (1<<i);
      this->_sbuf    [phase] = (tgt < (int)this->_comm->size()) ? this->_dbuf : NULL;
      this->_sbufln  [phase] = (tgt<(int)this->_comm->size()) ? nelems * datawidth : 0 ;
      tgt = this->_comm->ordinal() - (1<<i);
      this->_rbuf    [phase] = (tgt>=0) ? this->_tmpbuf : NULL;
      phase ++;
    }

  assert (phase == this->_numphases);
  this->_cb_prefixsums = xlpgas::Allreduce::getcallback (op, dt);
}
