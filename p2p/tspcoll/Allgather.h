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

#ifndef __pgasrt_tspcoll_allgather_h__
#define __pgasrt_tspcoll_allgather_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./CollExchange.h"
#include "./Communicator.h"

//#define DEBUG_ALLGATHER 1
#undef TRACE
#ifdef DEBUG_ALLGATHER
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* *********************************************************************** */
/*                     Allgather (Bruck's algorithm)                       */
/* *********************************************************************** */

namespace TSPColl
{
  class Allgather: public CollExchange
  {
  public:
    static 
    void * operator new (size_t, void * addr)    { return addr; }
    Allgather (Communicator *, NBTag tag, int instID, int offset);
    void reset (const void *, void *, size_t nbytes);
  private:
    char _dummy;
  };
}

/* *********************************************************************** */
/*                   Allgather constructor                                 */
/* *********************************************************************** */

inline TSPColl::Allgather::Allgather (Communicator *comm, NBTag tag, 
				      int instID, int offset):
	       CollExchange (comm, tag, instID, offset, false)
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
  _numphases   *= 3;
  _phase        = _numphases;
  _sendcomplete = _numphases;
}

/* **************************************************************** */
/*              start a new allgather operation                     */
/* **************************************************************** */

inline
void TSPColl::Allgather::reset (const void *sbuf, void *rbuf, size_t nbytes)
{
  /* --------------------------------------------------- */
  /*    copy source buffer to dest buffer                */
  /* --------------------------------------------------- */

  memcpy ((char *)rbuf + nbytes * _comm->rank(), sbuf, nbytes);

  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0, phase=_numphases/3; i<_numphases/3; i++, phase+=2)
    {
      int previndex  = (_comm->rank()+2*_comm->size()-(1<<i))%_comm->size();
      int nextindex  = (_comm->rank()+(1<<i))%_comm->size();

      _dest[phase]   = _comm->absrankof (previndex);
      _dest[phase+1] = _dest[phase];

      _sbuf[phase]   = (char *)rbuf + _comm->rank() * nbytes;
      _sbuf[phase+1] = (char *)rbuf;

      _rbuf[phase]   = (char *)rbuf + nextindex*nbytes;
      _rbuf[phase+1] = (char *)rbuf;

      if (_comm->rank() + (1<<i) >= _comm->size())
 	{
	  _sbufln[phase]   = nbytes * (_comm->size() - _comm->rank());
	  _sbufln[phase+1] = nbytes * (_comm->rank() + (1<<i) - _comm->size());
	}
      else
	{
	  _sbufln[phase] = nbytes * (1<<i);
	  _sbufln[phase+1] = 0;
	}
    }

  CollExchange::reset();
}
#endif
