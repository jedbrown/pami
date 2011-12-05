/**
 * \file algorithms/protocols/tspcoll/Allgather.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Allgather.h"
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
/*                       start Allgather                              */
/* ************************************************************************* */
template <class T_NI>
xlpgas::Allgather<T_NI>::
Allgather (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset,T_NI* ni) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset, ni)
{
  pami_type_t allgathertype = PAMI_TYPE_BYTE;
  this->_tmpbuf = NULL;
  this->_tmpbuflen = 0;
  this->_dbuf = NULL;
  this->_nbytes = 0;
  this->_numphases = -1; for (int n=2*this->_comm->size()-1; n>0; n>>=1) this->_numphases++;
  for (int i=0; i< this->_numphases; i++)
    {
      //this first part of the collective makes a barrier;
      //int destindex = (this->ordinal()+2*this->_comm->size()-(1<<i))%this->_comm->size();
      int destindex = (this->ordinal()+(1<<i))%comm->size();
      this->_dest[i] = this->_comm->index2Endpoint(destindex);
      this->_sbuf[i] = &this->_dummy;
      this->_rbuf[i] = &this->_dummy;
      this->_sbufln[i] = 1;
      this->_pwq[i].configure((char *)this->_sbuf[i], this->_sbufln[i], this->_sbufln[i], (TypeCode *)allgathertype, (TypeCode *)allgathertype);
      this->_pwq[i].reset();
    }
  this->_numphases   *= 3;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

/* ************************************************************************* */
/*                      start allgather operation                         */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::Allgather<T_NI>::reset (const void         * sbuf,
			       void               * dbuf,
			       TypeCode           * stype,
			       size_t               stypecount,
			       TypeCode           * rtype,
			       size_t               rtypecount)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);
  size_t nsbytes = stype->GetDataSize() * stypecount;
  size_t nrbytes = rtype->GetDataSize() * rtypecount;
  /* --------------------------------------------------- */
  /*    copy source buffer to dest buffer                */
  /* --------------------------------------------------- */

  memcpy ((char *)dbuf + nrbytes * this->ordinal(), sbuf, nsbytes);

  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0, phase=this->_numphases/3; i<this->_numphases/3; i++, phase+=2)
    {
      int previndex  = (this->ordinal()+2*this->_comm->size()-(1<<i))%this->_comm->size();
      int nextindex  = (this->ordinal()+(1<<i))%this->_comm->size();

      this->_dest[phase]   = this->_comm->index2Endpoint (previndex);
      this->_dest[phase+1] = this->_dest[phase];

      this->_sbuf[phase]   = (char *)dbuf + this->ordinal() * nsbytes;
      this->_sbuf[phase+1] = (char *)dbuf;

      this->_rbuf[phase]   = (char *)dbuf + nextindex*nrbytes;
      this->_rbuf[phase+1] = (char *)dbuf;

      if (this->ordinal() + (1<<i) >= this->_comm->size())
        {
          this->_sbufln[phase]   = nsbytes * (this->_comm->size() - this->ordinal());
          this->_sbufln[phase+1] = nsbytes * (this->ordinal() + (1<<i) - this->_comm->size());
	  //if(this->_sbufln[phase+1] == 0) this->_sbuf[phase+1] = NULL;//mark that there is no data to send
        }
      else
        {
          this->_sbufln[phase] = nsbytes * (1<<i);
          this->_sbufln[phase+1] = 0;
	  //this->_sbuf[phase+1] = NULL;
        }
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], stype, rtype);
      this->_pwq[phase+1].configure((char *)this->_sbuf[phase+1], this->_sbufln[phase+1], this->_sbufln[phase+1], stype, rtype);
      this->_pwq[phase].reset();
      this->_pwq[phase+1].reset();
    }

  xlpgas::CollExchange<T_NI>::reset();
}
