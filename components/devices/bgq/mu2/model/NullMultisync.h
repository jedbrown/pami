
#ifndef __MU_NULL_MULTI_SYNC_H__
#define __MU_NULL_MULTI_SYNC_H__

#include "components/devices/MultisyncModel.h"
#include "components/devices/bgq/mu2/Context.h"
#include "sys/pami.h"

///
/// \brief Null Models to optimize space for native ineterfaces that do not call multisync
///
namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class NullMultisyncModel: public Interface::MultisyncModel <NullMultisyncModel, MU::Context, 0>
      {
      public:
	static const size_t   sizeof_msg      =  0;
	  
	NullMultisyncModel (pami_client_t     client,
			    pami_context_t    context,
			    MU::Context     & mucontext,
			    pami_result_t   & status):
	Interface::MultisyncModel <NullMultisyncModel, MU::Context, 0> (mucontext, status)
	{
	}

	/// \see PAMI::Device::Interface::MultisyncModel::postMultisync
	pami_result_t postMultisyncImmediate_impl(size_t            client,
						  size_t            context, 
						  pami_multisync_t *msync,
						  void             *devinfo = NULL) 
	{
	  printf ("PAMI Error : In NULL Multisync\n");
	  return PAMI_ERROR;
	}
	
	
	/// \see PAMI::Device::Interface::MultisyncModel::postMultisync
	pami_result_t postMultisync_impl(uint8_t (&state)[sizeof_msg],
					 size_t             client,
					 size_t             context, 
					 pami_multisync_t * msync,
					 void             * devinfo = NULL) 
	{
	  printf ("PAMI Error : In NULL Multisync\n");
	  return PAMI_ERROR;	
      }
      };
    };
  };
};
  
#endif
