

#ifndef __components_devices_bgq_mu2_msg_InjectDPutBase_h__
#define __components_devices_bgq_mu2_msg_InjectDPutBase_h__

#include "spi/include/mu/DescriptorBaseXX.h"
#include "util/trace.h"
#include "components/devices/bgq/mu2/InjChannel.h"
#include "components/devices/bgq/mu2/msg/MessageQueue.h"
#include "common/bgq/Mapping.h"
#include "components/memory/MemoryAllocator.h"
#include "components/devices/bgq/mu2/MU_Util.h"

#undef DO_TRACE_ENTEREXIT 
#undef DO_TRACE_DEBUG     
#ifdef CCMI_TRACE_ALL
  #define DO_TRACE_ENTEREXIT 1
  #define DO_TRACE_DEBUG     1
#else
  #define DO_TRACE_ENTEREXIT 0
  #define DO_TRACE_DEBUG     0
#endif

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {      
      ///
      /// \brief Inject one or more dput descriptors into an inject
      /// fifo      
      ///
      class InjectDPutBase {
      protected:
	bool                     _doneCompletion;
	
      public:
	InjectDPutBase (): _doneCompletion (false) {}

	inline bool done () { return _doneCompletion; }
	virtual bool advance() { PAMI_abort(); }	
      };  //-- InjectDPutBase
    };  //-- MU
  };  //-- Device
};  //-- PAMI

#endif
