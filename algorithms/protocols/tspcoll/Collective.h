#ifndef __xlpgas_Collective_h__
#define __xlpgas_Collective_h__

#include <stdio.h>
#include "algorithms/protocols/tspcoll/Array.h"
#include "algorithms/ccmi.h"
#include "sys/pami.h"
#include "common/type/TypeCode.h"
#include "common/type/TypeMachine.h"
#include "algorithms/protocols/allreduce/ReduceFunctions.h"
#include "TypeDefs.h"
// PAMI typedefs to avoid including pgas headers
typedef unsigned char*      xlpgas_local_addr_t;
typedef void *              user_func_t;
typedef unsigned            xlpgas_AMHeaderReg_t;
typedef pami_event_function xlpgas_event_function;
struct xlpgas_AMHeader_t
{
  xlpgas_AMHeaderReg_t  handler;    /**< header handler registration */
  unsigned              headerlen;  /**< total bytes in header       */
};
typedef pami_op                            xlpgas_ops_t;
typedef pami_dt                            xlpgas_dtypes_t;
typedef pami_endpoint_t                    xlpgas_endpoint_t;
typedef pami_event_function                xlpgas_LCompHandler_t;
//typedef pami_dispatch_ambroadcast_function PAMIAMHeader_bcast;
 typedef struct AMHeader_bcast
 {
   xlpgas_AMHeader_t   hdr;
   int                 senderID;
   int                 root;
   int                 dest_ctxt;
   int                 team_id;
   void*               parent_state;
   int                 len;
   xlpgas_AMHeaderReg_t user_cb;
 } AMHeader_bcast;
typedef struct AMHeader_reduce
{
  xlpgas_AMHeader_t   hdr;
  void*               parent_state;
  int                 senderID;
  int                 root;
  int                 dest_ctxt;
  int                 team_id;
  int                 len;
  xlpgas_dtypes_t     dt;
  xlpgas_ops_t        op;
  xlpgas_AMHeaderReg_t user_cb;
} AMHeader_reduce;

 typedef struct AMHeader_gather
 {
   xlpgas_AMHeader_t   hdr;
   void*               parent_state;
   int                 senderID;
   int                 root;
   int                 dest_ctxt;
   int                 team_id;
   int                 len;
   int                 rlen;
   xlpgas_AMHeaderReg_t user_cb;
 } AMHeader_gather;

#define XLPGAS_AMSEND_BARRIER       -1
#define XLPGAS_AMSEND_COLLEXCHANGE  -2
#define XLPGAS_TSP_AMSEND_COLLA2A   -3
#define XLPGAS_TSP_AMSEND_S         -4
#define XLPGAS_TSP_AMSEND_G         -5
#define XLPGAS_TSP_AMSEND_PERM      -6
#define XLPGAS_TSP_AMSEND_PSUM      -7
#define XLPGAS_TSP_AMSEND_COLLA2AV  -8
//for am collectives
#define XLPGAS_TSP_AMBCAST_PREQ       -9  // One sided collectives; parent request
#define XLPGAS_TSP_AMREDUCE_PREQ      -10
#define XLPGAS_TSP_AMREDUCE_CREQ      -11 // Child request
#define XLPGAS_TSP_AMGATHER_PREQ      -12
#define XLPGAS_TSP_AMGATHER_CREQ      -13 // Child request
#define XLPGAS_TSP_BCAST_TREE  -14

#define XLPGAS_MYNODE -1

static inline void xlpgas_fatalerror (int errcode, const char * strg, ...)
{
  char buffer[1024];
  va_list ap;
  va_start(ap, strg);
  vsnprintf (buffer, sizeof(buffer)-1, strg, ap);
  va_end(ap);
  if (errcode==0) errcode=-1;
  fprintf(stderr, "Collectives Runtime Error %d: %s\n",errcode,buffer);
  _exit(1);
}


