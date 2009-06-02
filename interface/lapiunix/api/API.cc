#include "collectives/interface/lapiunix/ccmi_collectives.h"
#include "collectives/interface/ccmi_internal.h"
#include "collectives/util/ccmi_util.h"
#include "collectives/interface/lapiunix/multisend/multisend_impl.h"
#include "collectives/interface/Geometry.h"
#include "collectives/util/logging/LogMgr.h"
#include "collectives/util/ccmi_debug.h"
#include "collectives/algorithms/protocols/barrier/barrier_impl.h"
#include "mapping_impl.h" // ? why

CCMI::Logging::LogMgr   * CCMI::Logging::LogMgr::_staticLogMgr __attribute__((weak));
CCMI::Adaptor::Adaptor  * _g_generic_adaptor;

extern "C" int CCMI_Collective_initialize ()
{
  if(_g_generic_adaptor == NULL)
  {
    void * buf = CCMI_Alloc(sizeof (CCMI::Adaptor::Adaptor));
    if(buf)
      _g_generic_adaptor = new (buf) CCMI::Adaptor::Adaptor();

    int size = _g_generic_adaptor->mapping()->size();
    int nbarrier = 16;
    //_g_rank_connlist = (void **) CCMI_Alloc(sizeof(void *) * size);
    //_g_singleton_connlist = (void **) CCMI_Alloc (sizeof(void *) * nbarrier);

  }

  CCMI::Logging::LogMgr::setLogMgr (_g_generic_adaptor->getLogMgr ());

  return(_g_generic_adaptor == NULL);
}

extern "C" int CCMI_Generic_adaptor_advance() {
  _g_generic_adaptor->advance();
}

extern "C" int CCMI_Collective_finalize ()
{

  FILE *fp = fopen ("log", "a");

  _g_generic_adaptor->getLogMgr()->dumpTimers(fp, _g_generic_adaptor->mapping());

  free (_g_generic_adaptor);
  fclose (fp);

  return CCMI_SUCCESS;
}

unsigned                    _ccmi_cached_geometry_comm;
CCMI_Geometry_t           * _ccmi_cached_geometry;

extern "C" int CCMI_Geometry_analyze(CCMI_Geometry_t *grequest,
				     CCMI_CollectiveProtocol_t *proto)
{
  return(((CCMI::Adaptor::ProtocolFactory *)proto) ->
         Analyze((CCMI::Adaptor::Geometry *)grequest));

}

extern "C" int CCMI_Geometry_free(CCMI_Geometry_t *grequest)
{
  CCMI::Adaptor::Geometry *geometry = (CCMI::Adaptor::Geometry *)grequest;
  geometry->freePermutation();
  geometry->freeAllocations();
  return CCMI_SUCCESS;
}


extern "C" int CCMI_Geometry_initialize (CCMI_Geometry_t            * grequest,
                                         unsigned                     id,
                                         unsigned                   * ranks,
                                         unsigned                     nranks,
                                         CCMI_CollectiveProtocol_t ** protocols,
                                         unsigned                     nprotocols,
                                         CCMI_CollectiveProtocol_t ** localprotocols,
                                         unsigned                     nlocalprotocols,
                                         CCMI_CollectiveRequest_t   * request,
                                         unsigned                     numcolors,
                                         unsigned                     globalcontext)
{
  CCMI::Adaptor::Geometry *geometry =
  new (grequest, sizeof(CCMI_Geometry_t))
    CCMI::Adaptor::Geometry (_g_generic_adaptor->mapping(),
			     ranks,
			     nranks,
			     id,
			     (bool)globalcontext);

  CCMI_assert (((unsigned long)request & 0x0f) == 0);

  //fprintf (stderr, "In geometry initialize %d protocols, %d local protocols\n", nprotocols,nlocalprotocols);

  CCMI::Executor::Executor *exe = NULL;

  CCMI_Executor_t *executors = (CCMI_Executor_t *) request;
  //  See _compile_time_assert_ instead
  //    CCMI_assert(sizeof(CCMI_Executor_t) * 2 <= sizeof(CCMI_CollectiveRequest_t));

  CCMI_assert (nprotocols > 0);
  for(unsigned count = 0; count < nprotocols; count++ )
  {
    CCMI::Adaptor::Barrier::BarrierFactory * factory =
    (CCMI::Adaptor::Barrier::BarrierFactory *) protocols[count];

    TRACE_ERR ((stderr, "Creating Barrier Protocol\n"));
    if(CCMI_Geometry_analyze(grequest, protocols[count]))
    {
      exe = factory->generate(&executors[0], geometry);
      geometry->setBarrierExecutor(exe);

      break;
    }
  }

  // We allow a geometry to not specify a local protocol
  //CCMI_assert (nlocalprotocols > 0);
  for(unsigned count = 0; count < nlocalprotocols; count++ )
  {
    CCMI::Adaptor::Barrier::BarrierFactory * factory =
    (CCMI::Adaptor::Barrier::BarrierFactory *) localprotocols[count];

    TRACE_ERR ((stderr, "Creating Local Barrier Protocol\n"));
    if(CCMI_Geometry_analyze(grequest, localprotocols[count]))
    {
      CCMI::Executor::Executor *l_exe = factory->generate(&executors[1], geometry);
      geometry->setLocalBarrierExecutor(l_exe);

      break;
    }
  }

  //return 0 for success
  return(exe == NULL);
}



