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
Allgather (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset)
{
  this->_tmpbuf = NULL;
  this->_tmpbuflen = 0;
  this->_dbuf = NULL;
  this->_nbytes = 0;
  this->_numphases = -1; for (int n=2*this->_comm->size()-1; n>0; n>>=1) this->_numphases++;
  for (int i=0; i< this->_numphases; i++)
    {
      //this first part of the collective makes a barrier;
      //int destindex = (this->_comm->ordinal()+2*this->_comm->size()-(1<<i))%this->_comm->size();
      int destindex = (comm->ordinal()+(1<<i))%comm->size();
      this->_dest[i] = this->_comm->endpoint(destindex);
      this->_sbuf[i] = &this->_dummy;
      this->_rbuf[i] = &this->_dummy;
      this->_sbufln[i] = 1;
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
			       unsigned             nbytes)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);

  /* --------------------------------------------------- */
  /*    copy source buffer to dest buffer                */
  /* --------------------------------------------------- */

  memcpy ((char *)dbuf + nbytes * this->_comm->ordinal(), sbuf, nbytes);

  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0, phase=this->_numphases/3; i<this->_numphases/3; i++, phase+=2)
    {
      int previndex  = (this->_comm->ordinal()+2*this->_comm->size()-(1<<i))%this->_comm->size();
      int nextindex  = (this->_comm->ordinal()+(1<<i))%this->_comm->size();

      this->_dest[phase]   = this->_comm->endpoint (previndex);
      this->_dest[phase+1] = this->_dest[phase];

      this->_sbuf[phase]   = (char *)dbuf + this->_comm->ordinal() * nbytes;
      this->_sbuf[phase+1] = (char *)dbuf;

      this->_rbuf[phase]   = (char *)dbuf + nextindex*nbytes;
      this->_rbuf[phase+1] = (char *)dbuf;

      if (this->_comm->ordinal() + (1<<i) >= this->_comm->size())
        {
          this->_sbufln[phase]   = nbytes * (this->_comm->size() - this->_comm->ordinal());
          this->_sbufln[phase+1] = nbytes * (this->_comm->ordinal() + (1<<i) - this->_comm->size());
	  //if(this->_sbufln[phase+1] == 0) this->_sbuf[phase+1] = NULL;//mark that there is no data to send
        }
      else
        {
          this->_sbufln[phase] = nbytes * (1<<i);
          this->_sbufln[phase+1] = 0;
	  //this->_sbuf[phase+1] = NULL;
        }
    }

  xlpgas::CollExchange<T_NI>::reset();
}
