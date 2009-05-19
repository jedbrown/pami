
#include "ccmi_internal.h"
#include "ccmi_util.h"
#include "multisend/multisend_impl.h"
#include "Geometry.h"
#include "adaptor/ccmi_internal.h"
#include "logging/LogMgr.h"
#include "adaptor/ccmi_debug.h"
#include "protocols/barrier/barrier_impl.h"
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

//-----------------------------------------------------------------------------
//---------------------  Broadcast --------------------------------------------
//-----------------------------------------------------------------------------

#include "protocols/broadcast/mcbroadcast_impl.h"
#include "protocols/broadcast/async_impl.h"
#include "connmgr/ColorGeometryConnMgr.h"

extern "C" int CCMI_Broadcast_register (CCMI_CollectiveProtocol_t      * registration,
                                        CCMI_Broadcast_Configuration_t * configuration)
{
  int status = CCMI_ERROR;

  // Our max size for SyncBcast is MAX(context_id) * maxColors = 64K*3 = 192K
  // context_id is the communicator_id that is the same on all ranks.
  int nconn = 0;

  typedef struct
  {
    CCMI::Adaptor::Broadcast::BinomialBcastFactory         bcast_registration;
    CCMI::ConnectionManager::ColorGeometryConnMgr          cg_connmgr;
    CCMI::Adaptor::Generic::MulticastImpl                  minfo;
  } BinomialRegistration;

  typedef struct 
  {
    CCMI::Adaptor::Broadcast::RingBcastFactory             bcast_registration;
    CCMI::ConnectionManager::ColorGeometryConnMgr          cg_connmgr;
    CCMI::Adaptor::Generic::MulticastImpl                  minfo;
  } RingRegistration;
    
  typedef struct
  {
    CCMI::Adaptor::Broadcast::AsyncBinomialFactory    bcast_registration;
    CCMI::Adaptor::Generic::MulticastImpl             minfo;
  } AsyncBinomialRegistration;

  switch(configuration->protocol)
  {
  case CCMI_BINOMIAL_BROADCAST_PROTOCOL:
    {
      nconn = MAX_GEOMETRIES;
      CCMI_assert (sizeof (BinomialRegistration) <= sizeof (CCMI_CollectiveProtocol_t));

      BinomialRegistration *treg = (BinomialRegistration *) registration;

      new (& treg->cg_connmgr, sizeof(treg->cg_connmgr)) CCMI::ConnectionManager::ColorGeometryConnMgr(nconn);
      new (& treg->minfo, sizeof(treg->minfo)) CCMI::Adaptor::Generic::MulticastImpl();
      new (& treg->bcast_registration, sizeof(treg->bcast_registration))
	CCMI::Adaptor::Broadcast::BinomialBcastFactory
      (_g_generic_adaptor->mapping(), & treg->minfo, & treg->cg_connmgr, nconn );

      status = CCMI_SUCCESS;
    }
    break;
    
  case CCMI_RING_BROADCAST_PROTOCOL:
    {
      nconn = MAX_GEOMETRIES;

      CCMI_assert (sizeof (RingRegistration) <= sizeof (CCMI_CollectiveProtocol_t));
      RingRegistration *treg = (RingRegistration *) registration;
      
      new (& treg->minfo, sizeof(treg->minfo)) CCMI::Adaptor::Generic::MulticastImpl();
      new (& treg->cg_connmgr, sizeof(treg->cg_connmgr)) CCMI::ConnectionManager::ColorGeometryConnMgr(nconn);

      new (& treg->bcast_registration, sizeof(treg->bcast_registration))
        CCMI::Adaptor::Broadcast::RingBcastFactory
        (_g_generic_adaptor->mapping(), & treg->minfo, & treg->cg_connmgr, nconn );
      
      treg->minfo.initialize (_g_generic_adaptor);

      status = CCMI_SUCCESS;
    }
    break;    

    //////////////  Asynchronous Broadcast Protocols ///////////////////////////////
  case CCMI_ASYNCBINOMIAL_BROADCAST_PROTOCOL:
    {
      CCMI_assert (sizeof (AsyncBinomialRegistration) <=
		   sizeof (CCMI_CollectiveProtocol_t));
    
      AsyncBinomialRegistration *treg =
	(AsyncBinomialRegistration *) registration;
    
    new (& treg->minfo,
         sizeof(treg->minfo)) CCMI::Adaptor::Generic::MulticastImpl();
    
    CCMI::Adaptor::Broadcast::AsyncBinomialFactory *factory =
      new (& treg->bcast_registration, sizeof(treg->bcast_registration))
      CCMI::Adaptor::Broadcast::AsyncBinomialFactory
      (_g_generic_adaptor->mapping(), & treg->minfo, _g_generic_adaptor->mapping()->size()); 
    
    factory->setAsyncInfo(configuration->isBuffered,
                          configuration->cb_recv,
                          configuration->cb_geometry);
    
    //optimize one level of callbacks
    treg->minfo.initialize (_g_generic_adaptor);
    
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



//-----------------------------------------------------------------------------
//---------------------  Allreduce --------------------------------------------
//-----------------------------------------------------------------------------


#include "protocols/allreduce/sync_impl.h"
#include "protocols/allreduce/async_impl.h"

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
  case CCMI_RING_ALLREDUCE_PROTOCOL:
    {
      typedef struct
      {
        CCMI::Adaptor::Allreduce::Ring::Factory    allreduce_registration;
        CCMI::Adaptor::Generic::MulticastImpl               minfo;
      } SyncRegistration;

      CCMI_assert (sizeof (SyncRegistration) <=
                   sizeof (CCMI_CollectiveProtocol_t));

      SyncRegistration *treg =
      (SyncRegistration *) registration;

      new (& treg->minfo, sizeof(treg->minfo))
      CCMI::Adaptor::Generic::MulticastImpl();

      new (& treg->allreduce_registration, sizeof(treg->allreduce_registration))
      CCMI::Adaptor::Allreduce::Ring::Factory
      (_g_generic_adaptor->mapping(), & treg->minfo, configuration->cb_geometry, flags);

      treg->minfo.initialize(_g_generic_adaptor);
      status = CCMI_SUCCESS;
    }
    break;
  case CCMI_ASYNC_SHORT_BINOMIAL_ALLREDUCE_PROTOCOL:
    {
      typedef struct
      {
        CCMI::Adaptor::Allreduce::ShortBinomial::AsyncFactory  allreduce_registration;
        CCMI::Adaptor::Generic::MulticastImpl                  minfo;
      } AsyncBinomialRegistration;

      CCMI_assert (sizeof (AsyncBinomialRegistration) <=
                   sizeof (CCMI_CollectiveProtocol_t));

      AsyncBinomialRegistration *treg =
      (AsyncBinomialRegistration *) registration;

      new (& treg->minfo, sizeof(treg->minfo))
      CCMI::Adaptor::Generic::MulticastImpl();

      new (& treg->allreduce_registration, sizeof(treg->allreduce_registration))
      CCMI::Adaptor::Allreduce::ShortBinomial::AsyncFactory
      (_g_generic_adaptor->mapping(), & treg->minfo, configuration->cb_geometry, flags);
      

      treg->minfo.initialize(_g_generic_adaptor);

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

//-----------------------------------------------------------------------------
//------------------------  Reduce --------------------------------------------
//-----------------------------------------------------------------------------


extern "C"
int CCMI_Reduce_register (CCMI_CollectiveProtocol_t   * registration,
                          CCMI_Reduce_Configuration_t * configuration)
{
  CCMI::Adaptor::ConfigFlags flags;
  if(configuration->reuse_storage)
  {
    char* envopts = getenv("CCMI_REDUCE_REUSE_STORAGE_LIMIT");
    if(envopts != NULL)
      flags.reuse_storage_limit = atoi(envopts);
    else
      flags.reuse_storage_limit = CCMI_DEFAULT_REUSE_STORAGE_LIMIT;
  }
  else
    flags.reuse_storage_limit = 0;

  {
    char* envopts = getenv("CCMI_REDUCE_PIPELINE_OVERRIDE");
    if(envopts != NULL)
      flags.pipeline_override = atoi(envopts);
    else
      flags.pipeline_override = 0;
  }

  int status = CCMI_ERROR;

  switch(configuration->protocol)
  {
  case CCMI_RING_REDUCE_PROTOCOL:
    {
      typedef struct
      {
        CCMI::Adaptor::Allreduce::RingReduce::Factory reduce_registration;
        CCMI::Adaptor::Generic::MulticastImpl                  minfo;
      } SyncRegistration;

      CCMI_assert(sizeof(SyncRegistration) <= sizeof(CCMI_CollectiveProtocol_t));

      SyncRegistration *treg = (SyncRegistration *) registration;

      new (& treg->minfo, sizeof(treg->minfo))
      CCMI::Adaptor::Generic::MulticastImpl();

      new (& treg->reduce_registration, sizeof(treg->reduce_registration))
      CCMI::Adaptor::Allreduce::RingReduce::Factory
        (_g_generic_adaptor->mapping(), & treg->minfo, configuration->cb_geometry, flags);

      treg->minfo.initialize(_g_generic_adaptor);

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
int CCMI_Reduce (CCMI_CollectiveProtocol_t  * registration,
                 CCMI_CollectiveRequest_t   * request,
                 CCMI_Callback_t              cb_done,
                 CCMI_Consistency             consistency,
                 CCMI_Geometry_t            * geometry_request,
                 int                          root,
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
    unsigned status =  allreduce->restart((CCMI_CollectiveRequest_t*)request,
                                          *(CCMI_Callback_t *)&cb_done,
                                          (CCMI_Consistency)consistency,
                                          srcbuf,
                                          dstbuf,
                                          count,
                                          (CCMI_Dt)dtype,
                                          (CCMI_Op)op,
                                          root);
    //    if (status != CCMI_SUCCESS) fprintf(stderr, "CCMI_Reduce::ALERT: restart failed on executor %#X with status %#X\n", (int) allreduce, status);
    if(status == CCMI_SUCCESS)
    {
      TRACE_ADAPTOR((stderr, "<%#.8X>CCMI_Reduce::ALERT: restart successful with status %#X\n", 
                 (int) allreduce, status));

      geometry->setAllreduceComposite(allreduce);
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

  //fprintf(stderr, "CCMI_Reduce::ALERT: generate executor %#X with factory %#X\n",(int) allreduce,(int)factory);
  void *ptr =factory->generate((CCMI_CollectiveRequest_t*)request,
                               *(CCMI_Callback_t *) &cb_done,
                               (CCMI_Consistency) consistency,
                               geometry,
                               srcbuf,
                               dstbuf,
                               count,
                               (CCMI_Dt)dtype,
                               (CCMI_Op)op,
                               root);
  if(ptr == NULL)
  {
//    fprintf(stderr, "CCMI_Reduce::ALERT: generate failed\n");
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


