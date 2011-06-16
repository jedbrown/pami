/**
 * \file algorithms/protocols/tspcoll/ShortAllreduce.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Allreduce.h"
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
/*                       start a short allreduce                              */
/* ************************************************************************* */
template <class T_NI>
xlpgas::Allreduce::Short<T_NI>::
Short (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset)
{
  pami_type_t allreducetype = PAMI_TYPE_BYTE;
  this->_dbuf = NULL;
  this->_nelems = 0;
  for (this->_logMaxBF = 0; (1<<(this->_logMaxBF+1)) <= (int)this->_comm->size(); this->_logMaxBF++) ;
  int maxBF  = 1<<this->_logMaxBF;
  int nonBF  = this->_comm->size() - maxBF;
  int phase  = 0;

  /* -------------------------------------------- */
  /* phase 0: gather buffers from ordinals > n2prev  */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      //xlpgas_endpoint_t rdest = comm->endpoint (comm->ordinal() - maxBF);
      //_dest    [phase] = (comm->ordinal() >= maxBF) ? rdest : -1;
      this->_dest    [phase] = comm->endpoint (comm->ordinal() - maxBF);
      this->_sbuf    [phase] = NULL;    /* unknown */
      this->_rbuf    [phase] = ((int)comm->ordinal() < nonBF)  ? this->_phasebuf[phase][0] : NULL;
      this->_cb_rcvhdr[phase] = ((int)comm->ordinal() < nonBF)  ? cb_switchbuf : NULL;
      this->_postrcv [phase] = ((int)comm->ordinal() < nonBF)  ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;       /* unknown */
      this->_bufctr  [phase] = 0;
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], (TypeCode *)allreducetype, (TypeCode *)allreducetype);
      this->_pwq[phase].reset();
      phase ++;
    }

  /* -------------------------------------------- */
  /* butterfly phases                             */
  /* -------------------------------------------- */

  for (int i=0; i<_logMaxBF; i++)
    {
      //unsigned rdest   = comm->endpoint (comm->ordinal() ^ (1<<i));
      //_dest    [phase] = ((int)comm->ordinal() < maxBF) ? rdest : -1;
      this->_dest [phase] = comm->endpoint (comm->ordinal() ^ (1<<i));
      this->_sbuf    [phase] = NULL;     /* unknown */
      this->_rbuf    [phase] = ((int)comm->ordinal() < maxBF) ? this->_phasebuf[phase][0] : NULL;
      this->_cb_rcvhdr[phase] = ((int)comm->ordinal() < maxBF) ? cb_switchbuf : NULL;
      this->_postrcv [phase] = ((int)comm->ordinal() < maxBF) ? cb_allreduce : NULL;
      this->_sbufln  [phase] = 0;        /* unknown */
      this->_bufctr  [phase] = 0;
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], (TypeCode *)allreducetype, (TypeCode *)allreducetype);
      this->_pwq[phase].reset();
      phase ++;
    }

  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)
    {
      ///unsigned rdest   = comm->endpoint (comm->ordinal() + maxBF);
      //_dest    [phase] = ((int)comm->ordinal() < nonBF)  ? rdest : -1;
      this->_dest    [phase] = comm->endpoint (comm->ordinal() + maxBF);
      this->_sbuf    [phase] = NULL;     /* unknown */
      this->_rbuf    [phase] = NULL;     /* unknown */
      this->_cb_rcvhdr[phase] = NULL;
      this->_postrcv [phase] = NULL;
      this->_sbufln  [phase] = 0;        /* unknown */
      this->_bufctr  [phase] = 0;
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], (TypeCode *)allreducetype, (TypeCode *)allreducetype);
      this->_pwq[phase].reset();
      phase ++;
    }

  this->_numphases    = phase;
  this->_phase        = this->_numphases;
  this->_sendcomplete = this->_numphases;
}

