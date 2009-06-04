#include "collectives/interface/ll_multisend.h"
#include "collectives/interface/MultiSend.h"

// placeholder until we find where to get the ctxt from
void *   _g_ctxt;

#ifdef __cplusplus
extern "C"
{
#endif

    int LL_register(LL_CollectiveProtocol_t      *registration,
                    LL_CollectiveConfiguration_t *config)
    {
	LL_Result                                status;
	LL::MultiSend::MultisendProtocolFactory *factory;
	switch(config->cfg_type)
	    {
	    case LL_CFG_MULTISYNC:
		{
		    LL_Multisync_Configuration_t *cfg    = (LL_Multisync_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case LL_DEFAULT_MULTISYNC_PROTOCOL:
			    factory = new (registration) LL::MultiSend::MultisyncFactory(_g_ctxt,status);
			    break;
			default:
			    break;
			}
		}
		break;
	    case LL_CFG_MULTICAST:
		{
		    LL_Multicast_Configuration_t *cfg    = (LL_Multicast_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case LL_DEFAULT_MCAST_PROTOCOL:
			    factory = new (registration) LL::MultiSend::MulticastFactory(_g_ctxt,status);
			    break;
			default:
			    break;
			}
		}
		break;
	    case LL_CFG_MANY2MANY:
		{
		    LL_Manytomany_Configuration_t *cfg   = (LL_Manytomany_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case LL_DEFAULT_M2M_PROTOCOL:
			    factory = new (registration) LL::MultiSend::ManyToManyFactory(_g_ctxt,1,status);
			    break;
			default:
			    break;
			}
		}
		break;
	    case LL_CFG_MULTICOMBINE:
		{
		    LL_Multicombine_Configuration_t *cfg = (LL_Multicombine_Configuration_t *)config;
		    switch(cfg->protocol)
			{
			case LL_DEFAULT_MULTICOMBINE_PROTOCOL:
			    factory = new (registration) LL::MultiSend::MulticombineFactory(_g_ctxt,status);
			    break;
			default:
			    break;
			}
		}
		break;
	    default:
		break;
	    }
	return LL_SUCCESS;
    }


    LL_Result LL_Multicast(LL_Multicast_t *mcastinfo)
    {
	LL::MultiSend::MulticastFactory * factory =
	    (LL::MultiSend::MulticastFactory *) mcastinfo->registration;
	return factory->generate (mcastinfo);
    }

    LL_Result LL_Manytomany(LL_Manytomany_t *m2minfo)
    {
	LL::MultiSend::ManyToManyFactory * factory =
	    (LL::MultiSend::ManyToManyFactory *) m2minfo->registration;
	return factory->generate (m2minfo);
    }

    LL_Result LL_Multisync(LL_Multisync_t *msyncinfo)
    {
	LL::MultiSend::MultisyncFactory * factory =
	    (LL::MultiSend::MultisyncFactory *) msyncinfo->registration;
	return factory->generate (msyncinfo);
    }

    LL_Result LL_Multicombine(LL_Multicombine_t *mcombineinfo)
    {
	LL::MultiSend::MulticombineFactory * factory =
	    (LL::MultiSend::MulticombineFactory *) mcombineinfo->registration;
	return factory->generate (mcombineinfo);
    }

    int LL_Xfer (void* context, LL_xfer_t cmd)
    {
	return 0;
    }

    LL_Result LL_Multisend_getRoles(LL_CollectiveProtocol_t *registration,
                                    int           *numRoles,
                                    int           *replRole)
    {

	return LL_SUCCESS;
    }


#ifdef __cplusplus
};
#endif
