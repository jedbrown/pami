/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2009, 2010.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */
/**
 * \file algorithms/protocols/tspcoll/Allgather.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_Allgather_h__
#define __algorithms_protocols_tspcoll_Allgather_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./CollExchange.h"
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
  template <class T_Mcast>
  class Allgather: public CollExchange<T_Mcast>
  {
  public:
    static
    void * operator new (size_t, void * addr)    { return addr; }
    Allgather (XMI_GEOMETRY_CLASS *, NBTag tag, int instID, int offset);
    void reset (const void *, void *, size_t nbytes);
  private:
    char _dummy;
  };
}

/* *********************************************************************** */
/*                   Allgather constructor                                 */
/* *********************************************************************** */
template <class T_Mcast>
inline TSPColl::Allgather<T_Mcast>::Allgather (XMI_GEOMETRY_CLASS *comm, NBTag tag,
				      int instID, int offset):
  CollExchange<T_Mcast> (comm, tag, instID, offset, false)
{
  this->_numphases = -1; for (int n=2*this->_comm->size()-1; n>0; n>>=1) this->_numphases++;
  for (int i=0; i< this->_numphases; i++)
    {
      int rank       = this->_comm->virtrank();
      int destindex  = (rank+2*this->_comm->size()-(1<<i))%this->_comm->size();
      this->_dest[i] = this->_comm->absrankof(destindex);
      this->_sbuf[i] = &_dummy;
      this->_rbuf[i] = &_dummy;
      this->_sbufln[i] = 1;
    }
  this->_numphases   *= 3;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

/* **************************************************************** */
/*              start a new allgather operation                     */
/* **************************************************************** */

template <class T_Mcast>
inline
void TSPColl::Allgather<T_Mcast>::reset (const void *sbuf, void *rbuf, size_t nbytes)
{
  /* --------------------------------------------------- */
  /*    copy source buffer to dest buffer                */
  /* --------------------------------------------------- */
  int rank = this->_comm->virtrank();
  memcpy ((char *)rbuf + nbytes * rank, sbuf, nbytes);

  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0, phase=this->_numphases/3; i<this->_numphases/3; i++, phase+=2)
    {
      int previndex  = (rank+2*this->_comm->size()-(1<<i))%this->_comm->size();
      int nextindex  = (rank+(1<<i))%this->_comm->size();

      this->_dest[phase]   = this->_comm->absrankof (previndex);
      this->_dest[phase+1] = this->_dest[phase];

      this->_sbuf[phase]   = (char *)rbuf + rank * nbytes;
      this->_sbuf[phase+1] = (char *)rbuf;

      this->_rbuf[phase]   = (char *)rbuf + nextindex*nbytes;
      this->_rbuf[phase+1] = (char *)rbuf;

      if ((size_t)rank + (1<<i) >= this->_comm->size())
 	{
	  this->_sbufln[phase]   = nbytes * (this->_comm->size() - rank);
	  this->_sbufln[phase+1] = nbytes * (rank + (1<<i) - this->_comm->size());
	}
      else
	{
	  this->_sbufln[phase] = nbytes * (1<<i);
	  this->_sbufln[phase+1] = 0;
	}
    }

  CollExchange<T_Mcast>::reset();
}
#endif
