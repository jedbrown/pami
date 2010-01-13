
#ifndef __algorithms_protocols_CollectiveProtocolFactoryT_h__
#define __algorithms_protocols_CollectiveProtocolFactoryT_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/interfaces/NativeInterface.h"

namespace CCMI
{
  namespace Adaptor
  {
    ///
    /// \brief choose if this protocol is supports the input geometry
    ///
    typedef bool      (*AnalyzeFn)   (XMI_GEOMETRY_CLASS                  * g);

    template <class T, AnalyzeFn afn, class C>
      class CollectiveProtocolFactoryT: public CollectiveProtocolFactory
    {
      C                                          * _cmgr;
      Interfaces::NativeInterface                * _native;

    public:
      CollectiveProtocolFactoryT (C *cmgr, Interfaces::NativeInterface *native, xmi_dispatch_multicast_fn cb_head=NULL): CollectiveProtocolFactory(), _cmgr(cmgr), _native(native)
      {
	xmi_dispatch_callback_fn fn;
	fn.multicast = (xmi_dispatch_multicast_fn) cb_head;
	_native->setDispatch(fn, this);
      }

      virtual ~CollectiveProtocolFactoryT ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      /// \brief All protocols determine if a given geometry is adequate
      virtual bool Analyze(XMI_GEOMETRY_CLASS *g) { return afn(g); }      

      virtual Executor::Composite * generate(void                      * request_buf,
					     size_t                      rsize,
					     xmi_context_t               context, 
					     void                      * cmd) 
	{
	  T *t = new (request_buf) T(cmd, _cmgr, _native);
	  return (Executor::Composite *)t;
	}

    };
  };
};

#endif
