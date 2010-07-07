/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2010, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

////////////////////////////////////////////////////////////////////////////////
///
/// \file components/devices/bgq/mu2/global/Global.h
///
/// \brief MU Global Definitions
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __components_devices_bgq_mu_global_Global_h__
#define __components_devices_bgq_mu_global_Global_h__

#ifdef __FWEXT__

#include <firmware/include/fwext/fwext.h>
#include <firmware/include/Firmware.h>
#include <hwi/include/bqc/A2_core.h>
#include <hwi/include/bqc/A2_inlines.h>
#include <firmware/include/fwext/fwext_lib.h>
#include <firmware/include/fwext/fwext_nd.h>
#include <firmware/include/fwext/fwext_mu.h>

#endif // __FWEXT__

#include <stdio.h>
#include <stdlib.h>
#include "Mapping.h"
#include "components/devices/bgq/mu2/global/ResourceManager.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x


namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class Global
      {
        public:
	
	//////////////////////////////////////////////////////////////////////////
	///
	/// \brief MU Global Default Constructor
	///
	/////////////////////////////////////////////////////////////////////////
	Global ( PAMI::ResourceManager &pamiRM,
		 PAMI::Mapping         &mapping,
		 PAMI::BgqPersonality  &pers,
		 PAMI::Memory::MemoryManager   &mm ) :
	  _pamiRM( pamiRM ),
	  _mapping( mapping ),
	  _muRM( pamiRM, mapping, pers, mm )
	{
	} // End: Global Default Constructor
	  
	PAMI::ResourceManager &getPamiRM()  { return _pamiRM;  }
	PAMI::Mapping         &getMapping() { return _mapping; }
	ResourceManager       &getMuRM()    { return _muRM;    }

        private:


	//////////////////////////////////////////////////////////////////////////
	///
	/// Member data:
	///
	/////////////////////////////////////////////////////////////////////////

	PAMI::ResourceManager &_pamiRM;
	PAMI::Mapping         &_mapping;
	ResourceManager        _muRM;
	
      }; // Global class
    }; // MU     namespace
  };   // Device namespace
};     // PAMI   namespace



#undef TRACE

#endif   // __components_devices_bgq_mu_global_Global_h__
