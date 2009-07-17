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
/*                       start a long allreduce                              */
/* ************************************************************************* */
TSPColl::Allreduce::Long::
Long (Communicator * comm, NBTag tag, int instID, int offset) :
  CollExchange (comm, tag, instID, offset, false)
{
  _tmpbuf = NULL;
  _dbuf = NULL;
  _nelems = 0;
  for (_logMaxBF = 0; (1<<(_logMaxBF+1)) <= this->_comm->size(); _logMaxBF++) ;
  int maxBF  = 1<<_logMaxBF;
  int nonBF  = this->_comm->size() - maxBF;
  int phase  = 0;

  /* -------------------------------------------- */
  /* phase 0: gather buffers from ranks > n2prev  */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      /* send permission chits to potential senders */

      unsigned rdest = this->_comm->absrankof (this->_comm->rank() + maxBF);
      this->_dest    [phase] = (this->_comm->rank() <  nonBF) ? rdest : -1;
      this->_sbuf    [phase] = (this->_comm->rank() <  nonBF) ? &_dummy : NULL;
      this->_rbuf    [phase] = (this->_comm->rank() >= maxBF) ? &_dummy : NULL;
      this->_cb_recv2[phase] = NULL;
      this->_sbufln  [phase] = 1;
      phase ++;

      /* send data */

      rdest = this->_comm->absrankof (this->_comm->rank() - maxBF);
      this->_dest    [phase] = (this->_comm->rank() >= maxBF) ? rdest : -1;
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_cb_recv2[phase] = (this->_comm->rank() < nonBF)  ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;    /* data length not available */
      phase ++;
    }

  /* -------------------------------------------- */
  /* butterfly phase                              */
  /* -------------------------------------------- */

  for (int i=0; i<_logMaxBF; i++)
    {
      /* send permission chits to senders */

      unsigned rdest   = this->_comm->absrankof (this->_comm->rank() ^ (1<<i));
      this->_dest    [phase] = (this->_comm->rank() < maxBF) ? rdest : -1;
      this->_sbuf    [phase] = (this->_comm->rank() < maxBF) ? &_dummy : NULL;
      this->_rbuf    [phase] = (this->_comm->rank() < maxBF) ? &_dummy : NULL;
      this->_cb_recv2[phase] = NULL;
      this->_sbufln  [phase] = 1;
      phase ++;

      /* send data */

      this->_dest    [phase] = (this->_comm->rank() < maxBF) ? rdest : -1;
      this->_sbuf    [phase] = NULL; /* send buffer not available */
      this->_rbuf    [phase] = NULL; /* receive buffer not available */
      this->_cb_recv2[phase] = (this->_comm->rank() < maxBF) ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;    /* data length not available */
      phase ++;
    }

  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */
  
  if (nonBF > 0)
    {
      /* send permission slips */

      unsigned rdest = this->_comm->absrankof (this->_comm->rank() - maxBF);
      this->_dest    [phase] = (this->_comm->rank() >= maxBF) ? rdest : -1;
      this->_sbuf    [phase] = (this->_comm->rank() >= maxBF) ? &_dummy : NULL;
      this->_rbuf    [phase] = (this->_comm->rank() < nonBF)  ? &_dummy : NULL;
      this->_cb_recv2[phase] = NULL;
      this->_sbufln  [phase] = 1;
      phase ++;

      /* send data */

      rdest   = this->_comm->absrankof (this->_comm->rank() + maxBF);
      this->_dest    [phase] = (this->_comm->rank() < nonBF)  ? rdest : -1;
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
void TSPColl::Allreduce::Long::
cb_allreduce (CollExchange *coll, unsigned phase)
{
  TSPColl::Allreduce::Long * ar = (TSPColl::Allreduce::Long *) coll;
  void * inputs[] = {ar->_dbuf, ar->_tmpbuf};
  //  ar->_cb_allreduce (ar->_dbuf, ar->_tmpbuf, ar->_nelems);
  ar->_cb_allreduce (ar->_dbuf, inputs, 2, ar->_nelems);
}

/* ************************************************************************* */
/*                      start a long allreduce operation                     */
/* ************************************************************************* */
void TSPColl::Allreduce::Long::reset (const void         * sbuf, 
				      void               * dbuf, 
				      CM_Op              op,
				      CM_Dt              dt,
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

  if (nonBF > 0)   /* phase 0: gather buffers from ranks > n2prev */
    {
      phase ++;
      this->_sbuf    [phase] = (this->_comm->rank() >= maxBF) ? _dbuf : NULL;
      this->_rbuf    [phase] = (this->_comm->rank() < nonBF)  ? _tmpbuf : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }
  
  for (int i=0; i<_logMaxBF; i++)   /* middle phases: butterfly pattern */
    {
      phase ++;
      this->_sbuf    [phase] = (this->_comm->rank() < maxBF) ? _dbuf : NULL;
      this->_rbuf    [phase] = (this->_comm->rank() < maxBF) ? _tmpbuf : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }
  
  if (nonBF > 0)   /*  last phase: collect results */
    {
      phase ++;
      this->_sbuf    [phase] = (this->_comm->rank() < nonBF)  ? _dbuf  : NULL;
      this->_rbuf    [phase] = (this->_comm->rank() >= maxBF) ? _dbuf  : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  assert (phase == this->_numphases);
  //  _cb_allreduce = getcallback (op, dt);
  TSPColl::CollExchange::reset();
}
