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
#include  "algorithms/interfaces/Executor.h"
#include  "algorithms/composite/Composite.h"
namespace CCMI
{
  namespace Adaptor
  {

    class CollectiveProtocolFactory
    {
      public:
        CollectiveProtocolFactory ()
        {
          TRACE_ADAPTOR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
          _cb_geometry = NULL;
        }

        void setMapIdToGeometry(pami_mapidtogeometry_fn     cb_geometry)
        {
          TRACE_ADAPTOR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
          _cb_geometry = cb_geometry;
        }


        pami_geometry_t getGeometry(unsigned id)
        {
          TRACE_ADAPTOR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
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
                                   pami_dispatch_callback_function cb_async,
                                   pami_mapidtogeometry_fn        cb_geometry)
        {
          TRACE_ADAPTOR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
          PAMI_abort();
        };

        pami_mapidtogeometry_fn getMapIdToGeometry()
        {
          TRACE_ADAPTOR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
          return _cb_geometry;
        }

      protected:
        pami_mapidtogeometry_fn              _cb_geometry;
    };
  };
};

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
