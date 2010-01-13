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
 * \file algorithms/protocols/tspcoll/BinomBcast.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_BinomBcast_h__
#define __algorithms_protocols_tspcoll_BinomBcast_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./CollExchange.h"

/* *********************************************************************** */
/*                      binomial broadcast class                           */
/* *********************************************************************** */

namespace TSPColl
{
  template<class T_Mcast>
  class BinomBcast: public CollExchange<T_Mcast>
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    BinomBcast (XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int tagoff);
    void reset (int root, const void * sbuf, void *buf, size_t);
  private:
    char _dummy;
  };
};

/* *********************************************************************** */
/* *********************************************************************** */
template<class T_Mcast>
inline TSPColl::BinomBcast<T_Mcast>::
BinomBcast(XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int tagoff) :
  CollExchange<T_Mcast>(comm, tag, instID, tagoff, false)
{
  this->_numphases = -1; for (int n=2*this->_comm->size()-1; n>0; n>>=1) this->_numphases++;
  for (int i=0; i< this->_numphases; i++)
    {

      int rank = this->_comm->virtrank();
      int destindex = (rank+2*this->_comm->size()-(1<<i))%this->_comm->size();
      this->_dest[i] = this->_comm->absrankof(destindex);
      this->_sbuf[i] = &_dummy;
      this->_rbuf[i] = &_dummy;
      this->_sbufln[i] = 1;
    }
  this->_numphases   *= 2;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

/* ********************************************************************** */
/*                    start a new binomial broadcast                      */
/* ********************************************************************** */
template<class T_Mcast>
inline void TSPColl::BinomBcast<T_Mcast>::
reset (int rootindex, const void * sbuf, void * buf, size_t nbytes)
{
  if ((size_t)rootindex >= this->_comm->size())
    CCMI_FATALERROR (-1, "Invalid root index in BinomBcast");

  /* --------------------------------------------------- */
  /* --------------------------------------------------- */

  int rank = this->_comm->virtrank();
  if (rootindex == rank && sbuf != buf)
    memcpy (buf, sbuf, nbytes);

  /* --------------------------------------------------- */
  /* --------------------------------------------------- */

  int myrelrank = (rank + this->_comm->size() - rootindex) % this->_comm->size();
  for (int i=0, phase=this->_numphases/2; i<this->_numphases/2; i++, phase++)
    {
      int  dist       = 1<<(this->_numphases/2-1-i);
      int  sendmask   = (1<<(this->_numphases/2-i))-1;
      int  destrelrank= myrelrank + dist;
      int  srcrelrank = myrelrank - dist;
      bool dosend     = ((myrelrank&sendmask)==0)&&((size_t)destrelrank<this->_comm->size());
      bool dorecv     = ((srcrelrank&sendmask)==0)&&(srcrelrank>=0);
      int  destindex  = (destrelrank + rootindex)%this->_comm->size();
      this->_dest[phase]    = this->_comm->absrankof(destindex);
      this->_sbuf[phase]    = dosend ? buf : NULL;
      this->_sbufln[phase]  = dosend ? nbytes : 0;
      this->_rbuf[phase]    = dorecv ? buf : NULL;
    }

  CollExchange<T_Mcast>::reset();
}

#endif
