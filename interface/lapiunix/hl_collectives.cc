#include "../hl_collectives.h"
#include "../include/lapi.h"
#include "../../p2p/common/include/pgasrt.h"
#include "../../p2p/tspcoll/Communicator.h"
#include <stdio.h>

// Includes for CCMI
#include "../../ccmi/adaptor/ccmi_internal.h"
#include "../../ccmi/adaptor/ccmi_util.h"
#include "../../ccmi/adaptor/pgasp2p/multisend/multisend_impl.h"
#include "../../ccmi/adaptor/geometry/Geometry.h"
#include "../../ccmi/logging/LogMgr.h"
#include "../../ccmi/adaptor/ccmi_debug.h"
#include "../../ccmi/adaptor/protocols/barrier/barrier_impl.h"
#include "../../ccmi/adaptor/pgasp2p/api/mapping_impl.h" // ? why

#include <unistd.h>

#define USE_CCMI
using namespace std;

#define MAX_REGISTRATIONS_PER_TABLE 16

extern CCMI::Adaptor::Adaptor  * _g_generic_adaptor;
extern CCMI::Logging::LogMgr   * CCMI::Logging::LogMgr::_staticLogMgr;

extern "C"
{
    int LL_to_PGAS_op[] =
	{-1,              // LL_UNDEFINED_OP
	 -1,              // LL_NOOP
	 PGASRT_OP_MAX,   // LL_MAX,
	 PGASRT_OP_MIN,   // LL_MIN,
	 PGASRT_OP_ADD,   // LL_SUM,
	 PGASRT_OP_MUL,   // LL_PROD,
	 -1,              // LL_LAND,
	 -1,              // LL_LOR,
	 -1,              // LL_LXOR,
	 PGASRT_OP_AND,   // LL_BAND,
	 PGASRT_OP_OR,    // LL_BOR,
	 PGASRT_OP_XOR,   // LL_BXOR,
	 -1,              // LL_MAXLOC,
	 -1,              // LL_MINLOC,
	 -1,              // LL_USERDEFINED_OP,
	 -1              // LL_OP_COUNT
	};
    int LL_to_PGAS_dt[] =
	{
	 /* Standard/Primative DT's */
	 -1,                   // LL_UNDEFINED_DT = 0,
	 PGASRT_DT_chr,        // LL_SIGNED_CHAR,
	 PGASRT_DT_byte,       // LL_UNSIGNED_CHAR,
	 PGASRT_DT_srt,        // LL_SIGNED_SHORT,
	 PGASRT_DT_hwrd,       // LL_UNSIGNED_SHORT,
	 PGASRT_DT_int,        // LL_SIGNED_INT,
	 PGASRT_DT_word,       // LL_UNSIGNED_INT,
	 PGASRT_DT_llg,        // LL_SIGNED_LONG_LONG,
	 PGASRT_DT_dwrd,       // LL_UNSIGNED_LONG_LONG,
	 PGASRT_DT_flt,        // LL_FLOAT,
	 PGASRT_DT_dbl,        // LL_DOUBLE,
	 -1,                   // LL_LONG_DOUBLE,
	 -1,                   // LL_LOGICAL,
	 -1,                   // LL_SINGLE_COMPLEX,
	 -1,                   // LL_DOUBLE_COMPLEX,
	 /* Max/Minloc DT's */
	 -1,                   // LL_LOC_2INT,
	 -1,                   // LL_LOC_SHORT_INT,
	 -1,                   // LL_LOC_FLOAT_INT,
	 -1,                   // LL_LOC_DOUBLE_INT,
	 -1,                   // LL_LOC_2FLOAT,
	 -1,                   // LL_LOC_2DOUBLE,
	 -1,                   // LL_USERDEFINED_DT,
	 -1,                   // LL_DT_COUNT
	};


    typedef struct
    {
	int size;
	HL_CollectiveProtocol_t *entries[MAX_REGISTRATIONS_PER_TABLE];
    }HL_proto_table;

    CCMI_Geometry_t      *getGeometry (int comm);

    HL_Geometry_t         HL_World_Geometry;
    HL_Geometry_range_t   world_range;
    HL_mapIdToGeometry    cb_geometry_map;


    int HL_Collectives_initialize(int argc, 
				  char*argv[], 
				  HL_mapIdToGeometry cb_map)
    {
	// Set up pgasrt P2P Collectives
	__pgasrt_tsp_setup         (1, &argc, &argv);

	cb_geometry_map = cb_map;

	// Set up CCMI collectives
	if(_g_generic_adaptor == NULL)
	    {
		void * buf = CCMI_Alloc(sizeof (CCMI::Adaptor::Adaptor));
		if(buf)
		    _g_generic_adaptor = new (buf) CCMI::Adaptor::Adaptor();
		int size = _g_generic_adaptor->mapping()->size();
		int nbarrier = 16;
	    }
	CCMI::Logging::LogMgr::setLogMgr (_g_generic_adaptor->getLogMgr ());

	world_range.lo       = 0;
	world_range.hi       = HL_Size()-1;
	HL_Geometry_initialize(&HL_World_Geometry,
			       HL_World_Geometry_id,
			       &world_range,
			       1);
	__pgasrt_tsp_barrier       ();

	return HL_SUCCESS;
    }

    int HL_Poll()
    {
	_g_generic_adaptor->advance();
//	__pgasrt_tsp_wait(NULL);
	return 0;
    }

    int HL_Rank()
    {
	return __pgasrt_tsp_myID();
    }

    int HL_Size()
    {
	return __pgasrt_tsp_numnodes();
    }

    int HL_Collectives_finalize()
    {
	__pgasrt_tsp_finish();

//	FILE *fp = fopen ("log", "a");
//	_g_generic_adaptor->getLogMgr()->dumpTimers(fp, _g_generic_adaptor->mapping());
	free (_g_generic_adaptor);
//	fclose (fp);
	return 0;
    }


    int HL_register(HL_CollectiveProtocol_t      *registration,
                    HL_CollectiveConfiguration_t *config,
		    int                           key)
    {
	HL_Result                                status;
	switch(config->cfg_type)
	    {
	    case HL_CFG_BROADCAST:
		{
		    HL_Broadcast_Configuration_t *cfg    = (HL_Broadcast_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_BROADCAST_PROTOCOL:
			    return HL_SUCCESS;
			    break;
			default:
			    return HL_UNIMPL;
			    break;
			}
		}
		break;
	    case HL_CFG_ALLGATHER:
		{
		    HL_Allgather_Configuration_t *cfg    = (HL_Allgather_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_ALLGATHER_PROTOCOL:
			    return HL_SUCCESS;
			    break;
			default:
			    return HL_UNIMPL;
			    break;
			}
		}
		break;
	    case HL_CFG_ALLGATHERV:
		{
		    HL_Allgatherv_Configuration_t *cfg    = (HL_Allgatherv_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_ALLGATHERV_PROTOCOL:
			    return HL_SUCCESS;
			    break;
			default:
			    return HL_UNIMPL;
			    break;
			}
		}
		break;
	    case HL_CFG_SCATTER:
		{
		    HL_Scatter_Configuration_t *cfg    = (HL_Scatter_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_SCATTER_PROTOCOL:
			    return HL_SUCCESS;
			    break;
			default:
			    return HL_UNIMPL;
			    break;
			}
		}
		break;
	    case HL_CFG_SCATTERV:
		{
		    HL_Scatterv_Configuration_t *cfg    = (HL_Scatterv_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_SCATTERV_PROTOCOL:
			    return HL_SUCCESS;
			    break;
			default:
			    return HL_UNIMPL;
			    break;
			}
		}
		break;
	    case HL_CFG_ALLREDUCE:
		{
		    HL_Allreduce_Configuration_t *cfg    = (HL_Allreduce_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_ALLREDUCE_PROTOCOL:
			    return HL_SUCCESS;
			    break;
			default:
			    return HL_UNIMPL;
			    break;
			}
		}
		break;
	    case HL_CFG_ALLTOALLV:
		{
		    HL_Alltoallv_Configuration_t *cfg   = (HL_Alltoallv_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_ALLTOALLV_PROTOCOL:
			    return HL_SUCCESS;
			    break;
			default:
			    return HL_UNIMPL;
			    break;
			}
		}
		break;
	    case HL_CFG_BARRIER:
		{
		    HL_Barrier_Configuration_t *cfg = (HL_Barrier_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_BARRIER_PROTOCOL:
			    return HL_SUCCESS;
			    break;
			default:
			    return HL_UNIMPL;
			    break;
			}
		}
		break;
	    default:
		return HL_UNIMPL;
		break;
	    }
	return HL_SUCCESS;
    }


    class geometry_internal
    {
    public:
	geometry_internal(int                  my_rank,
			  unsigned             slice_count,
			  HL_Geometry_range_t *rank_slices,
			  int                  nranks,
			  unsigned            *ranks,
			  unsigned             id,
			  CCMI_mapIdToGeometry cb_geometry):
	    _ccmi_geometry(_g_generic_adaptor->mapping(),
			   ranks, nranks,id,0),
	    _minfo(),
	    _barrier_factory(static_cast<CCMI::MultiSend::MulticastInterface *>(&_minfo),
			     _g_generic_adaptor->mapping(),
			     cb_geometry),
	    _pgasrt_comm(my_rank, (int)slice_count,(TSPColl::Range*)rank_slices),
	    _ranklist(ranks)

	{
	    _minfo.initialize (_g_generic_adaptor);
	    CCMI::Executor::Executor *exe = NULL;
	    exe = _barrier_factory.generate(&_barrier_executors[0], &_ccmi_geometry);
	    _ccmi_geometry.setBarrierExecutor(exe);
	    exe = _barrier_factory.generate(&_barrier_executors[1], &_ccmi_geometry);
	    _ccmi_geometry.setLocalBarrierExecutor(exe);
	    _pgasrt_comm.setup();
	}
	CCMI::Adaptor::Geometry                              _ccmi_geometry;
	CCMI_Executor_t                                      _barrier_executors[2];
	CCMI::Adaptor::Generic::MulticastImpl::MulticastImpl _minfo;
	CCMI::Adaptor::Barrier::BinomialBarrierFactory       _barrier_factory;
	TSPColl::RangedComm                                  _pgasrt_comm;
	unsigned                                            *_ranklist;
    };

    CCMI_Geometry_t *getGeometry (int comm) 
    {
#if 0
	void              *g_ptr = &HL_World_Geometry;
	geometry_internal *ptr   = (geometry_internal *)g_ptr;
	return (CCMI_Geometry_t *)&ptr->_ccmi_geometry;
#endif
	// This is OK because _ccmi_geometry is the first data item in the class
	// If both pgasrt and ccmi are delivering this callback, we 
	// need to implement the geometry lookup for the right class.
	return (CCMI_Geometry_t *)&HL_World_Geometry;
    }


    int HL_Geometry_initialize (HL_Geometry_t            * geometry,
				unsigned                   id,
				HL_Geometry_range_t      * rank_slices,
				unsigned                   slice_count)
    {
	unsigned i;
	// Set up a CCMI geometry
	// Step 1:  Build a rank list
	// \todo:  add a ranged communicator to CCMI
	int nranks = 0;
	for(i=0; i<slice_count; i++)
	    nranks+=rank_slices[i].hi-rank_slices[i].lo+1;
	unsigned int *ranks = (unsigned int *)malloc(nranks*sizeof(*ranks));

	int k = 0;
	for(i=0; i<slice_count; i++)
	    {
		int range = rank_slices[i].hi-rank_slices[i].lo+1;
		int j     = 0;
		for(j=0;j<range;j++,k++)
		    ranks[k] = rank_slices[i].lo + j;
	    }
	assert(sizeof(*geometry) >= sizeof(geometry_internal));
	new(geometry)geometry_internal(__pgasrt_tsp_myID(),
				       slice_count,
				       rank_slices,
				       nranks,
				       ranks,
				       id,
				       (CCMI_mapIdToGeometry)cb_geometry_map);
	return HL_SUCCESS;
    }


    int HL_Geometry_finalize(HL_Geometry_t *geometry)
    {
	geometry_internal *g = (geometry_internal*)geometry;
	free(g->_ranklist);
	return HL_SUCCESS;
    }


    // Currently all algorithms are implemented
    int HL_Geometry_algorithm (HL_Geometry_t            *geometry,
			       HL_CollectiveProtocol_t **protocols,
			       int                      *num)
    {
	return HL_SUCCESS;
    }



    int HL_Xfer (void* context, hl_xfer_t *cmd)
    {
	switch(cmd->xfer_type)
	    {
	    case HL_XFER_BROADCAST:
		{
		    hl_broadcast_t        * parms   = &cmd->xfer_broadcast;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    int p_root                      = tspcoll->virtrankof(parms->root);
		    tspcoll->ibcast(p_root, parms->src, parms->dst, parms->bytes,
				    (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_ALLGATHER:
		{
		    hl_allgather_t        * parms   = &cmd->xfer_allgather;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->iallgather(parms->src, parms->dst, parms->bytes,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_ALLGATHERV:
		{
		    hl_allgatherv_t        * parms   = &cmd->xfer_allgatherv;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->iallgatherv(parms->src, parms->dst, parms->lengths,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_SCATTER:
		{
		    hl_scatter_t          * parms   = &cmd->xfer_scatter;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    int p_root = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatter(p_root,parms->src, parms->dst, parms->bytes,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_SCATTERV:
		{
		    hl_scatterv_t         * parms   = &cmd->xfer_scatterv;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    int p_root = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatterv(p_root, parms->src, parms->dst, parms->lengths,
				       (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_ALLREDUCE:
		{
		    hl_allreduce_t        * parms   = &cmd->xfer_allreduce;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->iallreduce(parms->src,           // source buffer
					parms->dst,           // dst buffer
					(__pgasrt_ops_t)LL_to_PGAS_op[parms->op], // op
					(__pgasrt_dtypes_t)LL_to_PGAS_dt[parms->dt], // dt
					parms->count,             // type
					(void (*)(void*))parms->cb_done.function,
					parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_ALLTOALLV:
		{
		    hl_alltoall_t         * parms   = &cmd->xfer_alltoall;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		}
		break;
	    case HL_XFER_BARRIER:
		{
		    hl_barrier_t          * parms   = &cmd->xfer_barrier;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
#ifdef USE_CCMI
		    CCMI::Adaptor::Geometry   *_c_geometry    = (CCMI::Adaptor::Geometry *)&g->_ccmi_geometry;
		    CCMI::Executor::Executor  *_c_bar         = _c_geometry->getBarrierExecutor();
		    _c_bar->setDoneCallback    ((void (*)(void*, CCMI_Error_t*))parms->cb_done.function, parms->cb_done.clientdata);
		    _c_bar->setConsistency ((CCMI_Consistency) 0);
		    _c_bar->start();
		    return HL_SUCCESS;
#else
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->ibarrier((void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
#endif
		    return HL_SUCCESS;
		}
		break;
	    default:
		return HL_UNIMPL;
		break;
	    }
	return HL_SUCCESS;
    }
};
