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

#ifndef __tspcoll_collbase_h__
#define __tspcoll_collbase_h__

#include "collectives/interface/lapiunix/common/include/pgasrt.h"
#include "NBColl.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//#define DEBUG_TSPCOLL 1
#undef TRACE
#ifdef DEBUG_TSPCOLL
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* *********************************************************************** */
/*  Base class for implementing a non-blocking collective using amsends.   */
/* *********************************************************************** */
namespace TSPColl
{
  class CollExchange: public NBColl
  {
  protected:
    static const int MAX_PHASES=64;
    typedef void (* cb_Coll_t) (CollExchange *, unsigned);
    
  public:  
    /* ------------------------------ */
    /*  public API                    */
    /* ------------------------------ */
    virtual void  kick             (void);
    virtual bool  isdone           () const;
    static void   amsend_reg       (void);

  protected:

    CollExchange                   (Communicator *, NBTag, 
				    int id, int off, bool strict=true,
				    void (*cb_complete)(void *)=NULL,
				    void * arg = NULL);
    void          reset            (void);


  private:
    
    /* ------------------------------ */
    /*  local functions               */
    /* ------------------------------ */
    
    void          send                     (int phase);
    static 
      __pgasrt_local_addr_t cb_incoming    (const __pgasrt_AMHeader_t * hdr,
					    void (**)(void *, void *),
					    void ** arg);
    static void   cb_recvcomplete          (void *, void * arg);
    static void   cb_senddone              (void *);
    
  protected:
    
    /* ------------------------------ */
    /* static: set by constructor     */
    /* ------------------------------ */
    
    int          _numphases;

    /* ------------------------------ */
    /* set by start()                 */
    /* ------------------------------ */
    
    int          _dest     [MAX_PHASES];    /* list of destination nodes     */
    void       * _sbuf     [MAX_PHASES];    /* list of source addresses      */
    void       * _rbuf     [MAX_PHASES];    /* list of destination addresses */
    size_t       _sbufln   [MAX_PHASES];    /* list of buffer lenghts        */
    cb_Coll_t    _cb_recv1 [MAX_PHASES];    /* immediate callback */
    cb_Coll_t    _cb_recv2 [MAX_PHASES];    /* callback to process buffer */
    
    /* --------------------------------- */
    /* STATE: changes during execution   */
    /* --------------------------------- */
    
  protected:
    int          _phase;                    /* phase in current execution    */
    int          _counter;                  /* how many times been reset     */
    int          _sendstarted; 
    int          _sendcomplete;             /* #sends complete               */
    int          _recvcomplete[MAX_PHASES]; /* #recv complete in each phase  */
    int          _cbcomplete  [MAX_PHASES]; /* #callbacks complete           */
    bool         _strict;                   /* early incoming msgs not perm. */
    
  private:
    
    /* ------------------------------ */
    /*      active message helpers    */
    /* ------------------------------ */
    
    struct AMHeader
    {
      __pgasrt_AMHeader_t hdr;
      NBTag               tag;
      int                 id;
      int                 offset;
      int                 counter;
      int                 phase;
    }
    _header [MAX_PHASES] __attribute__((__aligned__(16)));
    
    /* --------------------------------- */
    /* send & receive completion helper  */
    /* --------------------------------- */
    
    struct CompleteHelper
    {
      int                phase;
      int                counter;
      CollExchange     * base;
    }
    _cmplt [MAX_PHASES];

    DECL_MUTEX(_mutex);

    void internalerror (TSPColl::CollExchange::AMHeader *, int);
  };
};
  
/* *********************************************************************** */
/*                  register collexchange                                  */
/* *********************************************************************** */

inline void TSPColl::CollExchange::amsend_reg  (void)
{
  __pgasrt_tsp_amsend_reg (PGASRT_TSP_AMSEND_COLLEXCHANGE, cb_incoming);
}

/* *********************************************************************** */
/*                  CollExchange constructor                               */
/* *********************************************************************** */

inline TSPColl::CollExchange::
CollExchange (Communicator * comm, NBTag tag, int id, int offset, 
	      bool strict, void (*cb_complete)(void *), void *arg):
	       NBColl (comm, tag, id, cb_complete, arg), _strict(strict)
{
  _counter         = 0;
  _numphases       = -100 * tag;
  _phase           = _numphases+1;
  for (int i=0; i<MAX_PHASES; i++)
    {
      _sbuf[i]                 = NULL;
      _rbuf[i]                 = NULL;
      _sbufln[i]               = 0;
      _cb_recv1[i]             = NULL;
      _cb_recv2[i]             = NULL;
      _cmplt[i].phase          = i;
      _cmplt[i].base           = this;
      _cbcomplete[i]           = 0;
      _recvcomplete[i]         = 0;
      _header[i].hdr.handler   = (__pgasrt_AMHeaderHandler_t)
	PGASRT_TSP_AMSEND_COLLEXCHANGE;
      _header[i].hdr.headerlen = sizeof (struct AMHeader);
      _header[i].tag           = tag;
      _header[i].id            = id;
      _header[i].offset        = offset;
      _header[i].phase         = i;
      _header[i].counter       = 0;
    }

  _sendstarted = _sendcomplete = 0;
  MUTEX_INIT(&_mutex);
}

