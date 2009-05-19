#include "../hl_collectives.h"
#include "../include/lapi.h"
#include "../../p2p/common/include/pgasrt.h"
#include "../../p2p/tspcoll/Communicator.h"
#include <stdio.h>

using namespace std;

#define MAX_REGISTRATIONS_PER_TABLE 16

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

    HL_Geometry_t         HL_World_Geometry;
    HL_Geometry_range_t   world_range;

    int HL_Collectives_initialize(int argc, char*argv[])
    {
	__pgasrt_tsp_setup         (1, &argc, &argv);


	world_range.lo       = 0;
	world_range.hi       = HL_Size()-1;
	HL_Geometry_initialize(&HL_World_Geometry, 
			       HL_World_Geometry_id,
			       &world_range,
			       1);
	__pgasrt_tsp_barrier       ();
	return 0;

    }

    int HL_Poll()
    {
	__pgasrt_tsp_wait(NULL);
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


    int HL_Geometry_initialize (HL_Geometry_t            * geometry,
				unsigned                   id,
				HL_Geometry_range_t      * rank_slices,
				unsigned                   slice_count)
    {
	TSPColl::Communicator * tspcoll  
	    = new (geometry) TSPColl::RangedComm(__pgasrt_tsp_myID(),
						 slice_count,
						 (TSPColl::Range*)rank_slices);
	tspcoll->setup();
	return HL_SUCCESS;
    }


    int HL_Geometry_finalize(HL_Geometry_t *geometry)
    {
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
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)parms->geometry;
		    int p_root                      = tspcoll->virtrankof(parms->root);
		    tspcoll->ibcast(p_root, parms->src, parms->dst, parms->bytes,
				    (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_ALLGATHER:
		{
		    hl_allgather_t        * parms   = &cmd->xfer_allgather;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)parms->geometry;
		    tspcoll->iallgather(parms->src, parms->dst, parms->bytes,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_ALLGATHERV:
		{
		    hl_allgatherv_t        * parms   = &cmd->xfer_allgatherv;
		    TSPColl::Communicator  * tspcoll = (TSPColl::Communicator *)parms->geometry;
		    tspcoll->iallgatherv(parms->src, parms->dst, parms->lengths,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_SCATTER:
		{
		    hl_scatter_t          * parms   = &cmd->xfer_scatter;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)parms->geometry;
		    int p_root = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatter(p_root,parms->src, parms->dst, parms->bytes,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_SCATTERV:
		{
		    hl_scatterv_t         * parms   = &cmd->xfer_scatterv;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)parms->geometry;
		    int p_root = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatterv(p_root, parms->src, parms->dst, parms->lengths,
				       (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_ALLREDUCE:
		{
		    hl_allreduce_t        * parms   = &cmd->xfer_allreduce;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)parms->geometry;
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
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)parms->geometry;
		}
		break;
	    case HL_XFER_BARRIER:
		{
		    hl_barrier_t          * parms   = &cmd->xfer_barrier;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)parms->geometry;
		    tspcoll->ibarrier((void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
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
