/**
 * \file algorithms/protocols/tspcoll/Scatter.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_Scatter_h__
#define __algorithms_protocols_tspcoll_Scatter_h__

#include "algorithms/protocols/tspcoll/Team.h"
#include "algorithms/protocols/tspcoll/CollExchange.h"

#undef TRACE
#ifdef DEBUG_COLL
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

namespace xlpgas
{
  template <class T_NI>
  class Scatter : public Collective<T_NI>
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }

    Scatter (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset,T_NI* ni) :
      Collective<T_NI> (ctxt, comm, kind, tag, NULL, NULL, ni)
      {
	_rcvcount = 1;

	_header = (struct AMHeader *)__global.heap_mm->malloc (sizeof(struct AMHeader) );
	assert (_header != NULL);

	_header->hdr.handler   = XLPGAS_TSP_AMSEND_S;
	_header->hdr.headerlen = sizeof (struct AMHeader);
	_header->kind          = kind;
	_header->tag           = tag;
	_header->offset        = offset;
	_header->senderID      = this->ordinal();
	_header->dest_ctxt     = -1;
      }

    ~Scatter()
      {
        __global.heap_mm->free(_header);
      }


    static  void  amsend_reg       (xlpgas_AMHeaderReg_t amsend_regnum) {
      xlpgas_tsp_amsend_reg (amsend_regnum, Scatter::cb_incoming);
    }

    virtual void reset (int root,
		    const void         * sbuf,
		    void               * dbuf,
		    TypeCode           * stype,
		    size_t               stypecount,
		    TypeCode           * rtype,
		    size_t               rtypecount);

    static void cb_senddone (void * ctxt, void * arg, pami_result_t res);

    virtual void kick    ();
    virtual bool isdone  (void) const;

    static inline void cb_incoming(pami_context_t    context,
                                   void            * cookie,
                                   const void      * header_addr,
                                   size_t            header_size,
                                   const void      * pipe_addr,
                                   size_t            data_size,
                                   pami_endpoint_t   origin,
                                   pami_recv_t     * recv);

    static void cb_recvcomplete (void * unused, void * arg, pami_result_t result);

  private:
    const char         * _sbuf;         /* send buffer    */
    char               * _rbuf;         /* receive buffer */
    size_t               _len;          /* msg length     */
    size_t               _root;
    PAMI::PipeWorkQueue  _pwq;
    int                  _rcvcount;

    struct AMHeader
    {
      xlpgas_AMHeader_t    hdr;
      CollectiveKind      kind;
      int                 tag;
      int                 offset;
      int                 counter;
      int                 phase;
      int                 senderID;
      int                 dest_ctxt;
    } * _header;

  }; /* Scatter */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/Scatter.cc"

#endif /* __xlpgas_Scatter_h__ */