/* *********************************************************************** */
/*    reinitialize the state machine for another collective execution      */
/* *********************************************************************** */
inline void TSPColl::CollExchange::reset()
{
  _sendstarted = _sendcomplete = 0;
  _counter++;
  _phase = 0;
}

/* *********************************************************************** */
/*                   kick the state machine (make progress)                */
/* *********************************************************************** */

inline void TSPColl::CollExchange::kick()
{
  /* continued ATOMIC (code should be entered with mutex already locked */
  for (; _phase < _numphases; _phase++)
    {
      /* ---------------------------------------------------- */
      /* deal with sending what we have to send in this phase */
      /* ---------------------------------------------------- */

      if (_sendstarted <= _phase)
	{
	  _sendstarted++;
	  if (_sbuf[_phase]) 
	    { 
	      int phase = _phase;
	      MUTEX_UNLOCK(&_mutex);
	      send(phase); 
	      return; 
	    } 
	  else
	    _sendcomplete++;
	}

      /* ------------------------------------------------------------ */
      /* reception and callback : all complete? advance to next phase */
      /* ------------------------------------------------------------ */

      if (_cbcomplete[_phase] >= _counter) 
	{
	  continue;
	}

      /* ------------------------------------------------------- */
      /* special case: nothing to receive in this phase; advance */
      /* ------------------------------------------------------- */

      if (_rbuf[_phase] == NULL)
	{
	  assert (_cb_recv2[_phase] == NULL);
	  _recvcomplete[_phase]++;          /* no receive, no callback */
	  _cbcomplete[_phase]++;
	  assert (_recvcomplete[_phase] <= _counter);
	  assert (_cbcomplete[_phase] <= _counter);
	  continue;
	}

      /* ---------------------------------------------------- */
      /* reception not complete - NOT advancing to next phase */
      /* ---------------------------------------------------- */
      
      if (_recvcomplete[_phase] < _counter) goto the_end;

      /* -------------------------------------------------------- */
      /* reception complete - no callback - advance to next phase */
      /* -------------------------------------------------------- */

      if (_cb_recv2[_phase] == NULL) /* no cb */
	{
	  TRACE((stderr, "%d: NOCB tag=%d ctr=%d phase=%d\n",
		 PGASRT_MYNODE, _tag, _counter, _phase));
	  _cbcomplete[_phase]++;
	  continue;
	}

      /* -------------------------------------------------------- */
      /* cannot run callback until send in this phase is complete */
      /* -------------------------------------------------------- */

      if (_sendcomplete <= _phase) { TRACE((stderr, "*\n")); goto the_end; }

      /* ------------ */
      /* run callback */
      /* ------------ */

      TRACE((stderr, "%d: CBCK tag=%d ctr=%d phase=%d\n",
	     PGASRT_MYNODE, _tag, _counter, _phase));
      _cb_recv2[_phase] (this, _phase); 
      _cbcomplete[_phase]++;
    }

  TRACE((stderr, "%d: FINI tag=%d ctr=%d phase=%d/%d sendcmplt=%d\n",
	 PGASRT_MYNODE, _tag, _counter, 
	 _phase, _numphases, _sendcomplete));

  if (_cb_complete) 
    if (_phase == _numphases) { _phase++; _cb_complete (_arg); }

 the_end:
  ;
  /* END ATOMIC */
  MUTEX_UNLOCK(&_mutex);
}

/* *********************************************************************** */
/*    advance the progress engine                                          */
/* *********************************************************************** */

inline bool TSPColl::CollExchange::isdone() const
{
  return (_phase >= _numphases && _sendcomplete >= _numphases);
}

/* *********************************************************************** */
/*                     send an active message                              */
/* *********************************************************************** */

inline void TSPColl::CollExchange::send (int phase)
{
  TRACE((stderr, "%d: SEND tag=%d ctr=%d phase=%d tgt=%d nbytes=%d\n",
	 PGASRT_MYNODE,  _tag, _counter, phase, 
	 _dest[phase], _sbufln[phase]));
  
  _header[phase].counter = _counter;
  assert (_dest[phase] != -1);
  
  void * r = __pgasrt_tsp_amsend (_dest[phase],
				  & _header[phase].hdr,
				  (__pgasrt_local_addr_t) _sbuf[phase],
				  _sbufln[phase],
				  CollExchange::cb_senddone,
				  &_cmplt[phase]);
  assert (r == NULL);
}

