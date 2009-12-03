
#ifndef  __ccmi_interfaces_nativeinterface__
#define  __ccmi_interfaces_nativeinterface__

#include "sys/xmi.h"

namespace CCMI {
  namespace Interfaces {
    class NativeInterface {
    protected:
      unsigned         _myrank;
      unsigned         _numranks;

    public:      
      NativeInterface(unsigned myrank, unsigned numranks): _myrank(myrank),_numranks(numranks){}      
      
      unsigned myrank()   { return _myrank; }
      unsigned numranks() { return _numranks; }

      /// \brief this call is called when the native interface is initialized
      virtual void setDispatch (xmi_dispatch_callback_fn fn, void *cookie) = 0;
      
      virtual xmi_result_t multicast    (xmi_multicast_t *mcast) = 0;
      virtual xmi_result_t multysync    (xmi_multisync_t *msync) = 0; 
      virtual xmi_result_t multicombine (xmi_multicombine_t *mcombine) = 0;
    };
  };
};

#endif
