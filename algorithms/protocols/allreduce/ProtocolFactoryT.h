/**
 * \file algorithms/protocols/allreduce/ProtocolFactoryT.h
 * \brief ???
 */
#ifndef __algorithms_protocols_allreduce_ProtocolFactoryT_h__
#define __algorithms_protocols_allreduce_ProtocolFactoryT_h__

#include "algorithms/protocols/CollectiveProtocolFactoryT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      template <class T, MetaDataFn get_metadata, class C>
	class ProtocolFactoryT: public CollectiveProtocolFactoryT<T, get_metadata, C>
      {
      public:
	ProtocolFactoryT (C                           *cmgr,
			  Interfaces::NativeInterface *native,
			  pami_dispatch_multicast_function cb_head=NULL):
	CollectiveProtocolFactoryT<T, get_metadata, C>(cmgr, native, cb_head)
	{
	}

	virtual ~ProtocolFactoryT ()
	{
	}

	/// NOTE: This is required to make "C" programs link successfully with virtual destructors
	void operator delete(void * p)
	{
	  CCMI_abort();
	}

	virtual Executor::Composite * generate(pami_geometry_t              g,
					     void                      * cmd) {
	  PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
	  T * arcomposite = (T *)geometry->getAllreduceComposite();

	  pami_xfer_t *allreduce = (pami_xfer_t *)cmd;
	  ///If the allreduce algorithm was created by this factory before, just restart it
	  if(arcomposite != NULL  &&  arcomposite->getAlgorithmFactory() == this)
	    {
	      pami_result_t status =  (pami_result_t)arcomposite->restart(allreduce);
	      if(status == PAMI_SUCCESS)
	      {
		geometry->setAllreduceComposite(arcomposite);
		return NULL;
	      }
	    }

	  if(arcomposite != NULL) // Different factory?  Cleanup old executor.
	  {
	    geometry->setAllreduceComposite(NULL);
	    arcomposite->~T(); //Call destructor
	    CollectiveProtocolFactoryT<T, get_metadata, C>::_alloc.returnObject(arcomposite);
	  }

	  T* obj = (T*)CollectiveProtocolFactoryT<T, get_metadata, C>::_alloc.allocateObject();
	  new (obj) T(CollectiveProtocolFactoryT<T, get_metadata, C>::_native,  // Native interface
		      CollectiveProtocolFactoryT<T, get_metadata, C>::_cmgr,    // Connection Manager
		      geometry,          // Geometry Object
		      (pami_xfer_t*) cmd, // Parameters
		      allreduce->cb_done,
		      allreduce->cookie);

	  //obj->setAlgorithmFactory(this); //Dont use restart just yet
	  geometry->setAllreduceComposite(obj);
	  return NULL;
	}
      };
    };
  };
}; //CCMI

#endif
