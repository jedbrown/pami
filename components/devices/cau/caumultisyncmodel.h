/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caumultisyncmodel.h
 * \brief ???
 */

#ifndef __components_devices_cau_caumultisyncmodel_h__
#define __components_devices_cau_caumultisyncmodel_h__

#include <pami.h>
#include "components/devices/MultisyncModel.h"
#include "components/devices/cau/caumessage.h"

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
      class CAUMultisyncModel :
    public Interface::MultisyncModel<CAUMultisyncModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
      {
      public:
	static const size_t msync_model_state_bytes = sizeof(T_Message);
	static const size_t sizeof_msg              = sizeof(T_Message);
	CAUMultisyncModel (T_Device &device, pami_result_t &status) :
	  Interface::MultisyncModel<CAUMultisyncModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>(device,status),
	  _device(device)
	    {
	      status = PAMI_SUCCESS;
	    }
	  pami_result_t postMultisync (uint8_t         (&state)[msync_model_state_bytes],
				       pami_multisync_t *msync)
	  {
	  }
	  T_Device                     &_device;
      };
  };
};
#endif // __components_devices_mpi_mpimultisyncmodel_h__