//-----------------------------------------------------------------------------
//---------------------  Barrier ----------------------------------------------
//-----------------------------------------------------------------------------

extern "C" int CCMI_Barrier (CCMI_Geometry_t     * grequest,
                             CCMI_Callback_t       cb_done,
                             CCMI_Consistency      consistency)
{

  CCMI::Adaptor::Geometry *geometry =
  (CCMI::Adaptor::Geometry *)grequest;
  CCMI::Executor::Executor  *bar = geometry->getBarrierExecutor();

  TRACE_ADAPTOR ((stderr, "<%#.8X>CCMI_Barrier() Start Barrier cb_done %X, data %X\n", (int)bar, (int) cb_done.function, (int)cb_done.clientdata));

  bar->setDoneCallback    (cb_done.function, cb_done.clientdata);
  bar->setConsistency ((CCMI_Consistency) consistency);
  bar->start();
  return 0;
}



extern "C" int CCMI_Barrier_register (CCMI_CollectiveProtocol_t   * registration,
                                      CCMI_Barrier_Configuration_t * configuration)
{
  typedef struct
  {
    CCMI::Adaptor::Barrier::BinomialBarrierFactory   bar_registration;
    CCMI::Adaptor::Generic::MulticastImpl           minfo;
  } BinomialRegistration;

  int status = 1;

  switch(configuration->protocol)
  {
  case CCMI_BINOMIAL_BARRIER_PROTOCOL:
    {
      CCMI_assert (sizeof (BinomialRegistration) <=
                   sizeof (CCMI_CollectiveProtocol_t));

      BinomialRegistration *treg = (BinomialRegistration *) registration;

      new (& treg->minfo, sizeof(treg->minfo)) CCMI::Adaptor::Generic::MulticastImpl::MulticastImpl ();

      new (& treg->bar_registration, sizeof(treg->bar_registration)) CCMI::Adaptor::Barrier::
	BinomialBarrierFactory (static_cast<CCMI::MultiSend::MulticastInterface *>(&treg->minfo),
			      _g_generic_adaptor->mapping(),
			      configuration->cb_geometry);
      //optimize one level of callbacks
      treg->minfo.initialize (_g_generic_adaptor);

      status = CCMI_SUCCESS;
    }
    break;

  default:
    status = CCMI_INVAL;
    //CCMI_assert (0);
    break;
  };

  return status;
}

#if 0

//-----------------------------------------------------------------------------
//---------------------  Broadcast --------------------------------------------
//-----------------------------------------------------------------------------

#include "protocols/broadcast/mcbroadcast_impl.h"
#include "protocols/broadcast/async_impl.h"
#include "connmgr/ColorConnMgr.h"
#include "connmgr/ColorGeometryConnMgr.h"
#include "protocols/broadcast/tree/TreeBroadcast.h"

