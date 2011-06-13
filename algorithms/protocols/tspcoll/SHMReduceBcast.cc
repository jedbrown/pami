#include "algorithms/protocols/tspcoll/SHMReduceBcast.h"
#include "algorithms/protocols/tspcoll/Team.h"

#define XLPGAS_SHM_ROOT 0

extern "C" char* xlpgas_shm_buf;
extern "C" char* xlpgas_shm_buf_bcast;

template <class T_NI>
xlpgas::SHMReduce<T_NI>::
SHMReduce (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
  Collective<T_NI> (ctxt, comm, kind, tag, NULL,NULL), fl(comm->size(),comm->ordinal(),XLPGAS_SHM_ROOT,xlpgas_shm_buf)
{
}


/* ************************************************************************* */
/*                      start a bcast operation                         */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::SHMReduce<T_NI>::reset (int rootindex,
			       const void         * sbuf, 
			       void               * dbuf,
			       xlpgas_ops_t       op,
			       xlpgas_dtypes_t    dt,
			       unsigned           nelems,
			       user_func_t*       uf)
{

  cau_reduce_op_t cau_op = xlpgas::cau_op_dtype(op,dt);
  fl.reduce ( (int64_t*)sbuf, (int64_t*)dbuf , cau_op);			
}

template <class T_NI>
xlpgas::SHMBcast<T_NI>::
SHMBcast (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
  Collective<T_NI> (ctxt, comm, kind, tag, NULL,NULL), fl(comm->size(),comm->ordinal(),XLPGAS_SHM_ROOT,xlpgas_shm_buf_bcast)
{
}


/* ************************************************************************* */
/*                      start a bcast operation                         */
/* ************************************************************************* */
template <class T_NI>
void xlpgas::SHMBcast<T_NI>::reset (int rootindex,
			      const void         * sbuf, 
			      void               * dbuf,
			      unsigned           nbytes)
{
  *((int64_t*)dbuf) = *((int64_t*)sbuf);
  fl.bcast ((xlpgas_local_addr_t)dbuf, nbytes);			
}