namespace xlpgas
{
//  class Team;
  typedef PAMI_GEOMETRY_CLASS Team;

  /* *********************************************************** */
  /* *********************************************************** */

  enum CollectiveKind
    {
      BarrierKind=0,
      BarrierKind2,
      AllgatherKind,
      AllgathervKind,
      ScatterKind,
      ScattervKind,
      GatherKind,
      GathervKind,
      BcastKind,
      BcastKind2,
      BcastTreeKind,
      ShortAllreduceKind,
      LongAllreduceKind,
      AllreduceKind2,
      AlltoallKind,
      AlltoallvKind,
      PermuteKind,
      PrefixKind,
      AllreducePPKind,//from here down we have point to point versions
      BarrierPPKind,
      BcastPPKind,
      AllgatherPPKind,
      AlltoallPPKind,
      MAXKIND
    };

  /* *********************************************************** */
  /*        a generic non-blocking transport collective          */
  /* *********************************************************** */
  template <class T_NI>
  class Collective
    {
    public:
      static void     Initialize();

    public:
      Collective (int                      ctxt,
		  Team           * comm,
		  CollectiveKind           kind,
		  int                      tag,
		  xlpgas_LCompHandler_t     cb_complete,
		  void                   * arg);

    public:
      int            tag  () const { return _tag; }
      int            kind    () const { return _kind; }
      Team * comm    () const { return _comm; }
      void setComplete (xlpgas_LCompHandler_t cb,
			void *arg) { _cb_complete = cb; _arg = arg; }
      void setContext (pami_context_t ctxt) {_pami_ctxt=ctxt;}

    public:
      virtual void   kick   () { } /* force progress */
      virtual bool   isdone () const { return true; } /* check completion */

      virtual void   reset () { printf("ERROR: virtual method called (1)\n");}
      virtual void   reset (int root, const void * sbuf, void * rbuf, unsigned nbytes) {
	printf("ERROR: virtual method called (2)\n");
      }
      virtual void   reset (const void * sbuf, void * rbuf, unsigned nbytes) {
	printf("ERROR: virtual method called (3) \n");
      }
      virtual void reset (const void *sbuf, void * rbuf,
			  xlpgas_ops_t op, xlpgas_dtypes_t dtype,
			  unsigned nelems) {
	printf("ERROR: virtual method called (5)\n");
      }
      virtual void reset (const void *sbuf, void * rbuf,
			  xlpgas_ops_t op, xlpgas_dtypes_t dtype,
			  unsigned nelems, user_func_t*) {
	printf("ERROR: virtual method called (6)\n");
      }
      virtual void reset (const void *sbuf, void * rbuf, size_t*) {
	printf("ERROR: virtual method called (7)\n");
      }
      virtual void reset (const void *sbuf, void * rbuf,
			  const size_t*, const size_t*, const size_t*, const size_t*) {
	printf("ERROR: virtual method called (8)\n");
      }
      virtual void setNI(T_NI *p2p_iface) { _p2p_iface = p2p_iface; }
    protected:
      int                       _ctxt;
      pami_context_t            _pami_ctxt;
      Team                     * _comm;
      CollectiveKind            _kind;
      int                       _tag;
      xlpgas_LCompHandler_t      _cb_complete;
      void                    * _arg;
      T_NI                    * _p2p_iface;
      DECL_MUTEX(_mutex);
    };

  /* *********************************************************** */
  /*   Managing non-blocking collectives at runtime.             */
  /* The manager is a singleton.                                 */
  /* *********************************************************** */
  template <class T_NI>
  class CollectiveManager
    {
      typedef util::Array<Collective<T_NI> *> CollArray_t;


    public:
      /* ---------------- */
      /* external API     */
      /* ---------------- */

      static void Initialize (int ncontexts);
      static CollectiveManager * instance(int ctxt) { return _instances[ctxt];}

      Collective<T_NI> * find (CollectiveKind kind, int tag);