extern "C" int CCMI_Broadcast_register (CCMI_CollectiveProtocol_t      * registration,
                                        CCMI_Broadcast_Configuration_t * configuration)
{
  int status = CCMI_ERROR;

  // Our max size for SyncBcast is MAX(context_id) * maxColors = 64K*3 = 192K
  // context_id is the communicator_id that is the same on all ranks.
  int nconn = 0;

  typedef struct
  {
    CCMI::Adaptor::Broadcast::BinomialBcastFactory      bcast_registration;
    CCMI::ConnectionManager::ColorGeometryConnMgr           cg_connmgr;
    CCMI::ConnectionManager::ColorConnMgr                   c_connmgr;
    CCMI::Adaptor::Generic::MulticastImpl               minfo;
  } BinomialRegistration;

  switch(configuration->protocol)
  {
  case CCMI_TORUS_BINOMIAL_BROADCAST_PROTOCOL:
    {
      if(!_g_commcolor_connlist)
      {
        int nbcast = MAX_GEOMETRIES * MAX_COLORS;
        _g_commcolor_connlist  = (void **) malloc(sizeof(void *) * nbcast);
      }

      nconn = MAX_GEOMETRIES;
      CCMI_assert (sizeof (BinomialRegistration) <= sizeof (CCMI_CollectiveProtocol_t));

      BinomialRegistration *treg = (BinomialRegistration *) registration;

      new (& treg->cg_connmgr, sizeof(treg->cg_connmgr)) CCMI::ConnectionManager::ColorGeometryConnMgr(nconn);
      
      new (& treg->minfo, sizeof(treg->minfo)) CCMI::Adaptor::MultiSend::MulticastImpl
	(CCMI_MEMFIFO_DMA_MSEND_PROTOCOL, nconn, _g_commcolor_connlist);

      new (& treg->bcast_registration, sizeof(treg->bcast_registration))
	CCMI::Adaptor::Broadcast::BinomialBcastFactory
      (_g_generic_adaptor->mapping(), & treg->minfo, & treg->cg_connmgr, nconn );

      status = CCMI_SUCCESS;
    }
    break;

  default:
    status = CCMI_INVAL;
    break;
  };

  return status;
}


extern "C" int CCMI_Broadcast (CCMI_CollectiveProtocol_t  * registration,
                               CCMI_CollectiveRequest_t   * request,
                               CCMI_Callback_t    cb_done,
                               CCMI_Consistency   consistency,
                               CCMI_Geometry_t  * geometry,
                               unsigned           root,
                               char             * src,
                               unsigned           bytes)
{
  CCMI::Adaptor::Broadcast::BroadcastFactory *factory =

  (CCMI::Adaptor::Broadcast::BroadcastFactory *) registration;

  CCMI_assert (((CCMI::Adaptor::Geometry *) geometry)->getBarrierExecutor() != NULL);

  if(bytes == 0) // let's take care of this zero length crap right now
  {
    cb_done.function(cb_done.clientdata, NULL);
  }
  else
  {
    CCMI_Callback_t cb_done_ccmi;
    cb_done_ccmi.function = cb_done.function;
    cb_done_ccmi.clientdata = cb_done.clientdata;
    
    factory->generate(request, sizeof(CCMI_CollectiveRequest_t), cb_done_ccmi,
                      (CCMI_Consistency) consistency,
                      (CCMI::Adaptor::Geometry *) geometry,
                      root,
                      src,
                      bytes);
  }

  return CCMI_SUCCESS;
}



#endif

//-----------------------------------------------------------------------------
//---------------------  Allreduce --------------------------------------------
//-----------------------------------------------------------------------------


#include "collectives/algorithms/protocols/allreduce/sync_impl.h"

extern "C"
int CCMI_Allreduce_register (CCMI_CollectiveProtocol_t      * registration,
                             CCMI_Allreduce_Configuration_t * configuration)
{

  CCMI::Adaptor::ConfigFlags flags;
  if(configuration->reuse_storage)
  {
    char* envopts = getenv("CCMI_ALLREDUCE_REUSE_STORAGE_LIMIT");
    if(envopts != NULL)
      flags.reuse_storage_limit = atoi(envopts);
    else
      flags.reuse_storage_limit = CCMI_DEFAULT_REUSE_STORAGE_LIMIT;
  }
  else
    flags.reuse_storage_limit = 0;

  {
    char* envopts = getenv("CCMI_ALLREDUCE_PIPELINE_OVERRIDE");
    if(envopts != NULL)
      flags.pipeline_override = atoi(envopts);
    else
      flags.pipeline_override = 0;
  }
  int status = CCMI_ERROR;

  switch(configuration->protocol)
  {
  case CCMI_BINOMIAL_ALLREDUCE_PROTOCOL:
    {
      typedef struct
      {
        CCMI::Adaptor::Allreduce::Binomial::Factory    allreduce_registration;
        CCMI::Adaptor::Generic::MulticastImpl                         minfo;
      } SyncBinomialRegistration;

      CCMI_assert (sizeof (SyncBinomialRegistration) <=
                   sizeof (CCMI_CollectiveProtocol_t));

      SyncBinomialRegistration *treg =
      (SyncBinomialRegistration *) registration;

      new (& treg->minfo, sizeof(treg->minfo))
      CCMI::Adaptor::Generic::MulticastImpl();

      new (& treg->allreduce_registration, sizeof(treg->allreduce_registration))
      CCMI::Adaptor::Allreduce::Binomial::Factory
      (_g_generic_adaptor->mapping(), & treg->minfo, configuration->cb_geometry, flags);

      treg->minfo.initialize(_g_generic_adaptor);
                             /*(CCMI_MEMFIFO_DMA_MSEND_PROTOCOL,
                             _g_adaptor->mapping()->size(),
                             _g_rank_connlist);*/

      status = CCMI_SUCCESS;
    }
    break;
  case CCMI_SHORT_BINOMIAL_ALLREDUCE_PROTOCOL:
    {
      typedef struct
      {
        CCMI::Adaptor::Allreduce::ShortBinomial::Factory  allreduce_registration;
        CCMI::Adaptor::Generic::MulticastImpl              minfo;
      } SyncBinomialRegistration;

      CCMI_assert (sizeof (SyncBinomialRegistration) <=
                   sizeof (CCMI_CollectiveProtocol_t));

      SyncBinomialRegistration *treg =
      (SyncBinomialRegistration *) registration;

      new (& treg->minfo, sizeof(treg->minfo))
      CCMI::Adaptor::Generic::MulticastImpl();

      new (& treg->allreduce_registration, sizeof(treg->allreduce_registration))
      CCMI::Adaptor::Allreduce::ShortBinomial::Factory
      (_g_generic_adaptor->mapping(), & treg->minfo, configuration->cb_geometry, flags);

      treg->minfo.initialize(_g_generic_adaptor);
                            /*(CCMI_MEMFIFO_DMA_MSEND_PROTOCOL,
                             _g_adaptor->mapping()->size(),
                             _g_rank_connlist);*/

      status = CCMI_SUCCESS;
    }
    break;

  default:
    CCMI_assert (0);
    break;
  }

  return status;
}



