#include "../hl_collectives.h"
#include <stdio.h>

// Includes for CCMI
#include "collectives/interface/ccmi_internal.h"
#include "collectives/util/ccmi_util.h"
#include "collectives/interface/genericmpi/multisend/multisend_impl.h"
#include "collectives/interface/Geometry.h"
#include "collectives/util/logging/LogMgr.h"
#include "collectives/util/ccmi_debug.h"
#include "collectives/algorithms/protocols/barrier/barrier_impl.h"
#include "collectives/algorithms/protocols/allreduce/sync_impl.h"
#include "collectives/algorithms/protocols/allreduce/async_impl.h"
#include "collectives/interface/genericmpi/api/mapping_impl.h" // ? why
#include "collectives/interface/genericmpi/GenericComm.h"
#include "collectives/algorithms/protocols/tspcoll/NBColl.h"


#include <unistd.h>

//#define USE_CCMI
using namespace std;

#define MAX_REGISTRATIONS_PER_TABLE 16

CCMI::Adaptor::Adaptor  * _g_generic_adaptor;
CCMI::Logging::LogMgr   * CCMI::Logging::LogMgr::_staticLogMgr;

extern "C"
{
    int LL_to_CCMI_op[] =
	{CCMI_UNDEFINED_OP,        // LL_UNDEFINED_OP
	 CCMI_NOOP,                // LL_NOOP
	 CCMI_MAX,                 // LL_MAX,
	 CCMI_MIN,                 // LL_MIN,
	 CCMI_SUM,                 // LL_SUM,
	 CCMI_PROD,                // LL_PROD,
	 CCMI_LAND,                // LL_LAND,
	 CCMI_LOR,                 // LL_LOR,
	 CCMI_LXOR,                // LL_LXOR,
	 CCMI_BAND,                // LL_BAND,
	 CCMI_BOR,                 // LL_BOR,
	 CCMI_BXOR,                // LL_BXOR,
	 CCMI_MAXLOC,              // LL_MAXLOC,
	 CCMI_MINLOC,              // LL_MINLOC,
	 CCMI_USERDEFINED_OP,      // LL_USERDEFINED_OP,
	 CCMI_OP_COUNT             // LL_OP_COUNT
	};
    int LL_to_CCMI_dt[] =
	{
	 /* Standard/Primative DT's */
	 CCMI_UNDEFINED_DT,        // LL_UNDEFINED_DT = 0,
	 CCMI_SIGNED_CHAR,         // LL_SIGNED_CHAR,
	 CCMI_UNSIGNED_CHAR,       // LL_UNSIGNED_CHAR,
	 CCMI_SIGNED_SHORT,        // LL_SIGNED_SHORT,
	 CCMI_UNSIGNED_SHORT,      // LL_UNSIGNED_SHORT,
	 CCMI_SIGNED_INT,          // LL_SIGNED_INT,
	 CCMI_UNSIGNED_INT,        // LL_UNSIGNED_INT,
	 CCMI_SIGNED_LONG_LONG,    // LL_SIGNED_LONG_LONG,
	 CCMI_UNSIGNED_LONG_LONG,  // LL_UNSIGNED_LONG_LONG,
	 CCMI_FLOAT,               // LL_FLOAT,
	 CCMI_DOUBLE,              // LL_DOUBLE,
	 CCMI_LONG_DOUBLE,         // LL_LONG_DOUBLE,
	 CCMI_LOGICAL,             // LL_LOGICAL,
	 CCMI_SINGLE_COMPLEX,      // LL_SINGLE_COMPLEX,
	 CCMI_DOUBLE_COMPLEX,      // LL_DOUBLE_COMPLEX,
	 /* Max/Minloc DT's */
	 CCMI_LOC_2INT,            // LL_LOC_2INT,
	 CCMI_LOC_SHORT_INT,       // LL_LOC_SHORT_INT,
	 CCMI_LOC_FLOAT_INT,       // LL_LOC_FLOAT_INT,
	 CCMI_LOC_DOUBLE_INT,      // LL_LOC_DOUBLE_INT,
	 CCMI_LOC_2FLOAT,          // LL_LOC_2FLOAT,
	 CCMI_LOC_2DOUBLE,         // LL_LOC_2DOUBLE,
	 CCMI_USERDEFINED_DT,      // LL_USERDEFINED_DT,
	 -1
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
			  {
			    CCMI::Adaptor::Generic::MulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::MulticastImpl();
			    COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BcastTag, minfo);		    
			    return HL_SUCCESS;
			    break;
			  }
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
			  {
			    CCMI::Adaptor::Generic::MulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::MulticastImpl();
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::AllgatherTag, minfo);			    
			    return HL_SUCCESS;
			    break;
			  }
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
			  {
			    CCMI::Adaptor::Generic::MulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::MulticastImpl();
			    COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::AllgathervTag, minfo);			    
			    return HL_SUCCESS;
			    break;
			  }
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
			  {
			    typedef struct scatter_info
			    {
			      CCMI::Adaptor::Generic::MulticastImpl barrier;
			      CCMI::Adaptor::Generic::MulticastImpl scatter;
			    }scatter_info;
			    COMPILE_TIME_ASSERT(sizeof(scatter_info) < sizeof (*registration));
			    scatter_info *scinfo = (scatter_info *) registration;
			    new(&scinfo->barrier) CCMI::Adaptor::Generic::MulticastImpl();
			    new(&scinfo->scatter) CCMI::Adaptor::Generic::MulticastImpl();
			    scinfo->barrier.initialize(_g_generic_adaptor);
			    scinfo->scatter.initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BarrierTag, &scinfo->barrier);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::ScatterTag, &scinfo->scatter);
			    return HL_SUCCESS;
			    break;
			  }
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
			  {
			    typedef struct scatterv_info
			    {
			      CCMI::Adaptor::Generic::MulticastImpl barrier;
			      CCMI::Adaptor::Generic::MulticastImpl scatterv;
			    }scatterv_info;
			    COMPILE_TIME_ASSERT(sizeof(scatterv_info) < sizeof (*registration));
			    scatterv_info *scinfo = (scatterv_info *) registration;
			    new(&scinfo->barrier) CCMI::Adaptor::Generic::MulticastImpl();
			    new(&scinfo->scatterv) CCMI::Adaptor::Generic::MulticastImpl();
			    scinfo->barrier.initialize(_g_generic_adaptor);
			    scinfo->scatterv.initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BarrierTag, &scinfo->barrier);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::ScattervTag, &scinfo->scatterv);
			    return HL_SUCCESS;
			    break;
			  }
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
			    {
#ifdef USE_CCMI
				CCMI::Adaptor::ConfigFlags flags;
				flags.reuse_storage_limit = ((unsigned)2*1024*1024*1024 - 1);
				flags.pipeline_override   = 0;

				typedef struct
				{
				    CCMI::Adaptor::Allreduce::Binomial::Factory    allreduce_registration;
				    CCMI::Adaptor::Generic::MulticastImpl                         minfo;
				} SyncBinomialRegistration;

				CCMI_assert (sizeof (SyncBinomialRegistration) <=
					     sizeof (CCMI_CollectiveProtocol_t));

				SyncBinomialRegistration *treg =
				    (SyncBinomialRegistration *) registration;

				new (& treg->minfo)//, sizeof(treg->minfo))
				    CCMI::Adaptor::Generic::MulticastImpl();
				
				new (& treg->allreduce_registration)//, sizeof(treg->allreduce_registration))
				    CCMI::Adaptor::Allreduce::Binomial::Factory
				    (_g_generic_adaptor->mapping(), & treg->minfo, (CCMI_mapIdToGeometry)cb_geometry_map, flags);

				treg->minfo.initialize(_g_generic_adaptor);
#else
				CCMI::Adaptor::Generic::MulticastImpl *minfo = 
				  new(registration) CCMI::Adaptor::Generic::MulticastImpl();
				minfo->initialize(_g_generic_adaptor);
				TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::ShortAllreduceTag, minfo);
