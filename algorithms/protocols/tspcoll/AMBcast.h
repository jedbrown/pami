#ifndef __xlpgas_AMBcast_h__
#define __xlpgas_AMBcast_h__

#include "algorithms/protocols/tspcoll/AMExchange.h"

namespace xlpgas
{

  /**
   * Active message Broadcast; One sided implementation;
   */
 template <class T_NI>
 class AMBcast : public AMExchange<AMHeader_bcast, AMBcast<T_NI>, T_NI>
  {
    typedef AMExchange<AMHeader_bcast, AMBcast<T_NI>, T_NI> base_type;
  public:
    void * operator new (size_t, void * addr) { return addr; }
    AMBcast (int ctxt, int team_id, AMHeader_bcast* ucb, int parent=-1)  :
      base_type (ctxt,team_id, ucb, parent) {
	this->_ret_values = false;
	this->_header->hdr.handler   = XLPGAS_TSP_AMBCAST_PREQ;
    }
    virtual void reset (int root, const void * sbuf, unsigned nbytes);
    virtual void root(void);
    virtual void* parent_incomming(void);
  }; /* AMBcast */


  void bcast_tree_collective_reg();

  /**
   * Broadcast implemented using active message broadcast
   */
 template <class T_NI>
 class BcastTree : public Collective<T_NI>
  {
    typedef Collective<T_NI> base_type;
    AMBcast<T_NI>* a;
    AMHeader_bcast header;
  public:
    void * operator new (size_t, void * addr) { return addr; }
    BcastTree (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
    base_type (ctxt, comm, kind, tag, NULL,0) {
      a=NULL;
    }

    virtual void reset (int root, const void * sbuf, void* rbuf, unsigned nbytes);
    virtual void kick  ();
    virtual bool isdone(void) const ;
  }; /* AMBcastTree */



} /* Xlpgas */

#include "algorithms/protocols/tspcoll/AMBcast.cc"

#endif /* __xlpgas_AMBcast_h__ */
