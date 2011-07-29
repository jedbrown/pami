/**
 * \file algorithms/protocols/tspcoll/Broadcast.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Broadcast.h"
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

//#define DEBUG_ALLREDUCE 1
#undef TRACE
#ifdef DEBUG_ALLREDUCE
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* ************************************************************************* */
/*                       start a long broadcast                              */
/* ************************************************************************* */
template <class T_NI>
xlpgas::Broadcast<T_NI>::
Broadcast (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset,T_NI* ni) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset, ni)
{
  pami_type_t bcasttype = PAMI_TYPE_BYTE;
  this->_tmpbuf = NULL;
  this->_tmpbuflen = 0;
  this->_dbuf = NULL;
  this->_nbytes = 0;
  this->_numphases = -1; for (int n=2*this->_comm->size()-1; n>0; n>>=1) this->_numphases++;
  for (int i=0; i< this->_numphases; i++)
    {
      int destindex = (this->ordinal()+2*this->_comm->size()-(1<<i))%this->_comm->size();
      this->_dest[i] = this->_comm->endpoint(destindex);
      this->_sbuf[i] = &this->_dummy;
      this->_rbuf[i] = &this->_dummy;
      this->_sbufln[i] = 1;
      this->_pwq[i].configure((char *)this->_sbuf[i], this->_sbufln[i], this->_sbufln[i], (TypeCode *)bcasttype, (TypeCode *)bcasttype);/*The root doesn't need to send to itself*/
      this->_pwq[i].reset();
    }
  this->_numphases   *= 2;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}


/* ************************************************************************* */
/*                      start a bcast operation                         */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::Broadcast<T_NI>::reset (int rootindex,
			       const void         * sbuf,
			       void               * dbuf,
			       TypeCode           * type,
			       size_t               typecount)
{
  if(rootindex == (int)this->ordinal()) {
    assert (sbuf != NULL);
  }

  assert (dbuf != NULL);
  size_t nbytes = type->GetDataSize() * typecount;
  if (rootindex >= (int)this->_comm->size())
    xlpgas_fatalerror (-1, "Invalid root index in Bcast");

  /* --------------------------------------------------- */
  /* --------------------------------------------------- */

  if (rootindex == (int)this->ordinal() && sbuf != dbuf){
    memcpy (dbuf, sbuf, nbytes);
  }

  int myrelrank = (this->ordinal() + this->_comm->size() - rootindex) % this->_comm->size();
  for (int i=0, phase=this->_numphases/2; i<this->_numphases/2; i++, phase++)
    {
      int  dist       = 1<<(this->_numphases/2-1-i);
      int  sendmask   = (1<<(this->_numphases/2-i))-1;
      int  destrelrank= myrelrank + dist;
      int  srcrelrank = myrelrank - dist;
      bool dosend     = ((myrelrank&sendmask)==0)&&(destrelrank<(int)this->_comm->size());
      bool dorecv     = ((srcrelrank&sendmask)==0)&&(srcrelrank>=0);
      int  destindex  = (destrelrank + rootindex)%this->_comm->size();
      this->_dest[phase]    = this->_comm->endpoint(destindex);
      this->_sbuf[phase]    = dosend ? dbuf : NULL;
      this->_sbufln[phase]  = dosend ? nbytes : 0;
      this->_rbuf[phase]    = dorecv ? dbuf : NULL;
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], type, type);/*The root doesn't need to send to itself*/
      this->_pwq[phase].reset();
    }
  xlpgas::CollExchange<T_NI>::reset();
  return;

  /* --------------------------------------------------- */
  /* --------------------------------------------------- */
  /*
  for (_logMaxBF = 0; (1<<(_logMaxBF+1)) <= _comm->size(); _logMaxBF++) ;
  int maxBF  = 1<<_logMaxBF;
  int nonBF  = _comm->size() - maxBF;
  int phase  = 0;

  _numphases = -1; for (int n=2*_comm->size()-1; n>0; n>>=1) _numphases++;
  _numphases   *= 2;

  int myrelrank = (ordinal() + _comm->size() - rootindex) % _comm->size();
  phase=0;
  for (int i=0; i<_numphases/2; i++)
    {
      int  dist       = 1<<(_numphases/2-1-i);
      int  sendmask   = (1<<(_numphases/2-i))-1;
      int  destrelrank= myrelrank + dist;
      int  srcrelrank = myrelrank - dist;
      bool dosend     = ((myrelrank&sendmask)==0)&&(destrelrank<_comm->size());
      bool dorecv     = ((srcrelrank&sendmask)==0)&&(srcrelrank>=0);
      int  destindex  = (destrelrank + rootindex)%_comm->size();

      //_dest[phase]    = _comm->endpoint(destindex);
      //_sbuf[phase]    = dosend ? dbuf : NULL;
      //_sbufln[phase]  = dosend ? nbytes : 0;
      //_rbuf[phase]    = dorecv ? dbuf : NULL;

      //ask permision
      //_dest[phase]    = _comm->endpoint(destindex);
      //_sbuf[phase]    = dosend ? &_dummy : NULL;
      //_sbufln[phase]  = dosend ? 1 : 0;
      //_rbuf[phase]    = dorecv ? &_dummy : NULL;
      //_postrcv[phase] = NULL;

      //printf("L%d: in phase %d send to %d : dosend=%d dorecv=%d\n", ordinal(), phase, _dest[phase].node, dosend, dorecv);
      //phase++;

      //broadcast data
      _dest[phase]    = _comm->endpoint(destindex);
      _sbuf[phase]    = dosend ? dbuf : NULL;
      _sbufln[phase]  = dosend ? nbytes : 0;
      _rbuf[phase]    = dorecv ? dbuf : NULL;
      _postrcv[phase] = NULL;

      //printf("L%d: in phase %d send to %d : dosend=%d dorecv=%d\n", ordinal(), phase, _dest[phase].node, dosend, dorecv);

      phase++;
    }

  _numphases = phase;
  //printf("%d: NUMPHASES =%d\n", ordinal(), _numphases);
  xlpgas::CollExchange::reset();
  */
}
