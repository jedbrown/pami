
#ifndef __MU_NULL_MULTI_COMBINE_H__
#define __MU_NULL_MULTI_COMBINE_H__

#include "components/devices/MulticombineModel.h"
#include "components/devices/bgq/mu2/Context.h"
#include "sys/pami.h"

///
/// \brief Null Models to optimize space for native ineterfaces that do not call multicombine
///
namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class NullMulticombineModel: public Interface::MulticombineModel < NullMulticombineModel, MU::Context, 0 >
      {
      public:
	static const size_t   sizeof_msg      =  0;
	  
	NullMulticombineModel (pami_client_t     client,
			       pami_context_t    context,
			       MU::Context     & mucontext,
			       pami_result_t   & status):
	Interface::MulticombineModel <NullMulticombineModel, MU::Context, 0> (mucontext, status)
	{
	}

	pami_result_t postMulticombineImmediate_impl(size_t                   client,
						     size_t                   context,
						     pami_multicombine_t    * mcombine,
						     void                   * devinfo = NULL)
	  
	{
	  printf ("PAMI Error : In NULL Multicombine\n");
	  return PAMI_ERROR;
	}
	
      
	pami_result_t postMulticombine_impl(uint8_t (&state)[sizeof_msg],
					    size_t                client,
					    size_t                context, 
					    pami_multicombine_t * mcomb,
					    void                * devinfo = NULL) 
	{
	printf ("PAMI Error : In NULL Multicombine\n");
	return PAMI_ERROR;	
	}
      };
    };
  };
};
#endif
