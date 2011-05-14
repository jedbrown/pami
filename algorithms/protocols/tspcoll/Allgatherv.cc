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
Allgatherv (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset)
{
  this->_tmpbuf = NULL;
  this->_tmpbuflen = 0;
  this->_dbuf = NULL;
  //this->_nbytes = 0;
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
/*                      start allgatherv operation                         */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::Allgatherv<T_NI>::reset (const void         * sbuf,
				void               * rbuf,
				size_t             * lengths)
{
  size_t allsumbytes= 0;
  for(int i=0;i<(int)this->_comm->size();i++)
    allsumbytes+=lengths[i];

  size_t mysumbytes = 0;
  for(int i=0;i<(int)this->_comm->ordinal();i++)
    mysumbytes +=lengths[i];

  /* --------------------------------------------------- */
  /*    copy source buffer to dest buffer                */
  /* --------------------------------------------------- */
  memcpy ((char *)rbuf + mysumbytes, sbuf, lengths[this->_comm->ordinal()]);
  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0, phase=this->_numphases/3; i<this->_numphases/3; i++, phase+=2)
    {
      int destindex = (this->_comm->ordinal()+2*this->_comm->size()-(1<<i))%this->_comm->size();
      this->_dest[phase]   =  this->_comm->endpoint (destindex);
      this->_dest[phase+1]   = this->_dest[phase];
      this->_sbuf[phase]   = (char *)rbuf + mysumbytes;
      this->_sbuf[phase+1]   = (char *)rbuf;

      size_t phasesumbytes=0;
      for (int n=0; n < (1<<i); n++)
        phasesumbytes += lengths[(this->_comm->ordinal()+n)%this->_comm->size()];

      this->_rbuf[phase]   = (char *)rbuf+ ((mysumbytes + phasesumbytes) % allsumbytes
);
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
  xlpgas::CollExchange<T_NI>::reset();
}
