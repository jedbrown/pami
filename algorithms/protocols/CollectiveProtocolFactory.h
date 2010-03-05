/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/CollectiveProtocolFactory.h
 * \brief ???
 */

#ifndef __algorithms_protocols_CollectiveProtocolFactory_h__
#define __algorithms_protocols_CollectiveProtocolFactory_h__

//#include "TypeDefs.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/composite/Composite.h"

namespace CCMI
{
  namespace Adaptor
  {


    class CollectiveProtocolFactory
    {
    public:
      CollectiveProtocolFactory ()
      {
	_cb_geometry = NULL;
      }

      void setMapIdToGeometry(xmi_mapidtogeometry_fn     cb_geometry) {
	_cb_geometry = cb_geometry;
      }

      xmi_geometry_t getGeometry(unsigned id) {
	CCMI_assert (_cb_geometry != NULL);
	return _cb_geometry (id);
      }

      virtual ~CollectiveProtocolFactory ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      virtual Executor::Composite * generate(xmi_geometry_t              geometry,
					     void                      * cmd) = 0;

      virtual void metadata(xmi_metadata_t *mdata) = 0;

      virtual void setAsyncInfo (bool                          is_buffered,
                                 xmi_dispatch_callback_fn      cb_async,
                                 xmi_mapidtogeometry_fn        cb_geometry)
	{
          XMI_abort();
        };

    protected:
      xmi_mapidtogeometry_fn     _cb_geometry;
    };
  };
};

#endif
