/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#include "../hl_collectives.h"
#include <stdio.h>

// Includes for CCMI
#include "interface/ccmi_internal.h"
#include "util/ccmi_util.h"
#include "interface/mpi/multisend/multisend_impl.h"
#include "interface/Geometry.h"
#include "util/logging/LogMgr.h"
#include "util/ccmi_debug.h"
#include "algorithms/protocols/barrier/impl.h"
#include "algorithms/protocols/allreduce/sync_impl.h"
#include "algorithms/protocols/allreduce/async_impl.h"
//#include "interface/mpi/api/mapping_impl.h" should be in Mapping.cc, not here
#include "interface/mpi/GenericComm.h"
#include "algorithms/protocols/tspcoll/NBColl.h"
#include "algorithms/protocols/broadcast/async_impl.h"
#include "algorithms/protocols/alltoall/Alltoall.h"

#include <unistd.h>

#define USE_CCMI
using namespace std;

#define MAX_REGISTRATIONS_PER_TABLE 16

CCMI::Adaptor::Adaptor  * _g_generic_adaptor;
CCMI::Logging::LogMgr   * CCMI::Logging::LogMgr::_staticLogMgr;

extern "C"
{

    int LL_to_CCMI_op[] =
	{CM_UNDEFINED_OP,        // LL_UNDEFINED_OP
	 CM_NOOP,                // LL_NOOP
	 CM_MAX,                 // LL_MAX,
	 CM_MIN,                 // LL_MIN,
	 CM_SUM,                 // LL_SUM,
	 CM_PROD,                // LL_PROD,
	 CM_LAND,                // LL_LAND,
	 CM_LOR,                 // LL_LOR,
	 CM_LXOR,                // LL_LXOR,
	 CM_BAND,                // LL_BAND,
	 CM_BOR,                 // LL_BOR,
	 CM_BXOR,                // LL_BXOR,
	 CM_MAXLOC,              // LL_MAXLOC,
	 CM_MINLOC,              // LL_MINLOC,
	 CM_USERDEFINED_OP,      // LL_USERDEFINED_OP,
	 CM_OP_COUNT             // LL_OP_COUNT
	};
    int LL_to_CCMI_dt[] =
	{
	 /* Standard/Primative DT's */
	 CM_UNDEFINED_DT,        // LL_UNDEFINED_DT = 0,
	 CM_SIGNED_CHAR,         // LL_SIGNED_CHAR,
	 CM_UNSIGNED_CHAR,       // LL_UNSIGNED_CHAR,
	 CM_SIGNED_SHORT,        // LL_SIGNED_SHORT,
	 CM_UNSIGNED_SHORT,      // LL_UNSIGNED_SHORT,
	 CM_SIGNED_INT,          // LL_SIGNED_INT,
	 CM_UNSIGNED_INT,        // LL_UNSIGNED_INT,
	 CM_SIGNED_LONG_LONG,    // LL_SIGNED_LONG_LONG,
	 CM_UNSIGNED_LONG_LONG,  // LL_UNSIGNED_LONG_LONG,
	 CM_FLOAT,               // LL_FLOAT,
	 CM_DOUBLE,              // LL_DOUBLE,
	 CM_LONG_DOUBLE,         // LL_LONG_DOUBLE,
	 CM_LOGICAL,             // LL_LOGICAL,
	 CM_SINGLE_COMPLEX,      // LL_SINGLE_COMPLEX,
	 CM_DOUBLE_COMPLEX,      // LL_DOUBLE_COMPLEX,
	 /* Max/Minloc DT's */
	 CM_LOC_2INT,            // LL_LOC_2INT,
	 CM_LOC_SHORT_INT,       // LL_LOC_SHORT_INT,
	 CM_LOC_FLOAT_INT,       // LL_LOC_FLOAT_INT,
	 CM_LOC_DOUBLE_INT,      // LL_LOC_DOUBLE_INT,
	 CM_LOC_2FLOAT,          // LL_LOC_2FLOAT,
	 CM_LOC_2DOUBLE,         // LL_LOC_2DOUBLE,
	 CM_USERDEFINED_DT,      // LL_USERDEFINED_DT,
	 -1
	};


    typedef struct
    {
	int size;
	CM_CollectiveProtocol_t *entries[MAX_REGISTRATIONS_PER_TABLE];
    }HL_proto_table;

    CCMI_Geometry_t      *getGeometry (int comm);

    HL_Geometry_t         HL_World_Geometry;
    unsigned              HL_World_Geometry_id=0;
    HL_Geometry_range_t   world_range;
    HL_mapIdToGeometry    cb_geometry_map;


    int HL_Collectives_initialize(int *argc,
				  char***argv,
				  HL_mapIdToGeometry cb_map)
    {
      MPI_Init(argc, argv);
      // Set up pgasrt P2P Collectives
      //	__pgasrt_tsp_setup         (1, argc, argv);
      
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
	//	TSPColl::NBCollManager::instance();
	HL_Geometry_initialize(&HL_World_Geometry,
			       HL_World_Geometry_id,
			       &world_range,
			       1);
	//	__pgasrt_tsp_barrier       ();
	MPI_Barrier(MPI_COMM_WORLD);
	return CM_SUCCESS;
    }

    int HL_Poll()
    {
	_g_generic_adaptor->advance();
//	__pgasrt_tsp_wait(NULL);
	return 0;
    }

    int HL_Rank()
    {
	return _g_generic_adaptor->mapping()->rank();
    }

    int HL_Size()
    {
	return _g_generic_adaptor->mapping()->size();
    }

    int HL_Collectives_finalize()
    {
      //	__pgasrt_tsp_finish();

//	FILE *fp = fopen ("log", "a");
//	_g_generic_adaptor->getLogMgr()->dumpTimers(fp, _g_generic_adaptor->mapping());
	free (_g_generic_adaptor);
	MPI_Finalize();
//	fclose (fp);
	return 0;
    }


    int HL_register(CM_CollectiveProtocol_t      *registration,
                    HL_CollectiveConfiguration_t *config,
		    int                           key)
    {
	CM_Result                                status;
	switch(config->cfg_type)
	    {
	    case HL_CFG_BROADCAST:
		{
		    HL_Broadcast_Configuration_t *cfg    = (HL_Broadcast_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_BROADCAST_PROTOCOL:
			  {
			    CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
			    COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BcastTag, minfo);		    
			    return CM_SUCCESS;
			    break;
			  }
			default:
			    return CM_UNIMPL;
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
			  {
			    CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::AllgatherTag, minfo);			    
			    return CM_SUCCESS;
			    break;
			  }
			default:
			    return CM_UNIMPL;
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
			  {
			    CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
			    COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::AllgathervTag, minfo);			    
			    return CM_SUCCESS;
			    break;
			  }
			default:
			    return CM_UNIMPL;
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
			  {
			    typedef struct scatter_info
			    {
			      CCMI::Adaptor::Generic::OldMulticastImpl barrier;
			      CCMI::Adaptor::Generic::OldMulticastImpl scatter;
			    }scatter_info;
			    COMPILE_TIME_ASSERT(sizeof(scatter_info) < sizeof (*registration));
			    scatter_info *scinfo = (scatter_info *) registration;
			    new(&scinfo->barrier) CCMI::Adaptor::Generic::OldMulticastImpl();
			    new(&scinfo->scatter) CCMI::Adaptor::Generic::OldMulticastImpl();
			    scinfo->barrier.initialize(_g_generic_adaptor);
			    scinfo->scatter.initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BarrierTag, &scinfo->barrier);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::ScatterTag, &scinfo->scatter);
			    return CM_SUCCESS;
			    break;
			  }
			default:
			    return CM_UNIMPL;
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
			  {
			    typedef struct scatterv_info
			    {
			      CCMI::Adaptor::Generic::OldMulticastImpl barrier;
			      CCMI::Adaptor::Generic::OldMulticastImpl scatterv;
			    }scatterv_info;
			    COMPILE_TIME_ASSERT(sizeof(scatterv_info) < sizeof (*registration));
			    scatterv_info *scinfo = (scatterv_info *) registration;
			    new(&scinfo->barrier) CCMI::Adaptor::Generic::OldMulticastImpl();
			    new(&scinfo->scatterv) CCMI::Adaptor::Generic::OldMulticastImpl();
			    scinfo->barrier.initialize(_g_generic_adaptor);
			    scinfo->scatterv.initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BarrierTag, &scinfo->barrier);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::ScattervTag, &scinfo->scatterv);
			    return CM_SUCCESS;
			    break;
			  }
			default:
			    return CM_UNIMPL;
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
			    {
#ifdef USE_CCMI
				CCMI::Adaptor::ConfigFlags flags;
				flags.reuse_storage_limit = ((unsigned)2*1024*1024*1024 - 1);
				flags.pipeline_override   = 0;

				typedef struct
				{
				    CCMI::Adaptor::Allreduce::Binomial::Factory    allreduce_registration;
				    CCMI::Adaptor::Generic::OldMulticastImpl                         minfo;
				} SyncBinomialRegistration;

				CCMI_assert (sizeof (SyncBinomialRegistration) <=
					     sizeof (CM_CollectiveProtocol_t));

				SyncBinomialRegistration *treg =
				    (SyncBinomialRegistration *) registration;

				new (& treg->minfo)//, sizeof(treg->minfo))
				    CCMI::Adaptor::Generic::OldMulticastImpl();
				
				new (& treg->allreduce_registration)//, sizeof(treg->allreduce_registration))
				    CCMI::Adaptor::Allreduce::Binomial::Factory
				    (_g_generic_adaptor->mapping(), & treg->minfo, NULL, (CCMI_mapIdToGeometry)cb_geometry_map, flags);

				treg->minfo.initialize(_g_generic_adaptor);
#else
				CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
				  new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
				minfo->initialize(_g_generic_adaptor);
				TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::ShortAllreduceTag, minfo);
