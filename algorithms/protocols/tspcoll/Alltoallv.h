/**
 * \file algorithms/protocols/tspcoll/Alltoallv.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_Alltoallv_h__
#define __algorithms_protocols_tspcoll_Alltoallv_h__

#include "algorithms/protocols/tspcoll/Alltoall.h"

#undef TRACE
#ifdef DEBUG_COLL
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

namespace xlpgas
{
  template <class T_NI>
  class Alltoallv : public Alltoall<T_NI>
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }

    Alltoallv (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset, T_NI* ni) :
      Alltoall<T_NI> (ctxt, comm, kind, tag, offset,ni)
      {
      }

    virtual void reset (const void   *sbuf,
			void *        dbuf,
			TypeCode     *stype,
			const size_t *scnts,
			const size_t *sdispls,
			TypeCode     *rtype,
			const size_t *rcnts,
			const size_t *rdispls);

    virtual void kick    ();

    static  void  amsend_reg       (xlpgas_AMHeaderReg_t amsend_regnum) {
      xlpgas_tsp_amsend_reg (amsend_regnum, Alltoallv::cb_incoming_v);
    }

    static inline void cb_incoming_v(pami_context_t    context,
                                     void            * cookie,
                                     const void      * header_addr,
                                     size_t            header_size,
                                     const void      * pipe_addr,
                                     size_t            data_size,
                                     pami_endpoint_t   origin,
                                     pami_recv_t     * recv);

  private:
    const size_t    * _scnts;      /* send counts    */
    const size_t    * _sdispls;    /* send displacements */
    const size_t    * _rcnts;      /* recv counts    */
    const size_t    * _rdispls;    /* recv displacements */

  }; /* Alltoallv */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/Alltoallv.cc"

#endif /* __xlpgas_Alltoallv_h__ */