/* ************************************************************************* */
/*                     allreduce executor                                    */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::Allreduce::Short<T_NI>::
cb_allreduce (CollExchange<T_NI> *coll, unsigned phase)
{
  xlpgas::Allreduce::Short<T_NI> * ar = (xlpgas::Allreduce::Short<T_NI> *) coll;
  int c = (ar->_counter+1) & 1;
  /*int *s = (int*)(ar->_phasebuf[phase][c]);
  int *d = (int*)(ar->_dbuf);
  for(int i=0;i<ar->_nelems;++i){
    printf("L%d Add %d %d\n",XLPGAS_MYNODE, s[i],d[i]);
  }
  */
  void * inputs[] = {ar->_dbuf, ar->_phasebuf[phase][c]};
  ar->_cb_allreduce (ar->_dbuf, inputs, 2, ar->_nelems);
}
template <class T_NI>
xlpgas_local_addr_t xlpgas::Allreduce::Short<T_NI>::
cb_switchbuf (CollExchange<T_NI> * coll, unsigned phase, unsigned counter)
{
  xlpgas::Allreduce::Short<T_NI> * ar = (xlpgas::Allreduce::Short<T_NI> *) coll;
  int c = (counter+1) & 1;
  return (xlpgas_local_addr_t) (ar->_phasebuf[phase][c]);
}

/* ************************************************************************* */
/*                      start a short allreduce operation                     */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::Allreduce::Short<T_NI>::reset (const void         * sbuf,
				     void               * dbuf,
				     pami_data_function   op,
				     TypeCode           * sdt,
				     size_t               nelems,
                     TypeCode           * rdt,
				     user_func_t        * uf)
{
  assert (sbuf != NULL);
  assert (dbuf != NULL);

  xlpgas::CollExchange<T_NI>::reset();//to lock

  _uf = uf; //user function pointer
  /* --------------------------------------------------- */
  /*         copy source to destination if necessary     */
  /* --------------------------------------------------- */
  /*
    printf("L%d: short allreduce [teamid=%d rank=%d sizeworld=%d nelems=%d]\n",
	 XLPGAS_MYNODE,
	 this->_comm->commID(),
	 this->_comm->ordinal(),
	 this->_comm->size(),
	 nelems
	 );
  */
  _dbuf   = dbuf;
  _nelems = nelems;
  size_t datawidth = sdt->GetDataSize();
  if (sbuf != dbuf) memcpy (dbuf, sbuf, nelems * datawidth);

  /* --------------------------------------------------- */
  /*  set source and destination buffers and node ids    */
  /* We deal with non-power-of-2 nodes                   */
  /* --------------------------------------------------- */

  int maxBF  = 1<<_logMaxBF;
  int nonBF  = this->_comm->size() - maxBF;
  int phase  = 0;

  if (nonBF > 0)   /* phase 0: gather buffers from ordinals > n2prev */
    {
      this->_sbuf    [phase] = ((int)this->_comm->ordinal() >= maxBF) ? dbuf  : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], sdt, rdt);
      this->_pwq[phase].reset();
      phase ++;
    }

  /* -------------------------------------------- */
  /* butterfly phases                             */
  /* -------------------------------------------- */

  for (int i=0; i<this->_logMaxBF; i++)   /* middle phases: butterfly pattern */
    {
      this->_sbuf    [phase] = ((int)this->_comm->ordinal() < maxBF) ? dbuf  : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], sdt, rdt);
      this->_pwq[phase].reset();
      phase ++;
    }


  /* -------------------------------------------- */
  /*  last phase: rebroadcast to non-power-of-2   */
  /* -------------------------------------------- */

  if (nonBF > 0)   /*  last phase: collect results */
    {
      this->_sbuf    [phase] = ((int)this->_comm->ordinal() < nonBF)  ? dbuf  : NULL;
      this->_rbuf    [phase] = ((int)this->_comm->ordinal() >= maxBF) ? dbuf  : NULL;
      this->_sbufln  [phase] = nelems * datawidth;
      this->_pwq[phase].configure((char *)this->_sbuf[phase], this->_sbufln[phase], this->_sbufln[phase], sdt, rdt);
      this->_pwq[phase].reset();
      phase ++;
    }
  assert (phase == this->_numphases);
  this->_cb_allreduce = getcallback (op, sdt);
}
