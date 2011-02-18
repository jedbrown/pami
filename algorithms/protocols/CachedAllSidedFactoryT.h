
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"

namespace CCMI
{
  namespace Adaptor
  {
    ///
    /// Cached All Sided Factory that caches the collective in the
    /// geometry and later retrieves the object. The reset method upates
    /// composite with new parameters
    ///
    template < class T, MetaDataFn get_metadata, class C, PAMI::Geometry::ckeys_t T_Key >
      class CachedAllSidedFactoryT : public AllSidedCollectiveProtocolFactoryT<T, get_metadata, C>
      {
      public:
      CachedAllSidedFactoryT(C                           *cmgr,
			     Interfaces::NativeInterface *native):
	AllSidedCollectiveProtocolFactoryT<T, get_metadata, C>(cmgr, native)
	  {
	    TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
	  }
	
	virtual Executor::Composite * generate(pami_geometry_t              geometry,
					       void                       * cmd)
	  {      
	    TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
	    PAMI_GEOMETRY_CLASS  *g = ( PAMI_GEOMETRY_CLASS *)geometry;
	    /// \todo does NOT support multicontext
	    T *c = (T *) g->getKey((size_t)0, T_Key);
	    
	    if (!c)
	      {
		c = (T *) AllSidedCollectiveProtocolFactoryT<T, get_metadata, C>::generate(geometry, cmd);
		g->setKey((size_t)0, /// \todo does NOT support multicontext
			  T_Key,
			  (void*)c);
	      }
	    else 
	      //Reset composite with new collective inputs
	      c->reset (geometry, cmd);

	    pami_xfer_t *xfer = (pami_xfer_t *)cmd;
	    c->setDoneCallback(xfer->cb_done, xfer->cookie);
	    return c;
	  }
      };
  };
};
