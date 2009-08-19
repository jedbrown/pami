#include "../xmi_collectives.h"
#include <stdio.h>

// Includes for CCMI
#include "interface/ccmi_internal.h"
#include "util/ccmi_util.h"
#include "interface/lapiunix/multisend/multisend_impl.h"
#include "interface/Geometry.h"
#include "util/logging/LogMgr.h"
#include "util/ccmi_debug.h"
#include "algorithms/protocols/barrier/impl.h"
#include "algorithms/protocols/allreduce/sync_impl.h"
#include "algorithms/protocols/allreduce/async_impl.h"
#include "interface/lapiunix/api/mapping_impl.h" // ? why
#include "interface/lapiunix/GenericComm.h"
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

    int XMI_to_CCMI_op[] =
	{XMI_UNDEFINED_OP,        // XMI_UNDEFINED_OP
	 XMI_NOOP,                // XMI_NOOP
	 XMI_MAX,                 // XMI_MAX,
	 XMI_MIN,                 // XMI_MIN,
	 XMI_SUM,                 // XMI_SUM,
	 XMI_PROD,                // XMI_PROD,
	 XMI_LAND,                // XMI_LAND,
	 XMI_LOR,                 // XMI_LOR,
	 XMI_LXOR,                // XMI_LXOR,
	 XMI_BAND,                // XMI_BAND,
	 XMI_BOR,                 // XMI_BOR,
	 XMI_BXOR,                // XMI_BXOR,
	 XMI_MAXLOC,              // XMI_MAXLOC,
	 XMI_MINLOC,              // XMI_MINLOC,
	 XMI_USERDEFINED_OP,      // XMI_USERDEFINED_OP,
	 XMI_OP_COUNT             // XMI_OP_COUNT
	};
    int XMI_to_CCMI_dt[] =
	{
	 /* Standard/Primative DT's */
	 XMI_UNDEFINED_DT,        // XMI_UNDEFINED_DT = 0,
	 XMI_SIGNED_CHAR,         // XMI_SIGNED_CHAR,
	 XMI_UNSIGNED_CHAR,       // XMI_UNSIGNED_CHAR,
	 XMI_SIGNED_SHORT,        // XMI_SIGNED_SHORT,
	 XMI_UNSIGNED_SHORT,      // XMI_UNSIGNED_SHORT,
	 XMI_SIGNED_INT,          // XMI_SIGNED_INT,
	 XMI_UNSIGNED_INT,        // XMI_UNSIGNED_INT,
	 XMI_SIGNED_LONG_LONG,    // XMI_SIGNED_LONG_LONG,
	 XMI_UNSIGNED_LONG_LONG,  // XMI_UNSIGNED_LONG_LONG,
	 XMI_FLOAT,               // XMI_FLOAT,
	 XMI_DOUBLE,              // XMI_DOUBLE,
	 XMI_LONG_DOUBLE,         // XMI_LONG_DOUBLE,
	 XMI_LOGICAL,             // XMI_LOGICAL,
	 XMI_SINGLE_COMPLEX,      // XMI_SINGLE_COMPLEX,
	 XMI_DOUBLE_COMPLEX,      // XMI_DOUBLE_COMPLEX,
	 /* Max/Minloc DT's */
	 XMI_LOC_2INT,            // XMI_LOC_2INT,
	 XMI_LOC_SHORT_INT,       // XMI_LOC_SHORT_INT,
	 XMI_LOC_FLOAT_INT,       // XMI_LOC_FLOAT_INT,
	 XMI_LOC_DOUBLE_INT,      // XMI_LOC_DOUBLE_INT,
	 XMI_LOC_2FLOAT,          // XMI_LOC_2FLOAT,
	 XMI_LOC_2DOUBLE,         // XMI_LOC_2DOUBLE,
	 XMI_USERDEFINED_DT,      // XMI_USERDEFINED_DT,
	 -1
	};


    typedef struct
    {
	int size;
	XMI_CollectiveProtocol_t *entries[MAX_REGISTRATIONS_PER_TABLE];
    }XMI_proto_table;

    CCMI_Geometry_t      *getGeometry (int comm);

    XMI_Geometry_t         XMI_World_Geometry;
    unsigned              XMI_World_Geometry_id=0;
    XMI_Geometry_range_t   world_range;
    XMI_mapIdToGeometry    cb_geometry_map;


    int XMI_Collectives_initialize(int *argc,
				  char***argv,
				  XMI_mapIdToGeometry cb_map)
    {
      // Set up pgasrt P2P Collectives
	__pgasrt_tsp_setup         (1, argc, argv);
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
	world_range.hi       = XMI_Size()-1;
	//	TSPColl::NBCollManager::instance();
	XMI_Geometry_initialize(&XMI_World_Geometry,
			       XMI_World_Geometry_id,
			       &world_range,
			       1);
	__pgasrt_tsp_barrier       ();
	return XMI_SUCCESS;
    }

    int XMI_Poll()
    {
	_g_generic_adaptor->advance();
//	__pgasrt_tsp_wait(NULL);
	return 0;
    }

    int XMI_Rank()
    {
	return _g_generic_adaptor->mapping()->rank();
    }

    int XMI_Size()
    {
	return _g_generic_adaptor->mapping()->size();
    }

    int XMI_Collectives_finalize()
    {
      //	__pgasrt_tsp_finish();

//	FILE *fp = fopen ("log", "a");
//	_g_generic_adaptor->getLogMgr()->dumpTimers(fp, _g_generic_adaptor->mapping());
	free (_g_generic_adaptor);
	__pgasrt_tsp_barrier       ();
//	fclose (fp);
	return 0;
    }


    int XMI_register(XMI_CollectiveProtocol_t      *registration,
                    XMI_CollectiveConfiguration_t *config,
		    int                           key)
    {
	XMI_Result                                status;
	switch(config->cfg_type)
	    {
	    case XMI_CFG_BROADCAST:
		{
		    XMI_Broadcast_Configuration_t *cfg    = (XMI_Broadcast_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_BROADCAST_PROTOCOL:
			  {
			    CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
			    COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BcastTag, minfo);		    
			    return XMI_SUCCESS;
			    break;
			  }
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
		break;
	    case XMI_CFG_ALLGATHER:
		{
		    XMI_Allgather_Configuration_t *cfg    = (XMI_Allgather_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_ALLGATHER_PROTOCOL:
			  {
			    CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::AllgatherTag, minfo);			    
			    return XMI_SUCCESS;
			    break;
			  }
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
		break;
	    case XMI_CFG_ALLGATHERV:
		{
		    XMI_Allgatherv_Configuration_t *cfg    = (XMI_Allgatherv_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_ALLGATHERV_PROTOCOL:
			  {
			    CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
			    COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::AllgathervTag, minfo);			    
			    return XMI_SUCCESS;
			    break;
			  }
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
		break;
	    case XMI_CFG_SCATTER:
		{
		    XMI_Scatter_Configuration_t *cfg    = (XMI_Scatter_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_SCATTER_PROTOCOL:
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
			    return XMI_SUCCESS;
			    break;
			  }
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
		break;
	    case XMI_CFG_SCATTERV:
		{
		    XMI_Scatterv_Configuration_t *cfg    = (XMI_Scatterv_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_SCATTERV_PROTOCOL:
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
			    return XMI_SUCCESS;
			    break;
			  }
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
		break;
	    case XMI_CFG_ALLREDUCE:
		{
		    XMI_Allreduce_Configuration_t *cfg    = (XMI_Allreduce_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_ALLREDUCE_PROTOCOL:
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
					     sizeof (XMI_CollectiveProtocol_t));

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

				return XMI_SUCCESS;
			    }
			    break;
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
		break;
#if 0
	    case XMI_CFG_ALLTOALL:
		{
		    XMI_Alltoall_Configuration_t *cfg   = (XMI_Alltoall_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_ALLTOALL_PROTOCOL:
			  {
			    typedef struct
			    {
			      CCMI::Adaptor::AlltoallFactory       alltoall_registration;
			      CCMI::Adaptor::Generic::ManytomanyImpl minfo;
			    } AlltoallRegistration;
			    
			    CCMI_assert( sizeof(AlltoallRegistration) <=
					 sizeof(XMI_CollectiveProtocol_t) );
			    
			    AlltoallRegistration * treg = 
			      (AlltoallRegistration *) registration;
			    
			    new ( & treg->minfo ) 
			      CCMI::Adaptor::Generic::ManytomanyImpl();
			    new ( & treg->alltoall_registration ) 
			      CCMI::Adaptor::AlltoallFactory(& treg->minfo,
							       _g_generic_adaptor->mapping() );
			    
			    treg->minfo.initialize(_g_generic_adaptor);
			  
			    return XMI_SUCCESS;
			  }
			  break;
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
		break;

	    case XMI_CFG_ALLTOALLV:
		{
		    XMI_Alltoallv_Configuration_t *cfg   = (XMI_Alltoallv_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_ALLTOALLV_PROTOCOL:
			    return XMI_SUCCESS;
			    break;
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
		break;
#endif
	    case XMI_CFG_BARRIER:
		{
		    XMI_Barrier_Configuration_t *cfg = (XMI_Barrier_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_BARRIER_PROTOCOL:
			  {
			    CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
			      new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
			    COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
			    minfo->initialize(_g_generic_adaptor);
			    TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BarrierTag, minfo);		    
			    return XMI_SUCCESS;
			    break;
			  }
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
		break;
	    case XMI_CFG_AMBROADCAST:
		{
		    XMI_AMBroadcast_Configuration_t *cfg   = (XMI_AMBroadcast_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_AMBROADCAST_PROTOCOL:
			  {
			    typedef struct
			    {
			      CCMI::Adaptor::Broadcast::AsyncBinomialFactory    bcast_registration;
			      CCMI::Adaptor::Generic::OldMulticastImpl             minfo;
			    } AsyncBinomialRegistration;
			    CCMI_assert (sizeof (AsyncBinomialRegistration) <=
					 sizeof (XMI_CollectiveProtocol_t));
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
			    return XMI_SUCCESS;
			    break;
			  }
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
	    case XMI_CFG_AMSCATTER:
		{
		    XMI_AMScatter_Configuration_t *cfg   = (XMI_AMScatter_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_AMSCATTER_PROTOCOL:
			    return XMI_SUCCESS;
			    break;
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
	    case XMI_CFG_AMGATHER:
		{
		    XMI_AMGather_Configuration_t *cfg   = (XMI_AMGather_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_AMGATHER_PROTOCOL:
			    return XMI_SUCCESS;
			    break;
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}
	    case XMI_CFG_AMREDUCE:
		{
		    XMI_AMReduce_Configuration_t *cfg   = (XMI_AMReduce_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case XMI_DEFAULT_AMREDUCE_PROTOCOL:
			    return XMI_SUCCESS;
			    break;
			default:
			    return XMI_UNIMPL;
			    break;
			}
		}

	    default:
		return XMI_UNIMPL;
		break;
	    }
	return XMI_SUCCESS;
    }

    class geometry_internal
    {
    public:
	geometry_internal(int                  my_rank,
			  unsigned             slice_count,
			  XMI_Geometry_range_t *rank_slices,
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
	void              *g_ptr = &XMI_World_Geometry;
	geometry_internal *ptr   = (geometry_internal *)g_ptr;
	return (CCMI_Geometry_t *)&ptr->_ccmi_geometry;
	// This is OK because _ccmi_geometry is the first data item in the class
	// If both pgasrt and ccmi are delivering this callback, we
	// need to implement the geometry lookup for the right class.
	return (CCMI_Geometry_t *)&XMI_World_Geometry;
    }


    int XMI_Geometry_initialize (XMI_Geometry_t            * geometry,
				unsigned                   id,
				XMI_Geometry_range_t      * rank_slices,
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
	return XMI_SUCCESS;
    }


    int XMI_Geometry_finalize(XMI_Geometry_t *geometry)
    {
	geometry_internal *g = (geometry_internal*)geometry;
	free(g->_ranklist);
	return XMI_SUCCESS;
    }


    // Currently all algorithms are implemented
    int XMI_Geometry_algorithm (XMI_Geometry_t            *geometry,
			       XMI_CollectiveProtocol_t **protocols,
			       int                      *num)
    {
	return XMI_SUCCESS;
    }



    int XMI_Xfer (void* context, XMI_Xfer_t *cmd)
    {
	switch(cmd->xfer_type)
	    {
	    case XMI_XFER_BROADCAST:
		{
		    XMI_Broadcast_t        * parms   = &cmd->xfer_broadcast;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    int p_root                      = tspcoll->virtrankof(parms->root);
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;
		    tspcoll->ibcast(minfo, p_root, parms->src, parms->dst, parms->bytes,
				    (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return XMI_SUCCESS;
		}
		break;
	    case XMI_XFER_ALLGATHER:
		{
		    XMI_Allgather_t        * parms   = &cmd->xfer_allgather;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;
		    tspcoll->iallgather(minfo,parms->src, parms->dst, parms->bytes,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return XMI_SUCCESS;
		}
		break;
	    case XMI_XFER_ALLGATHERV:
		{
		    XMI_Allgatherv_t        * parms   = &cmd->xfer_allgatherv;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;
		    tspcoll->iallgatherv(minfo,parms->src, parms->dst, parms->lengths,
					(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return XMI_SUCCESS;
		}
		break;
	    case XMI_XFER_SCATTER:
		{
		    typedef struct scatter_info
		    {
		      CCMI::Adaptor::Generic::OldMulticastImpl barrier;
		      CCMI::Adaptor::Generic::OldMulticastImpl scatter;
		    }scatter_info;
		    XMI_Scatter_t          * parms   = &cmd->xfer_scatter;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    scatter_info          * scinfo  = (scatter_info*)parms->registration;
		    int                     p_root  = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatter(&scinfo->barrier, &scinfo->scatter,
				      p_root,parms->src, parms->dst, parms->bytes,
				      (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return XMI_SUCCESS;
		}
		break;
	    case XMI_XFER_SCATTERV:
		{
		    typedef struct scatterv_info
		    {
		      CCMI::Adaptor::Generic::OldMulticastImpl barrier;
		      CCMI::Adaptor::Generic::OldMulticastImpl scatterv;
		    }scatterv_info;
		    XMI_Scatterv_t         * parms   = &cmd->xfer_scatterv;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    scatterv_info         * scinfo  = (scatterv_info*)parms->registration;
		    int                     p_root  = tspcoll->virtrankof(parms->root);
		    tspcoll->iscatterv(&scinfo->barrier, &scinfo->scatterv,
				       p_root, parms->src, parms->dst, parms->lengths,
				       (void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
		    return XMI_SUCCESS;
		}
		break;
	    case XMI_XFER_ALLREDUCE:
		{
		    XMI_Allreduce_t        * parms   = &cmd->xfer_allreduce;
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
				unsigned status =  allreduce->restart((XMI_CollectiveRequest_t*)parms->request,
								      *(XMI_Callback_t *)&parms->cb_done,
								      (CCMI_Consistency)0,
								      parms->src,
								      parms->dst,
								      parms->count,
								      (XMI_Dt)XMI_to_CCMI_dt[parms->dt],
								      (XMI_Op)XMI_to_CCMI_op[parms->op]);
				if(status == XMI_SUCCESS)
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
			void *ptr =factory->generate((XMI_CollectiveRequest_t*)parms->request,
						     *(XMI_Callback_t *) &parms->cb_done,
						     (CCMI_Consistency)0,
						     _c_geometry,
						     parms->src,
						     parms->dst,
						     parms->count,
						     (XMI_Dt)XMI_to_CCMI_dt[parms->dt],
						     (XMI_Op)XMI_to_CCMI_op[parms->op]);
			if(ptr == NULL)
			    {
				return XMI_UNIMPL;
			    }

			return XMI_SUCCESS;
		    }
#else
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->iallreduce(minfo,
					parms->src,           // source buffer
					parms->dst,           // dst buffer
					(XMI_Op)XMI_to_CCMI_op[parms->op], // op
					(XMI_Dt)XMI_to_CCMI_dt[parms->dt], // dt
					parms->count,             // type
					(void (*)(void*))parms->cb_done.function,
					parms->cb_done.clientdata);
#endif
		    return XMI_SUCCESS;
		}
		break;
	    case XMI_XFER_ALLTOALLV:
		{
		    XMI_Alltoall_t         * parms   = &cmd->xfer_alltoall;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		}
		break;

	    case XMI_XFER_ALLTOALL:
		{
		    XMI_Alltoall_t         * parms   = &cmd->xfer_alltoall;

geometry_internal         * g       = (geometry_internal*)parms->geometry;
		CCMI::Adaptor::Geometry   *_c_geometry    = (CCMI::Adaptor::Geometry *)&g->_ccmi_geometry;

		    CCMI::Adaptor::AlltoallFactory *factory =
		      (CCMI::Adaptor::AlltoallFactory *)parms->registration;

		    XMI_Callback_t cb_done_ccmi;
		    cb_done_ccmi.function   = (void (*)(void*, XMI_Error_t*))parms->cb_done.function;
		    cb_done_ccmi.clientdata = parms->cb_done.clientdata;

		    factory->generate((XMI_CollectiveRequest_t *)parms->request,
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

	    case XMI_XFER_BARRIER:
		{
		    XMI_Barrier_t          * parms   = &cmd->xfer_barrier;
		    geometry_internal     * g       = (geometry_internal*)parms->geometry;
#ifdef USE_CCMI
		    CCMI::Adaptor::Geometry   *_c_geometry    = (CCMI::Adaptor::Geometry *)&g->_ccmi_geometry;
		    CCMI::Executor::Executor  *_c_bar         = _c_geometry->getBarrierExecutor();
		    _c_bar->setDoneCallback    ((void (*)(void*, XMI_Error_t*))parms->cb_done.function, parms->cb_done.clientdata);
		    _c_bar->setConsistency ((CCMI_Consistency) 0);
		    _c_bar->start();
		    return XMI_SUCCESS;
#else
		    CCMI::Adaptor::Generic::OldMulticastImpl *minfo =
		      (CCMI::Adaptor::Generic::OldMulticastImpl *)parms->registration;		    
		    TSPColl::Communicator * tspcoll = (TSPColl::Communicator *)&g->_pgasrt_comm;
		    tspcoll->ibarrier(minfo,(void (*)(void*))parms->cb_done.function,parms->cb_done.clientdata);
#endif
		    return XMI_SUCCESS;
		}
		break;
	    case XMI_XFER_AMBROADCAST:
	      {
#ifdef USE_CCMI
		XMI_Ambroadcast_t          * parms   = &cmd->xfer_ambroadcast;
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
		    XMI_Callback_t cb_done_ccmi;
		    cb_done_ccmi.function   = (void (*)(void*, XMI_Error_t*))parms->cb_done.function;
		    cb_done_ccmi.clientdata = parms->cb_done.clientdata;
		    factory->generate(parms->request, 
				      sizeof(XMI_CollectiveRequest_t), 
				      cb_done_ccmi,
				      (CCMI_Consistency)0,
				      _c_geometry,
				      _g_generic_adaptor->mapping()->rank(), //root
				      parms->src,
				      parms->bytes);
		  }

  return XMI_SUCCESS;
		
#endif
		return XMI_SUCCESS;
	      }
	      break;
	    case XMI_XFER_AMSCATTER:
	      {
		return XMI_SUCCESS;
	      }
	      break;
            case XMI_XFER_AMGATHER:
	      {
		return XMI_SUCCESS;
	      }
	      break;
            case XMI_XFER_AMREDUCE:
	      {
		return XMI_SUCCESS;
	      }
	      break;
	    default:
		return XMI_UNIMPL;
		break;
	    }
	return XMI_SUCCESS;
    }
};
