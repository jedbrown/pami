/**
 * \file algorithms/protocols/tspcoll/PrefixSums.cc
 * \brief ???
 */
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
PrefixSums (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset, T_NI* ni) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset, ni)
{
  pami_type_t scantype = PAMI_TYPE_BYTE;
  this->_tmpbuf = NULL;
  this->_tmpbuflen = 0;
  this->_dbuf = NULL;
  this->_nelems = 0;
  this->_exclusive = 0;
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
      int left = this->ordinal() - (1<<i);
      this->_dest    [phase] = this->_comm->endpoint (left);
      this->_sbuf    [phase] = (left >= 0) ? &this->_dummy : NULL;
      int right = this->ordinal() + (1<<i);
      this->_rbuf    [phase] = (right < (int)this->_comm->size()) ? &this->_dummy : NULL;
      this->_postrcv [phase] = NULL;
      this->_sbufln  [phase] = 1;
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], (TypeCode *)scantype, (TypeCode *)scantype);
      this->_pwq[phase].reset();
      //printf("%d: ### in phase %d will send permission to  %d \n", XLPGAS_MYNODE, phase, _dest[phase].node);
      phase ++;

      /* send data */
      this->_dest    [phase] = ( right < (int)this->_comm->size() ) ? this->_comm->endpoint(right) : this->_comm->endpoint(0);
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_postrcv [phase] = (left >= 0) ? cb_prefixsums : NULL;
      this->_sbufln  [phase] = 0;    /* data length not available */
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], (TypeCode *)scantype, (TypeCode *)scantype);
      this->_pwq[phase].reset();
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
  void * inputs[2];
  if(ar->_exclusive == 0)
    {
      inputs[0] = ar->_dbuf;
      inputs[1] = ar->_tmpbuf;
      ar->_cb_prefixsums (ar->_dbuf, inputs, 2, ar->_nelems);
    }
  else if(ar->ordinal() > 0)
    {
      TypeCode *dt = ar->_dt;
      size_t datawidth = dt->GetDataSize();
      inputs[1] = (((char *)ar->_tmpbuf) + ar->_nelems * datawidth);
      // In phase 1 we copy the received data to the destination buffer
      if(phase == 1)
        {
          memcpy(ar->_dbuf, inputs[1], ar->_nelems * datawidth);
        }
      else
        {
          inputs[0] = ar->_dbuf;
          ar->_cb_prefixsums (ar->_dbuf, inputs, 2, ar->_nelems);
        }
        inputs[0] = ar->_tmpbuf;
        ar->_cb_prefixsums (ar->_tmpbuf, inputs, 2, ar->_nelems);
    }
}


/* ************************************************************************* */
/*                      start prefixSums operation                           */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::PrefixSums<T_NI>::reset (const void         * sbuf,
				void               * dbuf,
				pami_data_function   op,
				TypeCode           * dt,
				size_t               nelems)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);
  xlpgas::CollExchange<T_NI>::reset();
  this->_dt = dt;
  this->_dbuf   = dbuf;
  this->_nelems = nelems;

  size_t datawidth = dt->GetDataSize();
  size_t tmpbufsize;

  /* For exclusive scan we need twice the amount of temporary buffer space
     _tmpbuf = {x,y}  , where
      x = partial "inclusive" scan result
      y = buffer received from left neighbour
      size of x = size of y = nelems * datawidth
  */
  if(_exclusive == 0)
    tmpbufsize = nelems * datawidth;
  else
    tmpbufsize = 2 * nelems * datawidth;


  /* need more memory in temp buffer? */
  if (this->_tmpbuflen < tmpbufsize)
  {
    if (this->_tmpbuf) {
      __global.heap_mm->free (this->_tmpbuf);
      this->_tmpbuf = NULL;
    }
    assert (nelems * datawidth > 0);

#if TRANSPORT == bgp
      //int alignment = MAXOF(sizeof(void*), datawidth);
      int alignment = sizeof(void*);
      int rc = __global.heap_mm->memalign (&this->_tmpbuf, alignment, tmpbufsize);
      //printf("L%d: AR bgp alment=%d sz=%d %d \n",XLPGAS_MYNODE, alignment,nelems*datawidth,rc)
#else
    this->_tmpbuf = __global.heap_mm->malloc (tmpbufsize);
    int rc = 0;
#endif
    if (rc || !this->_tmpbuf)
	  xlpgas_fatalerror (-1, "PrefixSums: memory allocation error, rc=%d", rc);
  }

 
  this->_tmpbuflen = tmpbufsize;
  if (sbuf != dbuf)
    {
      if(_exclusive == 0)
        {
          memcpy (dbuf, sbuf, nelems * datawidth);
        }
      else
        {
          memcpy (this->_tmpbuf, sbuf, nelems * datawidth);
        }
    }

  /* --------------------------------------------------- */
  /*  set source and destination buffers and node ids    */
  /* We deal with non-power-of-2 nodes                   */
  /* --------------------------------------------------- */
  int phase  = 0;
  void *sbuf_phase, *rbuf_phase;
  if(this->_exclusive == 0)
    {
      sbuf_phase = this->_dbuf;
      rbuf_phase = this->_tmpbuf;
    }
  else
    {
      sbuf_phase = this->_tmpbuf;
      rbuf_phase = (((char *)this->_tmpbuf) + nelems * datawidth);
    }

  if(tmpbufsize == 0 && !rbuf_phase)
  {
    rbuf_phase = this->_dbuf;
  }

  for (int i=0; i<this->_logMaxBF; i++)   /* prefix sums pattern */
    {
      phase ++;
      int tgt = this->ordinal() + (1<<i);
      this->_sbuf    [phase] = (tgt < (int)this->_comm->size()) ? sbuf_phase : NULL;
      this->_sbufln  [phase] = (tgt<(int)this->_comm->size()) ? nelems * datawidth : 0 ;
      tgt = this->ordinal() - (1<<i);
      this->_rbuf    [phase] = (tgt>=0) ? rbuf_phase : NULL;
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], dt, dt);
      this->_pwq[phase].reset();
      phase ++;
    }
  assert (phase == this->_numphases);
  this->_cb_prefixsums = xlpgas::Allreduce::getcallback (op, dt);
}
