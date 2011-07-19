/**
 * \file algorithms/protocols/tspcoll/Alltoall.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_Alltoall_h__
#define __algorithms_protocols_tspcoll_Alltoall_h__

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
  class Alltoall : public Collective<T_NI>
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }

    Alltoall (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
    Collective<T_NI> (ctxt, comm, kind, tag, NULL, NULL)
      {
	_sndcount[0] = comm->size(); _sndcount[1] = comm->size();
	_rcvcount[0] = comm->size(); _rcvcount[1] = comm->size();
	_odd = 1;

	_headers = (struct AMHeader *)__global.heap_mm->malloc (sizeof(struct AMHeader) * comm->size());
	assert (_headers != NULL);

	for (int i=0; i<(int)comm->size(); i++)
	  {
	    _headers[i].hdr.handler   = XLPGAS_TSP_AMSEND_COLLA2A;
	    _headers[i].hdr.headerlen = sizeof (struct AMHeader);
	    _headers[i].kind          = kind;
	    _headers[i].tag           = tag;
	    _headers[i].offset        = offset;
	    _headers[i].senderID      = comm->ordinal();
	  }
      }

    ~Alltoall()
      {
          __global.heap_mm->free(_headers);
      }

    static  void  amsend_reg       (xlpgas_AMHeaderReg_t amsend_regnum) {
      xlpgas_tsp_amsend_reg (amsend_regnum, Alltoall::cb_incoming);
    }

    virtual void reset (const void * sbuf,
			void * dbuf,
			unsigned nbytes);

    static void cb_senddone (void * ctxt, void * arg, pami_result_t result);

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

  protected:
    const char    * _sbuf;         /* send buffer    */
    char          * _rbuf;         /* receive buffer */
    size_t          _len;          /* msg length     */

    int             _sndcount[2], _rcvcount[2], _odd;

    struct AMHeader
    {
      xlpgas_AMHeader_t    hdr;
      CollectiveKind      kind;
      int                 tag;
      int                 offset;
      int                 counter;
      int                 phase;
      int                 dest_ctxt;
      int senderID;
    } * _headers;

  }; /* Alltoall */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/Alltoall.cc"

#endif /* __xlpgas_Alltoall_h__ */
