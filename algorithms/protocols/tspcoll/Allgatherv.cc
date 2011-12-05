/**
 * \file algorithms/protocols/tspcoll/Allgatherv.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Allgatherv.h"
#include "algorithms/protocols/tspcoll/Team.h"

#undef TRACE
#ifdef DEBUG_COLL
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* ************************************************************************* */
/*                       start Allgatherv                                    */
/* ************************************************************************* */
template <class T_NI>
xlpgas::Allgatherv<T_NI>::
Allgatherv (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset,T_NI* ni) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset, ni)
{
  pami_type_t allgathervtype = PAMI_TYPE_BYTE;
  this->_tmpbuf = NULL;
  this->_tmpbuflen = 0;
  this->_dbuf = NULL;
  //this->_nbytes = 0;
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
      this->_pwq[i].configure((char *)this->_sbuf[i], this->_sbufln[i], this->_sbufln[i], (TypeCode *)allgathervtype, (TypeCode *)allgathervtype);
      this->_pwq[i].reset();
    }
  this->_numphases   *= 3;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

/* ************************************************************************* */
/*                      start allgatherv operation                         */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::Allgatherv<T_NI>::reset (const void         * sbuf,
				void               * rbuf,
				TypeCode           * stype,
				size_t               stypecount,
				TypeCode           * rtype,
				size_t             * rtypecounts,
				size_t             * rdispls)
{
  size_t allsumbytes= 0;
  for(int i=0;i<(int)this->_comm->size();i++)
    allsumbytes+=(rtype->GetDataSize()*rtypecounts[i]);


  /* --------------------------------------------------- */
  /*    copy source buffer to dest buffer                */
  /* --------------------------------------------------- */
  memcpy ((char *)rbuf + rdispls[this->ordinal()], sbuf, rtype->GetDataSize()*rtypecounts[this->ordinal()]);
  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0, phase=this->_numphases/3; i<this->_numphases/3; i++, phase+=2)
    {
      int destindex = (this->ordinal()+2*this->_comm->size()-(1<<i))%this->_comm->size();
      this->_dest[phase]   =  this->_comm->index2Endpoint (destindex);
      this->_dest[phase+1]   = this->_dest[phase];
      this->_sbuf[phase]   = (char *)rbuf + rdispls[this->ordinal()];
      this->_sbuf[phase+1]   = (char *)rbuf;

      size_t phasesumbytes=0;
      for (int n=0; n < (1<<i); n++)
        phasesumbytes += (rtype->GetDataSize()*rtypecounts[(this->ordinal()+n)%this->_comm->size()]);

      this->_rbuf[phase]   = (char *)rbuf + ((rdispls[this->ordinal()] + phasesumbytes) % allsumbytes);
      this->_rbuf[phase+1]   = (char *)rbuf;
      if (rdispls[this->ordinal()] + phasesumbytes >= allsumbytes)
        {
          this->_sbufln[phase] = allsumbytes - rdispls[this->ordinal()];
          this->_sbufln[phase+1] = rdispls[this->ordinal()] + phasesumbytes - allsumbytes;
        }
      else
        {
          this->_sbufln[phase] = phasesumbytes;
          this->_sbufln[phase+1] = 0;
        }
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], stype, rtype);
      this->_pwq[phase].reset();
      this->_pwq[phase+1].configure((char *)this->_sbuf[phase+1], this->_sbufln[phase+1], this->_sbufln[phase+1], stype, rtype);
      this->_pwq[phase+1].reset();
    }
  /* ----------------------------------- */
  /* ----------------------------------- */
  xlpgas::CollExchange<T_NI>::reset();
}
