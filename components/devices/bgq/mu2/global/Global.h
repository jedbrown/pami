/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

#ifndef __components_devices_bgq_mu2_global_Global_h__
#define __components_devices_bgq_mu2_global_Global_h__

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


////////////////////////////////////////////////////////////////////////////////
/// \env{rgetpacing,PAMI_RGETPACINGSIZE}
///
/// Messages exceeding this size in bytes are
/// considered for pacing.  With pacing, the message is broken up into
/// smaller sub-messages, and there is a limit on the number of sub-messages
/// that can be in the network at one time from a given node.  Specifying a
/// very large size (e.g. 999999999) turns off pacing.
///
/// \default 8192
////////////////////////////////////////////////////////////////////////////////


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
		 PAMI::BgqJobPersonality  &pers,
		 PAMI::Memory::MemoryManager   &mm ) :
	  _pamiRM( pamiRM ),
	  _mapping( mapping ),
	  _muRM( pamiRM, mapping, pers, mm )
	{
	  // Determine the message size above which rgets will be paced.
	  char *s;
	  unsigned long v;
	  
	  _rgetPacingSize = 8192;
	  s = getenv( "PAMI_RGETPACINGSIZE" );
	  if ( s )
	    {
	      v = strtoul( s, 0, 10 );
	      _rgetPacingSize = (size_t)v;
	    }

	} // End: Global Default Constructor
	  
	~Global() {
	}

	PAMI::ResourceManager &getPamiRM()  { return _pamiRM;  }
	PAMI::Mapping         &getMapping() { return _mapping; }
	ResourceManager       &getMuRM()    { return _muRM;    }
	size_t                 getRgetPacingSize() { return _rgetPacingSize; }

        private:


	//////////////////////////////////////////////////////////////////////////
	///
	/// Member data:
	///
	/////////////////////////////////////////////////////////////////////////

	PAMI::ResourceManager &_pamiRM;
	PAMI::Mapping         &_mapping;
	ResourceManager        _muRM;
	size_t                 _rgetPacingSize;

      }; // Global class
    }; // MU     namespace
  };   // Device namespace
};     // PAMI   namespace



#undef TRACE

#endif   // __components_devices_bgq_mu_global_Global_h__
