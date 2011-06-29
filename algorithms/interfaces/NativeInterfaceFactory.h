
#ifndef __algorithms_interfaces_NativeInterfaceFactory_h__
#define __algorithms_interfaces_NativeInterfaceFactory_h__

#include <pami.h>
#include "util/common.h"
#include "algorithms/interfaces/NativeInterface.h"

namespace CCMI {
  namespace Interfaces {
class Geometry;
    //class NativeInterface;

    class NativeInterfaceFactory {      
    public:

      typedef enum _nisel {
	MULTICAST      = 0,
	MULTICOMBINE      ,
	MANYTOMANY        ,
	P2P               ,
	ALL               ,
      } NISelect;

      typedef enum _nitype {
	ALLSIDED        = 0,
	ACTIVE_MESSAGE     ,
      } NIType; 
      
      NativeInterfaceFactory() {}      
      
      /* 
	 \brief generate a native interface to optimize operations given by NISelect flag input
	 \param [in] dispatch id. Id of the dispatch for AM M* implementations
	 \param [in] sflag.  Construct M* interfaces to support subset or all of the M* calls. 
	 \param [out] ni. Output native interface
	 \return PAMI_SUCCESS or PAMI_ERROR	         
      */
      virtual pami_result_t generate 
	( int               *  dispatch_id,
	  NISelect             sflag,
	  NIType               tflag,
	  NativeInterface   *& ni) = 0;
      virtual pami_result_t analyze(size_t context_id, void *geometry, int phase, int* flag) = 0;

    };
  };
};


#endif
