/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///


#define TRACE_ERR(x)  // fprintf x

#include "collectives/interface/ll_multisend.h"

extern "C" int LL_Multicast(LL_Multicast_t *minfo) 
{
}


CM_Result LL_Multicast_register (CM_Protocol_t                * registration,
				 LL_Multicast_Configuration_t * configuration)
{

}


CM_Result LL_Manytomany_register (CM_Protocol_t      * registration,
				  LL_Manytomany_Configuration_t *configuration)
{

}

CM_Result LL_Multisync_register(CM_Protocol_t *registration,
				LL_Multisync_configuration_t *configuration)
{

}

CM_Result LL_Multicombine_register(CM_Protocol_t *registration,
				LL_Multicombine_configuration_t *configuration)
{

}

#undef TRACE_ERR