      template <class CollDefs>
      Collective<T_NI> * allocate (Team* comm, CollectiveKind kind, int id){
	assert (0 <= kind && kind < MAXKIND);
//	int nextID = _kindlist[kind]->len();
        int nextID = id;
	Collective<T_NI> * b;
	switch (kind)
	  {
	    /* Here I can make a templated allocator that is called
	       from each case statement with the proper type
	    */
	  case BarrierKind:
	    {
	      typedef typename CollDefs::barrier_type barrier_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(barrier_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(barrier_type));
	      new (b) barrier_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case BarrierPPKind:
	    {
	      typedef typename CollDefs::barrier_pp_type barrier_type;
	      b = (Collective<T_NI> *) __global.heap_mm->malloc (sizeof(barrier_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(barrier_type));
	      new (b) barrier_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case LongAllreduceKind:
	    {
	      typedef typename CollDefs::allreduce_type allreduce_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(allreduce_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(allreduce_type));
	      new (b) allreduce_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case ShortAllreduceKind:
	    {
	      typedef typename CollDefs::short_allreduce_type allreduce_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(allreduce_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(allreduce_type));
	      new (b) allreduce_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case AllreducePPKind:
	    {
	      typedef typename CollDefs::allreduce_pp_type allreduce_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(allreduce_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(allreduce_type));
	      new (b) allreduce_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case BcastKind:
	    {
	      typedef typename CollDefs::broadcast_type bcast_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(bcast_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(bcast_type));
	      new (b) bcast_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case BcastPPKind:
	    {
	      typedef typename CollDefs::broadcast_pp_type bcast_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(bcast_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(bcast_type));
	      new (b) bcast_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case BcastTreeKind:
	    {
	      typedef typename CollDefs::broadcast_tree_type bcast_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(bcast_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(bcast_type));
	      new (b) bcast_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case AllgatherKind:
	    {
	      typedef typename CollDefs::allgather_type allgather_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(allgather_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(allgather_type));
	      new (b) allgather_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }
	  case AllgatherPPKind:
	    {
	      typedef typename CollDefs::allgather_pp_type allgather_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(allgather_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(allgather_type));
	      new (b) allgather_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case AllgathervKind:
	    {
	      typedef typename CollDefs::allgatherv_type allgatherv_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(allgatherv_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(allgatherv_type));
	      new (b) allgatherv_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }
	  case AlltoallKind:
	    {
	      typedef typename CollDefs::alltoall_type alltoall_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(alltoall_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(alltoall_type));
	      new (b) alltoall_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }
	  case AlltoallPPKind:
	    {
	      typedef typename CollDefs::alltoall_pp_type alltoall_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(alltoall_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(alltoall_type));
	      new (b) alltoall_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }
	  case AlltoallvKind:
	    {
	      typedef typename CollDefs::alltoallv_type alltoallv_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(alltoallv_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(alltoallv_type));
	      new (b) alltoallv_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case GatherKind:
	    {
	      typedef typename CollDefs::gather_type gather_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(gather_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(gather_type));
	      new (b) gather_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case ScatterKind:
	    {
	      typedef typename CollDefs::scatter_type scatter_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(scatter_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(scatter_type));
	      new (b) scatter_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case PermuteKind:
	    {
	      typedef typename CollDefs::permute_type permute_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(permute_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(permute_type));
	      new (b) permute_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  case PrefixKind:
	    {
	      typedef typename CollDefs::prefixsums_type prefixsums_type;
	      b = (Collective<T_NI> *)__global.heap_mm->malloc (sizeof(prefixsums_type));
	      assert (b != NULL);
	      memset (b, 0, sizeof(prefixsums_type));
	      new (b) prefixsums_type (_ctxt, comm, kind, nextID, 0);
	      break;
	    }

	  default:
	    {
	      // xlpgas_fatalerror (-1, "Internal: invalid collective instance");
	      return NULL;
	    }
	  }
	(*_kindlist[kind])[nextID] = b;
	return b;
      }

      template <class CollDefs>
      inline void          multisend_reg(CollectiveKind kind, T_NI *p2p_iface)
        {
          switch (kind)
            {
	  case BarrierKind:
	    {
              typedef typename CollDefs::barrier_type barrier_type;
              p2p_iface->setSendDispatch(barrier_type::cb_incoming, this);
	      break;
	    }

	  case BarrierPPKind:
	    {
	      break;
	    }

	  case LongAllreduceKind:
	    {
              typedef typename CollDefs::allreduce_type allreduce_type;
              p2p_iface->setSendDispatch(allreduce_type::cb_incoming, this);
	      break;
	    }

	  case ShortAllreduceKind:
	    {
              typedef typename CollDefs::short_allreduce_type short_allreduce_type;
              p2p_iface->setSendDispatch(short_allreduce_type::cb_incoming, this);
	      break;
	    }

	  case AllreducePPKind:
	    {
	      break;
	    }

	  case BcastKind:
	    {
              typedef typename CollDefs::broadcast_type broadcast_type;
              p2p_iface->setSendDispatch(broadcast_type::cb_incoming, this);
	      break;
	    }

	  case BcastPPKind:
	    {
	      break;
	    }

	  case BcastTreeKind:
	    {
	      break;
	    }

	  case AllgatherKind:
	    {
              typedef typename CollDefs::allgather_type allgather_type;
              p2p_iface->setSendDispatch(allgather_type::cb_incoming, this);
	      break;
	    }
	  case AllgatherPPKind:
	    {
	      break;
	    }

	  case AllgathervKind:
	    {
              typedef typename CollDefs::allgatherv_type allgatherv_type;
              p2p_iface->setSendDispatch(allgatherv_type::cb_incoming, this);
	      break;
	    }
	  case AlltoallKind:
	    {
              typedef typename CollDefs::alltoall_type alltoall_type;
              p2p_iface->setSendDispatch(alltoall_type::cb_incoming, this);
	      break;
	    }
	  case AlltoallPPKind:
	    {
	      break;
	    }
	  case AlltoallvKind:
	    {
              typedef typename CollDefs::alltoallv_type alltoallv_type;
              p2p_iface->setSendDispatch(alltoallv_type::cb_incoming_v, this);
	      break;
	    }

	  case GatherKind:
	    {
              typedef typename CollDefs::gather_type gather_type;
              p2p_iface->setSendDispatch(gather_type::cb_incoming, this);
	      break;
	    }

	  case ScatterKind:
	    {
              typedef typename CollDefs::scatter_type scatter_type;
              p2p_iface->setSendDispatch(scatter_type::cb_incoming, this);
	      break;
	    }

	  case PermuteKind:
	    {
	      break;
	    }

	  case PrefixKind:
	    {
              typedef typename CollDefs::prefixsums_type prefixsums_type;
              p2p_iface->setSendDispatch(prefixsums_type::cb_incoming, this);
	      break;
	    }

	  default:
	    {
	      xlpgas_fatalerror (-1, "Internal: invalid collective registration");
	    }
	  }
        }
      void setGenericDevice(PAMI::Device::Generic::Device *g)
        {
          _genericDevice = g;
        }
    private:
      /* ------------ */
      /* data members */
      /* ------------ */

      CollArray_t              * _kindlist[MAXKIND];
      int                        _ctxt;
      static CollectiveManager ** _instances;
      PAMI::Device::Generic::Device *_genericDevice;
    private:
      /* ------------ */
      /* constructors */
      /* ------------ */
    public:
      CollectiveManager (int ctxt);
      void * operator new (size_t, void * addr) { return addr; }
    };
}

#include "algorithms/protocols/tspcoll/Collective.cc"
#include "algorithms/protocols/tspcoll/CollectiveManager.cc"
#endif