#endif

				return CM_SUCCESS;
			    }
			    break;
			default:
			    return CM_UNIMPL;
			    break;
			}
		}
		break;
#warning disabled HL_CFG_ALLTOALL until working
#if 0
	    case HL_CFG_ALLTOALL:
		{
		    HL_Alltoall_Configuration_t *cfg   = (HL_Alltoall_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_ALLTOALL_PROTOCOL:
			  {
			    typedef struct
			    {
			      CCMI::Adaptor::Alltoall::Factory       alltoall_registration;
			      CCMI::Adaptor::Generic::ManytomanyImpl minfo;
			    } AlltoallRegistration;
			    
			    CCMI_assert( sizeof(AlltoallRegistration) <=
					 sizeof(CM_CollectiveProtocol_t) );
			    
			    AlltoallRegistration * treg = 
			      (AlltoallRegistration *) registration;
			    
			    new ( & treg->minfo ) 
			      CCMI::Adaptor::Generic::ManytomanyImpl();
			    new ( & treg->alltoall_registration ) 
			      CCMI::Adaptor::Alltoall::Factory(& treg->minfo,
							       _g_generic_adaptor->mapping() );
			    
			    treg->minfo.initialize(_g_generic_adaptor);
			  
			    return CM_SUCCESS;
			  }
			  break;
			default:
			    return CM_UNIMPL;
			    break;
			}
		}
		break;
