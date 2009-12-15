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
/**
 * \file algorithms/protocols/tspcoll/Allgatherv.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_Allgatherv_h__
#define __algorithms_protocols_tspcoll_Allgatherv_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./CollExchange.h"

/* *********************************************************************** */
/*                     Allgather (Bruck's algorithm)                       */
/* *********************************************************************** */

namespace TSPColl
{
  template <class T_Mcast>
  class Allgatherv: public CollExchange<T_Mcast>
  {
  public:
    void * operator new (size_t, void * addr)    { return addr; }
    Allgatherv (XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int offset);
    void reset (const void *, void *, size_t * lengths);
  private:
    char _dummy;
  };
}

/* *********************************************************************** */
/*                   Allgather constructor                                 */
/* *********************************************************************** */
template <class T_Mcast>
inline TSPColl::Allgatherv<T_Mcast>::Allgatherv (XMI_GEOMETRY_CLASS * comm, NBTag tag,
					int instID, int off):
  CollExchange<T_Mcast> (comm, tag, instID, off, false)
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
  this->_numphases   *= 3;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

/* **************************************************************** */
/*              start a new allgather operation                     */
/* **************************************************************** */
template <class T_Mcast>
inline void TSPColl::
Allgatherv<T_Mcast>::reset (const void *sbuf, void *rbuf, size_t * lengths)
{
  size_t allsumbytes= 0;
  for(size_t i=0;i<this->_comm->size();i++)
    allsumbytes+=lengths[i];

  size_t mysumbytes = 0;
  int rank = this->_comm->virtrank();
  for(int i=0;i<rank;i++)
    mysumbytes +=lengths[i];

  /* --------------------------------------------------- */
  /*    copy source buffer to dest buffer                */
  /* --------------------------------------------------- */

  memcpy ((char *)rbuf + mysumbytes, sbuf, lengths[rank]);

  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0, phase=this->_numphases/3; i<this->_numphases/3; i++, phase+=2)
    {
      int destindex = (rank+2*this->_comm->size()-(1<<i))%this->_comm->size();
      this->_dest[phase]   =  this->_comm->absrankof (destindex);
      this->_dest[phase+1]   =this->_dest[phase];
      this->_sbuf[phase]   = (char *)rbuf + mysumbytes;
      this->_sbuf[phase+1]   = (char *)rbuf;

      size_t phasesumbytes=0;
      for (int n=0; n < (1<<i); n++)
	phasesumbytes += lengths[(rank+n)%this->_comm->size()];

      this->_rbuf[phase]   = (char *)rbuf+ ((mysumbytes + phasesumbytes) % allsumbytes);
      this->_rbuf[phase+1]   = (char *)rbuf;
      if (mysumbytes + phasesumbytes >= allsumbytes)
 	{
	  this->_sbufln[phase] = allsumbytes - mysumbytes;
	  this->_sbufln[phase+1] = mysumbytes + phasesumbytes - allsumbytes;
	}
      else
	{
	  this->_sbufln[phase] = phasesumbytes;
	  this->_sbufln[phase+1] = 0;
	}
    }

  /* ----------------------------------- */
  /* ----------------------------------- */

  CollExchange<T_Mcast>::reset();
}
#endif