#endif

				return HL_SUCCESS;
			    }
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
			  {
			    CCMI::Adaptor::Generic::MulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::MulticastImpl();
			    COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BarrierTag, minfo);		    
			    return HL_SUCCESS;
			    break;
			  }
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
	CCMI::Adaptor::Generic::MulticastImpl                _minfo;
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
		    CCMI::Adaptor::Generic::MulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::MulticastImpl *)parms->registration;
		    tspcoll->ibcast(minfo, p_root, parms->src, parms->dst, parms->bytes,
				    (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_ALLGATHER:
		{
		    hl_allgather_t        * parms   = &cmd->xfer_allgather;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    CCMI::Adaptor::Generic::MulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::MulticastImpl *)parms->registration;
		    tspcoll->iallgather(minfo,parms->src, parms->dst, parms->bytes,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_ALLGATHERV:
		{
		    hl_allgatherv_t        * parms   = &cmd->xfer_allgatherv;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    CCMI::Adaptor::Generic::MulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::MulticastImpl *)parms->registration;
		    tspcoll->iallgatherv(minfo,parms->src, parms->dst, parms->lengths,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_SCATTER:
		{
		    typedef struct scatter_info
		    {
		      CCMI::Adaptor::Generic::MulticastImpl barrier;
		      CCMI::Adaptor::Generic::MulticastImpl scatter;
		    }scatter_info;
		    hl_scatter_t          * parms   = &cmd->xfer_scatter;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    scatter_info          * scinfo  = (scatter_info*)parms->registration;
		    int                     p_root  = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatter(&scinfo->barrier, &scinfo->scatter,
				      p_root,parms->src, parms->dst, parms->bytes,
				      (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
		}
		break;
	    case HL_XFER_SCATTERV:
		{
		    typedef struct scatterv_info
		    {
		      CCMI::Adaptor::Generic::MulticastImpl barrier;
		      CCMI::Adaptor::Generic::MulticastImpl scatterv;
		    }scatterv_info;
		    hl_scatterv_t         * parms   = &cmd->xfer_scatterv;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    scatterv_info         * scinfo  = (scatterv_info*)parms->registration;
		    int                     p_root  = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatterv(&scinfo->barrier, &scinfo->scatterv,
				       p_root, parms->src, parms->dst, parms->lengths,
				       (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return HL_SUCCESS;
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

			CCMI::Adaptor::Allreduce::Factory *factory =
			    (CCMI::Adaptor::Allreduce::Factory *)
			    parms->registration;

			//Also check for change in protocols
			if(allreduce != NULL  &&  allreduce->getFactory() == factory)
			    {
				unsigned status =  allreduce->restart((CCMI_CollectiveRequest_t*)parms->request,
								      *(CCMI_Callback_t *)&parms->cb_done,
								      (CCMI_Consistency)0,
								      parms->src,
								      parms->dst,
								      parms->count,
								      (CCMI_Dt)LL_to_CCMI_dt[parms->dt],
								      (CCMI_Op)LL_to_CCMI_op[parms->op]);
				if(status == CCMI_SUCCESS)
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
			void *ptr =factory->generate((CCMI_CollectiveRequest_t*)parms->request,
						     *(CCMI_Callback_t *) &parms->cb_done,
						     (CCMI_Consistency)0,
						     _c_geometry,
						     parms->src,
						     parms->dst,
						     parms->count,
						     (CCMI_Dt)LL_to_CCMI_dt[parms->dt],
						     (CCMI_Op)LL_to_CCMI_op[parms->op]);
			if(ptr == NULL)
			    {
				return HL_UNIMPL;
			    }

			return HL_SUCCESS;
		    }
#else
		    CCMI::Adaptor::Generic::MulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::MulticastImpl *)parms->registration;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->iallreduce(minfo,
					parms->src,           // source buffer
					parms->dst,           // dst buffer
					(CCMI_Op)LL_to_CCMI_op[parms->op], // op
					(CCMI_Dt)LL_to_CCMI_dt[parms->dt], // dt
					parms->count,             // type
					(void (*)(void*))parms->cb_done.function,
					parms->cb_done.clientdata);
#endif
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
		    CCMI::Adaptor::Generic::MulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::MulticastImpl *)parms->registration;		    
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->ibarrier(minfo,(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
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