/* *********************************************************************** */
/*                             send complete                               */
/* *********************************************************************** */

inline void TSPColl::CollExchange::cb_senddone (void * arg)
{
  CollExchange * base  = ((CompleteHelper *) arg)->base;
  MUTEX_LOCK(&base->_mutex);
  /* BEGIN ATOMIC */
  base->_sendcomplete++;
  TRACE((stderr, 
	 "%d: SENT tag=%d ctr=%d phase=%d/%d tgt=%d nbyt=%d cplt=%d\n",
	 PGASRT_MYNODE, 
	 base->_tag, base->_counter, 
	 base->_phase, base->_numphases,
	 base->_dest[base->_phase], base->_sbufln[base->_phase],
	 base->_sendcomplete));
  base->kick();
}

/* *********************************************************************** */
/*                   incoming active message                               */
/* *********************************************************************** */

inline __pgasrt_local_addr_t TSPColl::CollExchange::
cb_incoming (const struct __pgasrt_AMHeader_t * hdr,
	     void (** completionHandler)(void *, void *),
	     void ** arg)
{
  struct AMHeader * header = (struct AMHeader *) hdr;
  void * base0 = NBCollManager::instance()->find (header->tag, header->id);
  if (base0 == NULL)
    __pgasrt_fatalerror (-1, "%d: incoming: cannot find coll=<%d,%d>",
			 PGASRT_MYNODE, header->tag, header->id);
  
  CollExchange * b = (CollExchange * ) ((char *)base0 + header->offset);
  TRACE((stderr, "%d: INC  tag=%d id=%d ctr=%d phase=%d nphases=%d "
	 "msgctr=%d msgphase=%d\n",
	 PGASRT_MYNODE, header->tag, header->id, b->_counter, 
	 b->_phase, b->_numphases, 
	 header->counter, header->phase));

  assert (b->_header[0].id == header->id);
  assert (b->_numphases > 0);
  if (b->_strict)
    {
      if (header->counter != b->_counter || b->_phase >= b->_numphases) 
	b->internalerror (header, __LINE__);
    }
  
  b->_cmplt[header->phase].counter = header->counter;
  *completionHandler = &CollExchange::cb_recvcomplete;
  *arg = &b->_cmplt[header->phase];
  __pgasrt_local_addr_t z = (__pgasrt_local_addr_t) b->_rbuf[header->phase];
  if (z == NULL) b->internalerror (header, __LINE__);
  return z;
}

/* *********************************************************************** */
/*                  active message reception complete                      */
/* *********************************************************************** */

inline void 
TSPColl::CollExchange::cb_recvcomplete (void * unused, void * arg)
{
  CollExchange * base  = ((CompleteHelper *) arg)->base;
  unsigned  phase = ((CompleteHelper *) arg)->phase;
  // int  counter = ((CompleteHelper *)arg)->counter;
  if (base->_strict)
    if (base->_recvcomplete[phase] > base->_counter) 
      base->internalerror (NULL, __LINE__);
  /* BEGIN ATOMIC */
  MUTEX_LOCK(&base->_mutex);
  base->_recvcomplete[phase]++;
  TRACE((stderr, "%d: IN_D tag=%d ctr=%d phase=%d msgphase=%d cplt=%d\n",
	 PGASRT_MYNODE, base->_tag, 
	 base->_counter, base->_phase, phase, base->_recvcomplete[phase]));
  if (base->_cb_recv1[phase]) base->_cb_recv1[phase](base, phase);
  base->kick();
}

/* *********************************************************************** */
/*      something bad happened. We print the state as best as we can.      */
/* *********************************************************************** */

inline void 
TSPColl::CollExchange::internalerror (AMHeader * header, int lineno)
{
  if (header)
    fprintf (stderr, "%d: CollExchange internal: line=%d "
	     "tag=%d id=%d phase=%d/%d ctr=%d "
	     "header: tag=%d id=%d phase=%d ctr=%d\n",
	     PGASRT_MYNODE, lineno,
             _tag, _instID, 
	     _phase, _numphases, _counter,
	     header->tag, header->id, header->phase,
	     header->counter);
  else
    fprintf (stderr, "%d: CollExchange internal: line=%d "
	     "tag=%d id=%d phase=%d/%d ctr=%d\n",
	     PGASRT_MYNODE, lineno,
	     _tag, _instID,
	     _phase, _numphases, _counter);
  abort();
}

#undef TRACE
#endif
