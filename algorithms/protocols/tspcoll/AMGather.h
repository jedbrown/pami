/**
 * \file algorithms/protocols/tspcoll/AMGather.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_AMGather_h__
#define __algorithms_protocols_tspcoll_AMGather_h__

namespace xlpgas
{
 template <class T_NI>
 class AMGather : public AMExchange<AMHeader_gather, AMGather<T_NI>, T_NI>
  {
    typedef AMExchange<AMHeader_gather, AMGather<T_NI>, T_NI> base_type;
  public:
    void * operator new (size_t, void * addr) { return addr; }
    AMGather (int ctxt, int team_id, AMHeader_gather* ucb, int parent=-1)  :
      base_type (ctxt,team_id,ucb, parent) {
	this->_ret_values = true;
	this->_header->hdr.handler = XLPGAS_TSP_AMGATHER_PREQ;
	this->FROMCHILD_AM         = XLPGAS_TSP_AMGATHER_CREQ;
    }

    virtual void reset (int root, const void * sbuf, unsigned nbytes);
    virtual void root(void);
    virtual void* parent_incomming(void);
    virtual void allocate_space_result(void);
    virtual void adjust_header(size_t);
    virtual void merge_data (void);
  }; /* AMGather */
} /* Xlpgas */

#endif /* __xlpgas_AMGather_h__ */
