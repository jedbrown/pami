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

#ifndef __tspcoll_binom_bcast_h__
#define __tspcoll_binom_bcast_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./CollExchange.h"
#include "./Communicator.h"

/* *********************************************************************** */
/*                      binomial broadcast class                           */
/* *********************************************************************** */

namespace TSPColl
{
  class BinomBcast: public CollExchange
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    BinomBcast (Communicator * comm, NBTag tag, int instID, int tagoff);
    void reset (int root, const void * sbuf, void *buf, size_t);
  private:
    char _dummy;
  };
};

/* *********************************************************************** */
/* *********************************************************************** */

inline TSPColl::BinomBcast::
BinomBcast(Communicator * comm, NBTag tag, int instID, int tagoff) :
	       CollExchange(comm, tag, instID, tagoff, false)
{
  _numphases = -1; for (int n=2*_comm->size()-1; n>0; n>>=1) _numphases++;
  for (int i=0; i< _numphases; i++)
    {
      int destindex = (_comm->rank()+2*_comm->size()-(1<<i))%_comm->size();
      _dest[i] = _comm->absrankof(destindex);
      _sbuf[i] = &_dummy;
      _rbuf[i] = &_dummy;
      _sbufln[i] = 1;
    }
  _numphases   *= 2;
  _phase        = _numphases;
  _sendcomplete = _numphases;
}

/* ********************************************************************** */
/*                    start a new binomial broadcast                      */
/* ********************************************************************** */

inline void TSPColl::BinomBcast::
reset (int rootindex, const void * sbuf, void * buf, size_t nbytes)
{
  if (rootindex >= _comm->size())
    __pgasrt_fatalerror (-1, "Invalid root index in BinomBcast");

  /* --------------------------------------------------- */
  /* --------------------------------------------------- */

  if (rootindex == _comm->rank() && sbuf != buf)
    memcpy (buf, sbuf, nbytes);

  /* --------------------------------------------------- */
  /* --------------------------------------------------- */

  int myrelrank = (_comm->rank() + _comm->size() - rootindex) % _comm->size();
  for (int i=0, phase=_numphases/2; i<_numphases/2; i++, phase++)
    {
      int  dist       = 1<<(_numphases/2-1-i);
      int  sendmask   = (1<<(_numphases/2-i))-1;
      int  destrelrank= myrelrank + dist;
      int  srcrelrank = myrelrank - dist;
      bool dosend     = ((myrelrank&sendmask)==0)&&(destrelrank<_comm->size());
      bool dorecv     = ((srcrelrank&sendmask)==0)&&(srcrelrank>=0);
      int  destindex  = (destrelrank + rootindex)%_comm->size();
      _dest[phase]    = _comm->absrankof(destindex);
      _sbuf[phase]    = dosend ? buf : NULL;
      _sbufln[phase]  = dosend ? nbytes : 0;
      _rbuf[phase]    = dorecv ? buf : NULL;
    }

  CollExchange::reset();
}

#endif