#endif

	    case HL_CFG_ALLTOALLV:
		{
		    HL_Alltoallv_Configuration_t *cfg   = (HL_Alltoallv_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_ALLTOALLV_PROTOCOL:
			    return CM_SUCCESS;
			    break;
			default:
			    return CM_UNIMPL;
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
			  {
			    CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
			    COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BarrierTag, minfo);		    
			    return CM_SUCCESS;
			    break;
			  }
			default:
			    return CM_UNIMPL;
			    break;
			}
		}
		break;
	    case HL_CFG_AMBROADCAST:
		{
		    HL_AMBroadcast_Configuration_t *cfg   = (HL_AMBroadcast_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_AMBROADCAST_PROTOCOL:
			  {
			    typedef struct
			    {
			      CCMI::Adaptor::Broadcast::AsyncBinomialFactory    bcast_registration;
			      CCMI::Adaptor::Generic::OldMulticastImpl             minfo;
			    } AsyncBinomialRegistration;
			    CCMI_assert (sizeof (AsyncBinomialRegistration) <=
					 sizeof (CM_CollectiveProtocol_t));
			    AsyncBinomialRegistration *treg =
			      (AsyncBinomialRegistration *) registration;
			    new (& treg->minfo) CCMI::Adaptor::Generic::OldMulticastImpl();
			    CCMI::Adaptor::Broadcast::AsyncBinomialFactory *factory =
			      new (& treg->bcast_registration)
			      CCMI::Adaptor::Broadcast::AsyncBinomialFactory
			      (_g_generic_adaptor->mapping(), & treg->minfo, _g_generic_adaptor->mapping()->size()); 
			    factory->setAsyncInfo(false,
						  (CCMI_RecvAsyncBroadcast)cfg->cb_recv,
						  (CCMI_mapIdToGeometry)cb_geometry_map);
			    treg->minfo.initialize (_g_generic_adaptor);
			    return CM_SUCCESS;
			    break;
			  }
			default:
			    return CM_UNIMPL;
			    break;
			}
		}
	    case HL_CFG_AMSCATTER:
		{
		    HL_AMScatter_Configuration_t *cfg   = (HL_AMScatter_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_AMSCATTER_PROTOCOL:
			    return CM_SUCCESS;
			    break;
			default:
			    return CM_UNIMPL;
			    break;
			}
		}
	    case HL_CFG_AMGATHER:
		{
		    HL_AMGather_Configuration_t *cfg   = (HL_AMGather_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_AMGATHER_PROTOCOL:
			    return CM_SUCCESS;
			    break;
			default:
			    return CM_UNIMPL;
			    break;
			}
		}
	    case HL_CFG_AMREDUCE:
		{
		    HL_AMReduce_Configuration_t *cfg   = (HL_AMReduce_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case HL_DEFAULT_AMREDUCE_PROTOCOL:
			    return CM_SUCCESS;
			    break;
			default:
			    return CM_UNIMPL;
			    break;
			}
		}

	    default:
		return CM_UNIMPL;
		break;
	    }
	return CM_SUCCESS;
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
			   ranks, (unsigned)nranks,id,0,false),
	    _minfo(),
	    _barrier_factory(static_cast<CCMI::MultiSend::OldMulticastInterface *>(&_minfo),
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
	CCMI::Adaptor::Generic::OldMulticastImpl             _minfo;
	CCMI::Adaptor::Barrier::BinomialBarrierFactory       _barrier_factory;
        TSPColl::RangedComm                                  _pgasrt_comm;
	unsigned                                            *_ranklist;
    };

    CCMI_Geometry_t *getGeometry (int comm)
    {
	void              *g_ptr = &HL_World_Geometry;
	geometry_internal *ptr   = (geometry_internal *)g_ptr;
	return (CCMI_Geometry_t *)&ptr->_ccmi_geometry;
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
	new(geometry)geometry_internal(_g_generic_adaptor->mapping()->rank(),
				       slice_count,
				       rank_slices,
				       nranks,
				       ranks,
				       id,
				       (CCMI_mapIdToGeometry)cb_geometry_map);
	return CM_SUCCESS;
    }


    int HL_Geometry_finalize(HL_Geometry_t *geometry)
    {
	geometry_internal *g = (geometry_internal*)geometry;
	free(g->_ranklist);
	return CM_SUCCESS;
    }


    // Currently all algorithms are implemented
    int HL_Geometry_algorithm (HL_Geometry_t            *geometry,
			       CM_CollectiveProtocol_t **protocols,
			       int                      *num)
    {
	return CM_SUCCESS;
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
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;
		    tspcoll->ibcast(minfo, p_root, parms->src, parms->dst, parms->bytes,
				    (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return CM_SUCCESS;
		}
		break;
	    case HL_XFER_ALLGATHER:
		{
		    hl_allgather_t        * parms   = &cmd->xfer_allgather;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;
		    tspcoll->iallgather(minfo,parms->src, parms->dst, parms->bytes,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return CM_SUCCESS;
		}
		break;
	    case HL_XFER_ALLGATHERV:
		{
		    hl_allgatherv_t        * parms   = &cmd->xfer_allgatherv;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;
		    tspcoll->iallgatherv(minfo,parms->src, parms->dst, parms->lengths,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return CM_SUCCESS;
		}
		break;
	    case HL_XFER_SCATTER:
		{
		    typedef struct scatter_info
		    {
		      CCMI::Adaptor::Generic::OldMulticastImpl barrier;
		      CCMI::Adaptor::Generic::OldMulticastImpl scatter;
		    }scatter_info;
		    hl_scatter_t          * parms   = &cmd->xfer_scatter;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    scatter_info          * scinfo  = (scatter_info*)parms->registration;
		    int                     p_root  = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatter(&scinfo->barrier, &scinfo->scatter,
				      p_root,parms->src, parms->dst, parms->bytes,
				      (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return CM_SUCCESS;
		}
		break;
	    case HL_XFER_SCATTERV:
		{
		    typedef struct scatterv_info
		    {
		      CCMI::Adaptor::Generic::OldMulticastImpl barrier;
		      CCMI::Adaptor::Generic::OldMulticastImpl scatterv;
		    }scatterv_info;
		    hl_scatterv_t         * parms   = &cmd->xfer_scatterv;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    scatterv_info         * scinfo  = (scatterv_info*)parms->registration;
		    int                     p_root  = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatterv(&scinfo->barrier, &scinfo->scatterv,
				       p_root, parms->src, parms->dst, parms->lengths,
				       (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return CM_SUCCESS;
		}
		break;
	    case HL_XFER_ALLREDUCE:
		{
		    hl_allreduce_t        * parms   = &cmd->xfer_allreduce;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
#ifdef USE_CCMI
		    {
			CCMI::Adaptor::Geometry   *_c_geometry =
			    (CCMI::Adaptor::Geometry *)
			    &g->_ccmi_geometry;

			CCMI::Adaptor::Allreduce::BaseComposite * allreduce =
			    (CCMI::Adaptor::Allreduce::BaseComposite *)
			    _c_geometry->getAllreduceComposite();

			CCMI::Adaptor::Allreduce::Ring::Factory *factory =
			    (CCMI::Adaptor::Allreduce::Ring::Factory *)
			    parms->registration;

			//Also check for change in protocols
			if(allreduce != NULL  &&  allreduce->getFactory() == factory)
			    {
				unsigned status =  allreduce->restart((CM_CollectiveRequest_t*)parms->request,
								      *(CM_Callback_t *)&parms->cb_done,
								      (CCMI_Consistency)0,
								      parms->src,
								      parms->dst,
								      parms->count,
								      (CM_Dt)LL_to_CCMI_dt[parms->dt],
								      (CM_Op)LL_to_CCMI_op[parms->op]);
				if(status == CM_SUCCESS)
				    {
					_c_geometry->setAllreduceComposite(allreduce);
					return status;
				    }
			    }

			if(allreduce != NULL) // Different factory?  Cleanup old executor.
			    {
				_c_geometry->setAllreduceComposite(NULL);
				allreduce->~BaseComposite();
			    }
			void *ptr =factory->generate((CM_CollectiveRequest_t*)parms->request,
						     *(CM_Callback_t *) &parms->cb_done,
						     (CCMI_Consistency)0,
						     _c_geometry,
						     parms->src,
						     parms->dst,
						     parms->count,
						     (CM_Dt)LL_to_CCMI_dt[parms->dt],
						     (CM_Op)LL_to_CCMI_op[parms->op]);
			if(ptr == NULL)
			    {
				return CM_UNIMPL;
			    }

			return CM_SUCCESS;
		    }
#else
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->iallreduce(minfo,
					parms->src,           // source buffer
					parms->dst,           // dst buffer
					(CM_Op)LL_to_CCMI_op[parms->op], // op
					(CM_Dt)LL_to_CCMI_dt[parms->dt], // dt
					parms->count,             // type
					(void (*)(void*))parms->cb_done.function,
					parms->cb_done.clientdata);
#endif
		    return CM_SUCCESS;
		}
		break;
	    case HL_XFER_ALLTOALLV:
		{
		    hl_alltoall_t         * parms   = &cmd->xfer_alltoall;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		}
		break;
#if 0
	    case HL_XFER_ALLTOALL:
		{
		    hl_alltoall_t         * parms   = &cmd->xfer_alltoall;

geometry_internal         * g       = (geometry_internal*)parms->geometry;
		CCMI::Adaptor::Geometry   *_c_geometry    = (CCMI::Adaptor::Geometry *)&g->_ccmi_geometry;

		    CCMI::Adaptor::Alltoall::Factory *factory =
		      (CCMI::Adaptor::Alltoall::Factory *)parms->registration;

		    CM_Callback_t cb_done_ccmi;
		    cb_done_ccmi.function   = (void (*)(void*, CM_Error_t*))parms->cb_done.function;
		    cb_done_ccmi.clientdata = parms->cb_done.clientdata;

		    factory->generate((CM_CollectiveRequest_t *)parms->request,
				      cb_done_ccmi,
				      (CCMI_Consistency)0,
				      _c_geometry,
				      parms->sndbuf,
				      parms->sndlens,
				      parms->sdispls,
				      parms->rcvbuf,
				      parms->rcvlens,
				      parms->rdispls,
				      parms->sndcounters,
				      parms->rcvcounters);
		}
		break;
#endif

	    case HL_XFER_BARRIER:
		{
		    hl_barrier_t          * parms   = &cmd->xfer_barrier;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
#ifdef USE_CCMI
		    CCMI::Adaptor::Geometry   *_c_geometry    = (CCMI::Adaptor::Geometry *)&g->_ccmi_geometry;
		    CCMI::Executor::Executor  *_c_bar         = _c_geometry->getBarrierExecutor();
		    _c_bar->setDoneCallback    ((void (*)(void*, CM_Error_t*))parms->cb_done.function, parms->cb_done.clientdata);
		    _c_bar->setConsistency ((CCMI_Consistency) 0);
		    _c_bar->start();
		    return CM_SUCCESS;
#else
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;		    
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->ibarrier(minfo,(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
#endif
		    return CM_SUCCESS;
		}
		break;
	    case HL_XFER_AMBROADCAST:
	      {
#ifdef USE_CCMI
		hl_ambroadcast_t          * parms   = &cmd->xfer_ambroadcast;
		geometry_internal         * g       = (geometry_internal*)parms->geometry;
		CCMI::Adaptor::Geometry   *_c_geometry    = (CCMI::Adaptor::Geometry *)&g->_ccmi_geometry;
		CCMI::Adaptor::Broadcast::BroadcastFactory<CCMI::CollectiveMapping> *factory =
		  (CCMI::Adaptor::Broadcast::BroadcastFactory<CCMI::CollectiveMapping> *) parms->registration;
		CCMI_assert (((CCMI::Adaptor::Geometry *) geometry)->getBarrierExecutor() != NULL);
		if(parms->bytes == 0)
		  {
		    parms->cb_done.function(parms->cb_done.clientdata, NULL);
		  }
		else
		  {
		    CM_Callback_t cb_done_ccmi;
		    cb_done_ccmi.function   = (void (*)(void*, CM_Error_t*))parms->cb_done.function;
		    cb_done_ccmi.clientdata = parms->cb_done.clientdata;
		    factory->generate(parms->request, 
				      sizeof(CM_CollectiveRequest_t), 
				      cb_done_ccmi,
				      (CCMI_Consistency)0,
				      _c_geometry,
				      _g_generic_adaptor->mapping()->rank(), //root
				      parms->src,
				      parms->bytes);
		  }

  return CM_SUCCESS;
		
#endif
		return CM_SUCCESS;
	      }
	      break;
	    case HL_XFER_AMSCATTER:
	      {
		return CM_SUCCESS;
	      }
	      break;
            case HL_XFER_AMGATHER:
	      {
		return CM_SUCCESS;
	      }
	      break;
            case HL_XFER_AMREDUCE:
	      {
		return CM_SUCCESS;
	      }
	      break;
	    default:
		return CM_UNIMPL;
		break;
	    }
	return CM_SUCCESS;
    }
};
