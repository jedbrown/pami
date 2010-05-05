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

      void setMapIdToGeometry(pami_mapidtogeometry_fn     cb_geometry) {
        _cb_geometry = cb_geometry;
      }


      pami_geometry_t getGeometry(unsigned id) {
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

      virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                             void                      * cmd) = 0;

      virtual void metadata(pami_metadata_t *mdata) = 0;

      virtual void setAsyncInfo (bool                          is_buffered,
                                 pami_dispatch_callback_fn      cb_async,
                                 pami_mapidtogeometry_fn        cb_geometry)
        {
          PAMI_abort();
        };

      pami_mapidtogeometry_fn getMapIdToGeometry() {
        return _cb_geometry;
      }

    protected:
      pami_mapidtogeometry_fn              _cb_geometry;
    };
  };
};

#endif