extern "C"
int CCMI_Allreduce (CCMI_CollectiveProtocol_t  * registration,
                    CCMI_CollectiveRequest_t   * request,
                    CCMI_Callback_t              cb_done,
                    CCMI_Consistency             consistency,
                    CCMI_Geometry_t            * geometry_request,
                    char                       * srcbuf,
                    char                       * dstbuf,
                    unsigned                     count,
                    CCMI_Dt                      dtype,
                    CCMI_Op                      op )
{
  CCMI::Adaptor::Geometry *geometry = (CCMI::Adaptor::Geometry *) geometry_request;
  CCMI::Adaptor::Allreduce::BaseComposite * allreduce =
  (CCMI::Adaptor::Allreduce::BaseComposite *) geometry->getAllreduceComposite();

  CCMI::Adaptor::Allreduce::Factory *factory =
  (CCMI::Adaptor::Allreduce::Factory *) registration;

  //Also check for change in protocols
  if(allreduce != NULL  &&  allreduce->getFactory() == factory)
  {
//      fprintf(stderr, "allreduce restart\n");

    unsigned status =  allreduce->restart((CCMI_CollectiveRequest_t*)request,
                                          *(CCMI_Callback_t *)&cb_done,
                                          (CCMI_Consistency)consistency,
                                          srcbuf,
                                          dstbuf,
                                          count,
                                          (CCMI_Dt)dtype,
                                          (CCMI_Op)op);

    if(status == CCMI_SUCCESS)
    {
//      fprintf(stderr, "allreducecomposite\n");
      geometry->setAllreduceComposite(allreduce);
      TRACE_ADAPTOR((stderr, "<%#.8X>CCMI_Allreduce::ALERT: restart successful with status %#X\n", 
                 (int) allreduce, status));
      return status;
    }
    else TRACE_ADAPTOR((stderr, "<%#.8X>CCMI_Allreduce::ALERT: restart unsuccessful with status %#X\n", 
                    (int) allreduce, status));

  }

  if(allreduce != NULL) // Different factory?  Cleanup old executor.
  {
    geometry->setAllreduceComposite(NULL);
    allreduce->~BaseComposite();
  }
  //fprintf(stderr, "CCMI_Allreduce::ALERT: generate executor %#X with factory %#X\n",(int) allreduce,(int)factory);
  void *ptr =factory->generate((CCMI_CollectiveRequest_t*)request,
                               *(CCMI_Callback_t *) &cb_done,
                               (CCMI_Consistency) consistency,
                               geometry,
                               srcbuf,
                               dstbuf,
                               count,
                               (CCMI_Dt)dtype,
                               (CCMI_Op)op);
  if(ptr == NULL)
  {
//    fprintf(stderr, "CCMI_Allreduce::ALERT: generate failed\n");
    return CCMI_UNIMPL;
  }

  return CCMI_SUCCESS;
}



extern "C" void __libccmicoll_dummy () __attribute__((weak));

extern "C" void __libccmicoll_dummy ()
{
  CCMI_abort();
}
extern "C" void __cxa_pure_virtual  () __attribute__((weak, alias("__libccmicoll_dummy")));


