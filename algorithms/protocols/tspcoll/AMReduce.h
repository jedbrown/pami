#ifndef __xlpgas_AMReduce_h__
#define __xlpgas_AMReduce_h__

namespace xlpgas
{
 template <class T_NI>
 class AMReduce : public AMExchange<AMHeader_reduce, AMReduce<T_NI>, T_NI>
  {
    typedef AMExchange<AMHeader_reduce, AMReduce<T_NI>, T_NI> base_type;
  public:
    void * operator new (size_t, void * addr) { return addr; }
    AMReduce (int ctxt, int team_id, AMHeader_reduce* ucb, int parent=-1)  :
      base_type (ctxt,team_id,ucb, parent) {
	this->_ret_values = true;
	this->_header->hdr.handler = XLPGAS_TSP_AMREDUCE_PREQ;
	this->FROMCHILD_AM         = XLPGAS_TSP_AMREDUCE_CREQ;
    }

    virtual void reset (int root, const void * sbuf, unsigned nbytes);
    virtual void root(void);
    virtual void* parent_incomming(void);
    virtual void allocate_space_result(void);
    virtual void merge_data (void);
  }; /* AMReduce */
} /* Xlpgas */

#endif /* __xlpgas_AMReduce_h__ */
